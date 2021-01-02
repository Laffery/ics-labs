/**********************************
 * 
 * name:
 * s_id:
 * 
 **********************************/

#include "cachelab.h"
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include "math.h"

// variables table you know what
int s = 0;
int E = 0;
int b = 0;
int v = 0;
int hit = 0;
int miss = 0; 
int eviction = 0;
char* trace_filename;

/* parse args of command line
 *
 * because getopt has an extern variable named optind symbols which 
 * arg we are parsing, so that we can't call getopt only one time,
 * just loop until the line end
 */
int parse_cmd(int argc,char* argv[]){
    while (optind < argc) {
        int arg = getopt(argc, argv, "s:E:b:t:v");
        switch (arg)
        {
        case 's':
            s = atoi(optarg);
            break;
        case 'E':
            E = atoi(optarg);
            break;
        case 'b':
            b = atoi(optarg);
            break;
        case 't':
            trace_filename = optarg;
            break;
        case 'v':
            v = 1;
            break;
        default:
            return 0;
        }
    }

    return 1;
}

// it's a line in a cache
// E line makes a set
// 2^s set makes a cache
typedef struct{
    int valid;  // valid byte
    int tag;    // tag byte
    int stamp;  // timestamp for LRU principle
} cache_line;

cache_line** cache; 

// malloc space for a cache, you can see defination of cache_line in header comment
void malloc_cache(void){
    
    cache = (cache_line **)malloc(pow(2, s) * sizeof(cache_line *)); 

    // init
    for(int i = 0; i < pow(2, s); ++i){
        cache[i] = (cache_line *)malloc(E * sizeof(cache_line));
        for(int j = 0; j < E; ++j){
            cache[i][j].valid = 0;
            cache[i][j].tag = 0;
            cache[i][j].stamp = 0;
        }
    }
}

// all stamp of cache lines++, for LRU
void timestamp(void){
    for(int i = 0; i < pow(2, s); ++i){
        for(int j = 0; j < E; ++j)
            cache[i][j].stamp += cache[i][j].valid;
    }
}


// execute a line of *.trace
void exe_line(long long int addr){
    // address in hex consists wish t bytes tag (high), s bytes set-index (mid)
    // and b bytes bits data, so we use bits option to get related values
    unsigned int set_addr = (addr >> b) & (-1u >> (32 - s)); // in which set
    unsigned int tag_addr = addr >> (b + s); // tags
    // printf("0x%llx 0x%x 0x%x ", addr, set_addr, tag_addr);

    for(int i = 0; i < E; ++i){
        if(cache[set_addr][i].valid && cache[set_addr][i].tag == tag_addr)// hit
        {
            // printf("hit at cache[%x][%d]\n", set_addr, i);
            if(v)
                printf(" hit");

            cache[set_addr][i].stamp = 0;// reset stamp
            hit++;
            return;
        }
    }

    // if reaches, means miss
    if(v)
        printf(" miss");
    for(int i = 0; i < E; ++i){
        if(cache[set_addr][i].valid == 0)// find a empty line
        {
            // printf("miss, select cache[%x][%d]\n", set_addr, i);
            cache[set_addr][i].valid = 1;
            cache[set_addr][i].stamp = 0;
            cache[set_addr][i].tag = tag_addr;
            miss++;
            return;
        }
    }

    // if reaches, means miss but no empty line --eviction
    // accorinding to LRU, find the line which has greatest stamp --means long time no visited
    if(v)
        printf(" eviction");
    
    miss++;
    eviction++;
    int max_stamp = 0;
    int max_stamp_index = 0;
    for(int i = 0; i < E; ++i){
        if(cache[set_addr][i].stamp > max_stamp)
        {
            max_stamp = cache[set_addr][i].stamp;
            max_stamp_index = i;
        }
    }
    // then op to line max_stamp_index
    // printf("evic, select cache[%x][%d]\n", set_addr, max_stamp_index);
    cache[set_addr][max_stamp_index].stamp = 0;
    cache[set_addr][max_stamp_index].tag = tag_addr;
}


int main(int argc, char* argv[])
{
    if(!parse_cmd(argc, argv))
        return 0;

    FILE *trace = fopen(trace_filename, "r");
    if(trace == NULL){
        printf("%s: No such file or directory\n", optarg);
        return 0;
    }

    malloc_cache();

    // parse *.trace line by line
    char line[20]; // temprory store a line of *.trace
    while(fgets(line, 20, trace)){
        // ignore 'I'
        if(line[0] == 'I')
            continue;

        // for vbose
        if(v){
            for(int i = 1; i < 20 && line[i] != '\n'; ++i)
                printf("%c", line[i]);
        }

        // parse line of *.trace, size is unused
        char type;
        long long int addr;
        int size;
        sscanf(line, " %c %llx,%d", &type, &addr, &size);

        // change string to long long int as address, because of ignoring cross field
        // , needn't parse what after ','
        switch (type)
        {
        case 'M':
            exe_line(addr); // no break, because 'M' both read and write
        case 'S': case 'L':
            exe_line(addr);
            break;
        default:
            break;
        }

        if(v)
            printf("\n");

        timestamp();
    }
    
    // free space
    for(int i = 0; i < pow(2, s); ++i)
        free(cache[i]);
    free(cache);

    // close file and print result
    fclose(trace);
    printSummary(hit, miss, eviction);
    return 0;
}
