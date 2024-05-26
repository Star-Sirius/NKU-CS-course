#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include<chrono>

using namespace std;
//静态线程 + barrier版本
const int n = 512;
float A[n][n];
const int NUM_THREADS = 6;

//和三重循环很像，将信号量换成barrier罢了
pthread_barrier_t barrier_Divsion;
pthread_barrier_t barrier_Elimination;


typedef struct {
    int t_id;
} threadParam_t;

// id不同，需要处理的部分不同
void* threadFunc(void* param) {
    // 传参
    threadParam_t* p = (threadParam_t*)param;
    int t_id = p->t_id;

    for (int k = 0; k < n; k++) {
        // 除法部分：id为0的线程进行除法操作,其他线程等待
        //考虑到除法部分进行线程分配的开销较大，并且目的仅为优化
        //一个O(n)级的部分，此处不进行多线程除法的编程实现
        if (t_id == 0) {
            for (int j = k + 1; j < n; j++) {
                A[k][j] /= A[k][k];
            }
            A[k][k] = 1.0;

        }
        //第一个同步点
        pthread_barrier_wait(&barrier_Divsion);

        for (int i = k + 1 + t_id; i < n; i += NUM_THREADS) {
            for (int j = k + 1; j < n; j++) {
                A[i][j] -= A[i][k] * A[k][j];
            }
            A[i][k] = 0;
        }
        // 第二个同步点
        pthread_barrier_wait(&barrier_Elimination);
    }
    pthread_exit(NULL);
    return 0;
}

int main()
{
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < i; j++)
            A[i][j] = 0;
        A[i][i] = 1;
        for (int j = i + 1; j < n; j++)
            A[i][j] = double(rand() % 10);
    }
    for (int k = 0; k < n; k++)
        for (int i = k + 1; i < n; i++)
            for (int j = 0; j < n; j++)
                A[i][j] += A[k][j];
    //初始化矩阵，采用三角矩阵+一系列行列变换

    auto start = chrono::high_resolution_clock::now();

    //初始化 barrier
    pthread_barrier_init(&barrier_Divsion, NULL, NUM_THREADS);
    pthread_barrier_init(&barrier_Elimination, NULL, NUM_THREADS);


    pthread_t handles[NUM_THREADS];
    threadParam_t param[NUM_THREADS];
    //创建对应的Handle和线程数据结构
    for (int t_id = 0; t_id < NUM_THREADS; t_id++)
    {
        param[t_id].t_id = t_id;
        pthread_create(&handles[t_id], NULL, threadFunc, (void*)(&param[t_id]));
    }


    for (int t_id = 0; t_id < NUM_THREADS; t_id++)
        pthread_join(handles[t_id], NULL);

    // 销毁障碍
    pthread_barrier_destroy(&barrier_Divsion);
    pthread_barrier_destroy(&barrier_Elimination);

    auto end = chrono::high_resolution_clock::now();
    auto diff = chrono::duration_cast<chrono::duration<double, milli>>(end - start);
    cout << "Size = " << n << ": " << diff.count() << "ms" << endl;
    cout << endl;
    system("pause");

    return 0;
}
