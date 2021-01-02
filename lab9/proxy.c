/*
 * proxy.c - ICS Web proxy
 *
 */

#include "csapp.h"
#include <stdarg.h>
#include <sys/select.h>

/*
 * Function prototypes
 */
int parse_uri(char *uri, char *target_addr, char *path, char *port);
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, char *uri, size_t size);

/*
 * new wrappers, simply return after printing a warning message when I/O fails. When either
 * of the read wrappers detects an error, it returns 0, as though it encountered EOF on the socket
 */ 
ssize_t Rio_readn_w(int fd, void *usrbuf, size_t n);
ssize_t Rio_readnb_w(rio_t *rp, void *usrbuf, size_t n);
ssize_t Rio_readlineb_w(rio_t *rp, void *usrbuf, size_t maxlen);
void Rio_writen_w(int fd, void *usrbuf, size_t n);

/* self-defined functions to implement */
typedef struct args{
    int connfd;
    struct sockaddr_in client_addr;
} args_t; // to send args to thread

void *thread(void *vargp);

sem_t mutex;// used to sync log files in thread

/*
 * main - Main routine for the proxy program
 */
int main(int argc, char **argv)
{
    /* Check arguments */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port number>\n", argv[0]);
        exit(0);
    }

    /* used variables table */
    int listenfd;
    args_t *argsptr;
    socklen_t clientlen = sizeof(struct sockaddr_in);
    pthread_t tid;

    Sem_init(&mutex, 0, 1);
    listenfd = Open_listenfd(argv[1]);
    Signal(SIGPIPE, SIG_IGN);

    while(1) {
        // malloc for each argument struct to avoid racing
        argsptr = Malloc(sizeof(args_t));
        argsptr->connfd = Accept(listenfd, (SA *)(&(argsptr->client_addr)), &clientlen);
        
        /* create new thread */
        Pthread_create(&tid, NULL, thread, argsptr);
    }

    /* close listen file descriptor */
    Close(listenfd);
    exit(0);
}

/* what a thread will do */
void *thread(void *vargp)
{
    // detached
    Pthread_detach(Pthread_self());

    // get received args, follows are used variables
    args_t *argsptr = (args_t *)vargp;
    int connfd = argsptr->connfd, clientfd;// file descriptor for connection and client
    struct sockaddr_in client_addr = argsptr->client_addr;// as its name, client addr
    Free(argsptr);// free arguments pointer space

    char buf[MAXLINE], reqline[MAXLINE], log[MAXLINE];
    char method[10], uri[MAXLINE], version[10];
    char hostname[MAXLINE], pathname[MAXLINE - 24], port[MAXLINE];
    
    rio_t rio_client, rio_server;// as their name
    int receive_size = 0, content_size = 0;// size of received and content
    size_t n;

    /* parse request line */

    /* initialize rio */
    Rio_readinitb(&rio_client, connfd);

    /* can't get request line, maybe empty */
    if(!Rio_readlineb_w(&rio_client, buf, MAXLINE)){
        fprintf(stderr, "Cannot get request line\n");
        Close(connfd);
        return NULL;
    }

    /* request line requires arguments: method, uri, version */
    if(sscanf(buf, "%s %s %s", method, uri, version) != 3){
        fprintf(stderr, "Incorrect request line format\n");
        Close(connfd);
        return NULL;
    }

    /* check out parse_uri error */
    if(parse_uri(uri, hostname, pathname, port) < 0){
        fprintf(stderr, "parse_uri error\n");
        Close(connfd);
        return NULL;
    }

    /* failed to connect server */
    if((clientfd = open_clientfd(hostname, port)) == -1){
        fprintf(stderr, "Cannot connect to server\n");
        Close(connfd);
        return NULL;
    }

    /* parse request content and rewrite to proxy */

    /* initialize rio */
    Rio_readinitb(&rio_server, clientfd);

    /* request header: eg. GET / HTTP/1.1 */
    sprintf(reqline, "%s%s%s%s%s%s", method, " /", pathname, " ", version, "\r\n");
    Rio_writen_w(clientfd, reqline, strlen(reqline));

    /* request body: eg. Content-Length: 1024 */
    while((n = Rio_readlineb_w(&rio_client, buf, MAXLINE))){
        Rio_writen_w(clientfd, buf, n);

        if(!strcmp("\r\n", buf))// EOF
            break;

        // get Content-Length
        if(!strncmp("Content-Length: ", buf, 16))
            sscanf(buf+16, "%d", &content_size);
    }

    if(strcmp("GET", method)){
        if(!content_size)
            while((n = Rio_readlineb_w(&rio_client, buf, MAXLINE))){
                Rio_writen_w(clientfd, buf, n);
                if(!strcmp(buf, "0\r\n"))// request body end with 0\r\n
                    break;
            }

        else // bit by bit copy content
        {
            while(content_size){
                if(Rio_readnb_w(&rio_client, buf, 1) > 0)
                    Rio_writen_w(clientfd, buf, 1);
                content_size--;
            }
        }
    }
    content_size = 0;// reset, we will use it later

    /* server receive request */
    while((n = Rio_readlineb_w(&rio_server, buf, MAXLINE)) && strcmp("\r\n", buf)){
        Rio_writen_w(connfd, buf, n);
        receive_size += n;

        if(!strncmp("Content-Length: ", buf, 16))
            sscanf(buf+16, "%d", &content_size);
    }

    Rio_writen_w(connfd, "\r\n", 2);// empty line
    receive_size += 2;

    if(!content_size) // empty content
        while((n = Rio_readlineb_w(&rio_server, buf, MAXLINE)) && strcmp("0\r\n", buf)){
            Rio_writen_w(connfd, buf, n);
            receive_size += n;
        }
    else // bit by bit copy to connect file descriptor
    {
        while(content_size){
            if(Rio_readnb_w(&rio_server, buf, 1) > 0){
                Rio_writen_w(connfd, buf, 1);
                receive_size += 1;
            }
            content_size--;
        }
    }
    
    /* write log to proxy.log, use PV lock for security */
    P(&mutex);
    FILE *file = fopen("proxy.log", "a");
    format_log_entry(log, &client_addr, uri, receive_size);
    printf("%s\n", log);
    fclose(file);
    V(&mutex);

    /* close file descriptors */
    Close(connfd);
    Close(clientfd);


    return NULL;
}

