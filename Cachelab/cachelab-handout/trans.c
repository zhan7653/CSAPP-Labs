/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes. s_bit = 5 b_bit = 5
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

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
    int i, j, i1, j1, a0, a1, a2, a3, a4, a5, a6, a7;
    if (M == 32) { // 8x8 block (misses=287)
        for (i = 0; i < 32; i+=8) {
            for (j = 0; j < 32; j+=8) {
                for (i1 = i; i1 < i+8; i1++) {
                    a0 = A[i1][j];
                    a1 = A[i1][j+1];
                    a2 = A[i1][j+2];
                    a3 = A[i1][j+3];
                    a4 = A[i1][j+4];
                    a5 = A[i1][j+5];
                    a6 = A[i1][j+6];
                    a7 = A[i1][j+7];
                    B[j][i1] = a0;
                    B[j+1][i1] = a1;
                    B[j+2][i1] = a2;
                    B[j+3][i1] = a3;
                    B[j+4][i1] = a4;
                    B[j+5][i1] = a5;
                    B[j+6][i1] = a6;
                    B[j+7][i1] = a7;
                }
            }
        }
    } else if (M == 64) { // 8x8 block with special handling (misses=1171)
        // The following description based on the first 8x8 block(A[0][0]~A[7][7]).
        for (i = 0; i < 64; i+=8) {
            for (j = 0; j < 64; j+=8) {
                // deal with the top half(A[0][0]~A[3][7]))
                for (i1 = i; i1 < i+4; i1++) { 
                    // load a line(8 int)
                    a0 = A[i1][j];
                    a1 = A[i1][j+1];
                    a2 = A[i1][j+2];
                    a3 = A[i1][j+3];
                    a4 = A[i1][j+4];
                    a5 = A[i1][j+5];
                    a6 = A[i1][j+6];
                    a7 = A[i1][j+7];
                    // the first four store in the final position
                    B[j][i1] = a0;
                    B[j+1][i1] = a1;
                    B[j+2][i1] = a2;
                    B[j+3][i1] = a3;
                    // the next four store in a temporary position
                    B[j][i1+4] = a4;
                    B[j+1][i1+4] = a5;
                    B[j+2][i1+4] = a6;
                    B[j+3][i1+4] = a7;                    
                }
                // deal with the lower left corner((A[4][0]~A[4][3])~(A[7][0]~A[7][3]))
                for (j1 = j; j1 < j+4; j1++) { 
                    // the first four load from the lower column of A(like A[4][0],A[5][0],...,A[7][0])
                    a0 = A[i+4][j1];
                    a1 = A[i+5][j1];
                    a2 = A[i+6][j1];
                    a3 = A[i+7][j1];
                    // the next four load form the temporary row of B(like B[0][4]~B[0][7] <- A[0][4]~A[0][7])
                    a4 = B[j1][i+4];
                    a5 = B[j1][i+5];
                    a6 = B[j1][i+6];
                    a7 = B[j1][i+7];
                    // store them in the final position
                    B[j1][i+4] = a0;
                    B[j1][i+5] = a1;
                    B[j1][i+6] = a2;
                    B[j1][i+7] = a3;
                    B[j1+4][i] = a4;
                    B[j1+4][i+1] = a5;
                    B[j1+4][i+2] = a6;
                    B[j1+4][i+3] = a7;
                }
                // deal with the lower right corner((A[4][4]~A[4][7])~(A[7][4]~A[7][7]))
                for (; j1 < j+8; j1++) {
                    a4 = A[i+4][j1];
                    a5 = A[i+5][j1];
                    a6 = A[i+6][j1];
                    a7 = A[i+7][j1];  
                    B[j1][i+4] = a4;
                    B[j1][i+5] = a5;
                    B[j1][i+6] = a6;
                    B[j1][i+7] = a7;                                      
                }
            }
        }



    } else if (M == 61) { // 16x16 block (misses=1816)
        for (j = 0; j < 61; j+=16) { // scan block move along column first
            for (i = 0; i < 67; i+=16) {
                for (i1 = i; i1 < i+16 && i1 < 67; i1++) {
                    for (j1 = j; j1 < j+16 && j1 < 61; j1++) {
                        a0 = A[i1][j1];
                        B[j1][i1] = a0;
                    }
                }
            }
        }
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

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

