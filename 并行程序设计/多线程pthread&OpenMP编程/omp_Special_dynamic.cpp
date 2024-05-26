#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <arm_neon.h>
#include <omp.h> 
#include <random>
#include <chrono>
//并行特殊高斯消去 dynamic版本
//guided版本和dynamic结构非常相似，只需要将dynamic换成guided即可
//因此不额外创建guided版本并发布了
using namespace std;
const int n = 512;  // 列数
const int m = 512;  // 被消元行行数
const int p = 64;   // 消元行行数
const int NUM_THREADS = 6;
int global_i;
int R[n][n];
int E[n][n];
int FirstNum;

typedef struct
{
    int t_id;
} threadParam_t;

int lp(int* A)
{
    int x = n - 1;
    int count = 0;
    while (!A[count])
    {
        x--;
        count++;
    }
    return x;
}

void threadFunc(int t_id)
{
    int avgnum = n / NUM_THREADS;
    int begin = t_id * avgnum;
    int end = (t_id + 1) * avgnum;
    if (t_id == NUM_THREADS - 1)
        end = n;

    int32x4_t vtE;
    int32x4_t vtR;
    for (int i = global_i; i < m; i++)
    {
        for (int j = begin; j + 4 < end; j += 4)
        {
            vtE = vld1q_s32(&E[i][j]);
            vtR = vld1q_s32(&R[FirstNum][j]);
            vtE = veorq_s32(vtE, vtR);
            vst1q_s32(&E[i][j], vtE);
        }
        for (int j = max(end - 4, begin); j < end; j++)
        {
            E[i][j] = E[i][j] ^ R[FirstNum][j];
        }
    }
}

int main()
{
    // 初始化
    srand((unsigned int)time(NULL)); // 产生随机种子

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            R[i][j] = 0;
            E[i][j] = 0;
        }
    }

    for (int i = 0; i < m; i++)
    {
        for (int j = 0; j < n; j++)
        {
            E[i][j] = rand() & 1;
        }
    }

    for (int i = 0; i < p; i++)
    {
        int NowRow = rand() % n;
        R[NowRow][n - NowRow - 1] = 1;
        for (int j = n - NowRow; j < n; j++)
        {
            R[NowRow][j] = rand() & 1;
        }
    }

    auto start = chrono::high_resolution_clock::now();

#pragma omp parallel num_threads(NUM_THREADS)
{
    int t_id = omp_get_thread_num();
#pragma omp for schedule(dynamic)// by dynamic
    for (int i = global_i; i < m; i++)
    {
        threadFunc(t_id);
    }
}


    auto end = chrono::high_resolution_clock::now();
    auto diff = chrono::duration_cast<chrono::duration<double, milli>>(end - start);
    cout << "Size = " << n << ": "
        << diff.count() << "ms" << endl;
    cout << endl;
    system("pause");
    return 0;
}