ssize_t Rio_readn_w(int fd, void *usrbuf, size_t n)
{
    ssize_t nread = rio_readn(fd, usrbuf, n);
    if(nread < 0)
        fprintf(stderr, "Rio_readn error\n");

    return nread < 0 ? 0 : nread;
}

ssize_t Rio_readnb_w(rio_t *rp, void *usrbuf, size_t n)
{
    ssize_t nb = rio_readnb(rp, usrbuf, n);
    if(nb < 0)
        fprintf(stderr, "Rio_readnb error\n");

    return nb < 0 ? 0 : nb;
}

ssize_t Rio_readlineb_w(rio_t *rp, void *usrbuf, size_t maxlen)
{
    ssize_t n = rio_readlineb(rp, usrbuf, maxlen);
    if(n < 0)
        fprintf(stderr, "Rio_readlineb error\n");

    return n < 0 ? 0 : n;
}

void Rio_writen_w(int fd, void *usrbuf, size_t n)
{
    if(rio_writen(fd, usrbuf, n) != n)
        fprintf(stderr, "Rio_writen error\n");
}

/*
 * parse_uri - URI parser
 *
 * Given a URI from an HTTP proxy GET request (i.e., a URL), extract
 * the host name, path name, and port.  The memory for hostname and
 * pathname must already be allocated and should be at least MAXLINE
 * bytes. Return -1 if there are any problems.
 */
int parse_uri(char *uri, char *hostname, char *pathname, char *port)
{
    char *hostbegin;
    char *hostend;
    char *pathbegin;
    int len;

    if (strncasecmp(uri, "http://", 7) != 0) {
        hostname[0] = '\0';
        return -1;
    }

    /* Extract the host name */
    hostbegin = uri + 7;
    hostend = strpbrk(hostbegin, " :/\r\n\0");
    if (hostend == NULL)
        return -1;
    len = hostend - hostbegin;
    strncpy(hostname, hostbegin, len);
    hostname[len] = '\0';

    /* Extract the port number */
    if (*hostend == ':') {
        char *p = hostend + 1;
        while (isdigit(*p))
            *port++ = *p++;
        *port = '\0';
    } else {
        strcpy(port, "80");
    }

    /* Extract the path */
    pathbegin = strchr(hostbegin, '/');
    if (pathbegin == NULL) {
        pathname[0] = '\0';
    }
    else {
        pathbegin++;
        strcpy(pathname, pathbegin);
    }

    return 0;
}

/*
 * format_log_entry - Create a formatted log entry in logstring.
 *
 * The inputs are the socket address of the requesting client
 * (sockaddr), the URI from the request (uri), the number of bytes
 * from the server (size).
 */
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr,
                      char *uri, size_t size)
{
    time_t now;
    char time_str[MAXLINE];
    char host[INET_ADDRSTRLEN];

    /* Get a formatted time string */
    now = time(NULL);
    strftime(time_str, MAXLINE, "%a %d %b %Y %H:%M:%S %Z", localtime(&now));

    if (inet_ntop(AF_INET, &sockaddr->sin_addr, host, sizeof(host)) == NULL)
        unix_error("Convert sockaddr_in to string representation failed\n");

    /* Return the formatted log entry string */
    sprintf(logstring, "%s: %s %s %zu", time_str, host, uri, size);
}
