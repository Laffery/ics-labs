/* 
 * test_case    miss
 * 32 * 32      287
 * 64 * 64      1243
 * 61 * 67      1950
 * 
 *  trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[N][M]);

void trans_32_32(int M, int N, int A[N][M], int B[M][N]);
void trans_64_64(int M, int N, int A[N][M], int B[M][N]);
void trans_61_67(int M, int N, int A[N][M], int B[M][N]);
void trans(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    if(M == 32 && N == 32){
        trans_32_32(32, 32, A, B);
        return;
    }

    if(M == 64 && N == 64){
        trans_64_64(64, 64, A, B);
        return;
    }

    if(M == 61 && N == 67){
        trans_61_67(61, 67, A, B);
        return;
    }

    trans(M, N, A, B);
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

char trans_32_32_desc[] = "32 * 32 transpose";
/*
 * Arguments of cache is (s = 5, E = 1, b = 5) => there are 32 lines in cache,  
 * every line has 32 bytes which equals to 8 int, so that the cache can store 
 * 8 rows of matrix A. So we use 8 * 8 block. Load one row of a block in one 
 * time then assign them to B. This method can decread great number of overlap.
 */ 
void trans_32_32(int M, int N, int A[N][M], int B[M][N])
{
    int a0, a1, a2, a3, a4, a5, a6, a7;
    for(int i = 0; i < N; i += 8){// row
        for(int j = 0; j < M; j += 8){// column
            for(int k = i; k < i + 8; ++k){
                // load data from A
                a0 = A[k][j];
                a1 = A[k][j+1];
                a2 = A[k][j+2];
                a3 = A[k][j+3];
                a4 = A[k][j+4];
                a5 = A[k][j+5];
                a6 = A[k][j+6];
                a7 = A[k][j+7];
                
                // write back to B
                B[j][k] = a0;
                B[j+1][k] = a1;
                B[j+2][k] = a2;
                B[j+3][k] = a3;
                B[j+4][k] = a4;
                B[j+5][k] = a5;
                B[j+6][k] = a6;
                B[j+7][k] = a7;
            }
        }
    }
}

char trans_64_64_desc[] = "64 * 64 transpose";
/*
 * Similar to 32 * 32, the cache can only store 4 rows of matrix A, so that 
 * previos 8 * 8 may not work. A improved method is to block every 8 * 8 matrix
 * into 4 * 4 matrixs. Just transpose these little matrixs in 8 * 8 matrix, then 
 * transpose 8 * 8 matrixs in 64 * 64 matrix. In case of only 4 int we use and 
 * another 4 int waste, we can temporarily store the rest data in matrix B.
 */
void trans_64_64(int M, int N, int A[N][M], int B[M][N])
{
    int a0, a1, a2, a3, a4, a5, a6, a7;
    for(int i = 0; i < N; i += 8){// row
        for(int j = 0; j < M; j += 8){// column
            for(int k = i; k < i + 4; ++k){ // top 4 rows in 8 * 8 matrix
                // transpose top left 4 * 4 matrix
                a0 = A[k][j];
                a1 = A[k][j+1];
                a2 = A[k][j+2];
                a3 = A[k][j+3];
                
                B[j][k] = a0;
                B[j+1][k] = a1;
                B[j+2][k] = a2;
                B[j+3][k] = a3;

                // transpose and move top right, whose destination is bottom left, to the same place
                a4 = A[k][j+4];
                a5 = A[k][j+5];
                a6 = A[k][j+6];
                a7 = A[k][j+7];
                
                B[j][k+4] = a4;
                B[j+1][k+4] = a5;
                B[j+2][k+4] = a6;
                B[j+3][k+4] = a7;
            }

            // transpose and move bottom left to top right 
            for(int k = j; k < j + 4; ++k){
                // load bottom left data
                a0 = A[i+4][k];
                a1 = A[i+5][k];
                a2 = A[i+6][k];
                a3 = A[i+7][k];

                // load top right data had been stored in B last step
                a4 = B[k][i+4];
                a5 = B[k][i+5];
                a6 = B[k][i+6];
                a7 = B[k][i+7];
                
                // move bottom left to top right
                B[k][i+4] = a0;
                B[k][i+5] = a1;
                B[k][i+6] = a2;
                B[k][i+7] = a3;

                // move top right to bottom left
                B[k+4][i] = a4;
                B[k+4][i+1] = a5;
                B[k+4][i+2] = a6;
                B[k+4][i+3] = a7;
            }

            // transpose bottom right 4 * 4 matrix
            for(int k = i + 4; k < i + 8; ++k){
                a0 = A[k][j+4];
                a1 = A[k][j+5];
                a2 = A[k][j+6];
                a3 = A[k][j+7];
                
                B[j+4][k] = a0;
                B[j+5][k] = a1;
                B[j+6][k] = a2;
                B[j+7][k] = a3;
            }
        }
    }
}

char trans_61_67_desc[] = "61 * 67 transpose";
/*
 * In fact, 61 and 67 are two prime number, it's natural to have overlapping
 * with A when locating matrix B, so just use a simple block and directly assign 
 * B with A data to handle this problem. People used to using 16 as a element, 
 * but a prime number 17 works best with 1950 of misses count.
 */
void trans_61_67(int M, int N, int A[N][M], int B[M][N])
{
    for(int i = 0; i < N; i += 17){
        for(int j = 0; j < M; j += 17){
            for(int k = i; k < i + 17 && k < N; k++)
                for(int l = j; l < j + 17 && l < M; l++)
                    B[l][k] = A[k][l];
        }
    }
}


/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

