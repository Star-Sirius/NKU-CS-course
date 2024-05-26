//静态线程+信号量,垂直划分
#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <chrono>
using namespace std;
const int n = 512;
float A[n][n];
//7+1=8，共有8个线程
const int NUM_THREADS = 7;

//信号量定义
sem_t sem_leader;
sem_t sem_Divsion[NUM_THREADS];
sem_t sem_Elimination[NUM_THREADS];
//线程数据结构定义：
typedef struct
{
    int t_id; //线程ID
}threadParam_t;
//线程函数定义
void* threadFunc(void* param)
{
    threadParam_t* p = (threadParam_t*)param;
    int t_id = p->t_id;
    for (int k = 0; k < n; k++)
    {
        int avgnum = (n - k - 1) / NUM_THREADS;
        int begin = k + 1 + avgnum * t_id;
        int end = (k + 1) + avgnum * (t_id + 1);
        if (t_id == 6)
            end = n;
        //初始化工作区域
        sem_wait(&sem_Divsion[t_id]);  // 阻塞，等待主线完成除法操作
        for (int i = k + 1; i < n; i++)
        {
            for (int j = begin; j < end; j++)
            {
                A[i][j] = A[i][j] - A[i][k] * A[k][j];
            }
        }
        sem_post(&sem_leader); // 唤醒主线程
        sem_wait(&sem_Elimination[t_id]); //阻塞，等待主线程唤醒进入下一轮

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


    auto start = chrono::high_resolution_clock::now();

    //初始化信号量
    sem_init(&sem_leader, 0, 0);
    for (int i = 0; i < NUM_THREADS; i++)
    {
        sem_init(&sem_Divsion[i], 0, 0);
        sem_init(&sem_Elimination[i], 0, 0);
    }
    //创建线程 
    pthread_t handles[NUM_THREADS];// 创建对应的 Handle
    threadParam_t param[NUM_THREADS];// 创建对应的线程数据结构
    for (int t_id = 0; t_id < NUM_THREADS; t_id++)
    {
        param[t_id].t_id = t_id;
        pthread_create(&handles[t_id], NULL, threadFunc, (void*)(&param[t_id]));
    }
    for (int k = 0; k < n; k++)
    {
        //主线程 乘法操作
        for (int j = k + 1; j < n; j++)
            A[k][j] = A[k][j] / A[k][k];
        A[k][k] = 1.0;

        for (int i = 0; i < NUM_THREADS; i++)
            sem_post(&sem_Divsion[i]);//唤醒工作线程
        for (int i = 0; i < NUM_THREADS; i++)
            sem_wait(&sem_leader); //主线程睡眠
        for (int i = k + 1; i < n; i++)
            A[i][k] = 0;
        for (int i = 0; i < NUM_THREADS; i++)
            sem_post(&sem_Elimination[i]);//主线程再次唤醒工作线程进入下一轮消去
    }

    for (int t_id = 0; t_id < NUM_THREADS; t_id++)
        pthread_join(handles[t_id], NULL);

    sem_destroy(&sem_leader);
    for (int i = 0; i < NUM_THREADS; i++)
    {
        sem_destroy(&sem_Divsion[i]);
        sem_destroy(&sem_Elimination[i]);
    }//销毁所有信号量

    auto end = chrono::high_resolution_clock::now();
    auto diff = chrono::duration_cast<chrono::duration<double, milli>>(end - start);
    cout << "Size = " << n << ": " << diff.count() << "ms" << endl;
    cout << endl;
    system("pause");


    return 0;
}
