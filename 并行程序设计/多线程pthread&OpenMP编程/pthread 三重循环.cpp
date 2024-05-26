#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include<chrono>

using namespace std;
//静态线程 + 信号量同步 + 三重循环全部纳入线程函数版本
//将id=0的零号线程视为主线程，在线程函数中，主线程id匹配后进行除法，
//运算结束后发信号，全部线程进入消去部分
const int n = 512;
float A[n][n];
const int NUM_THREADS = 7;
sem_t sem_leader;
sem_t sem_Divsion[NUM_THREADS - 1];
sem_t sem_Elimination[NUM_THREADS - 1];


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
        else {
            // 其余线程先等待
            sem_wait(&sem_Divsion[t_id - 1]);
        }
        // 除法完成,唤醒其他线程
        if (t_id == 0) {
            for (int i = 0; i < NUM_THREADS - 1; i++) {
                sem_post(&sem_Divsion[i]);
            }
        }

        //消去部分：所有线程都进行除法。
        //注意，在这步中采用了之前提到的穿插划分，即不需要再计算begin,end
        //块起始和块终止行号

        for (int i = k + 1 + t_id; i < n; i += NUM_THREADS) {
            for (int j = k + 1; j < n; j++) {
                A[i][j] -= A[i][k] * A[k][j];
            }
            A[i][k] = 0;
        }
        // 如果是主线程，接收各线程传输信号
        if (t_id == 0) {
            // 等待其余工作线程结束消去操作
            for (int i = 0; i < NUM_THREADS - 1; i++) {
                sem_wait(&sem_leader);
            }
            // 通知消去完成
            for (int i = 0; i < NUM_THREADS - 1; i++) {
                sem_post(&sem_Elimination[i]);
            }
        }
        else {
            // 通知主线程
            sem_post(&sem_leader);
            // 等待主线程的通知，进入下一轮
            sem_wait(&sem_Elimination[t_id - 1]);
        }
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


    sem_init(&sem_leader, 0, 0);
    for (int i = 0; i < NUM_THREADS; i++)
    {
        sem_init(&sem_Divsion[i], 0, 0);
        sem_init(&sem_Elimination[i], 0, 0);
    }//主线程和工作线程初始化

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


    sem_destroy(&sem_leader);
    for (int i = 0; i < NUM_THREADS; i++)
    {
        sem_destroy(&sem_Divsion[i]);
        sem_destroy(&sem_Elimination[i]);
    }
    //销毁信号量

    auto end = chrono::high_resolution_clock::now();
    auto diff = chrono::duration_cast<chrono::duration<double, milli>>(end - start);
    cout << "Size = " << n << ": " << diff.count() << "ms" << endl;
    cout << endl;
    system("pause");

    return 0;
}
