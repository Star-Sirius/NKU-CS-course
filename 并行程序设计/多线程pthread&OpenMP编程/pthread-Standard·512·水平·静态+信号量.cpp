#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include<chrono>

using namespace std;
//NOTICE1:
//水平和垂直的主要差异在线程函数threadFunc中，在工作线程处理的嵌套循环中
//水平（按行运算）中，外循环在begin--end代码块中按行递增（即begin,end是行标），
//内循环从k+1到n递增，运算第i行列下标从k+1到n-1的数组元素
//垂直（按列运算）中begin,end是列标，嵌套循环中，外循环在k+1到n-1行中遍历
//内循环在begin列到end列之间进行消去
//控制变量：矩阵运算方式分为水平和垂直，版本为 静态线程 + 信号量同步

//NOTICE2:线程数的变化取决于const int NUM_THREADS，对其进行更改，数据测量即可。
//NOTICE3:数据规模的变化取决于const int n，对其更改，测量即可。
//因此不再给出变更线程数和矩阵规模 的代码，以此标准版为主。

const int n=1024;
float A[n][n];
const int NUM_THREADS=3;
sem_t sem_leader;
sem_t sem_Divsion[NUM_THREADS];
sem_t sem_Elimination[NUM_THREADS];
//定义主线程和3个工作线程，工作开始和结束
typedef struct
{
    int t_id;
}threadParam_t;
//动态线程结构体中需要有 k 消去的轮次，静态线程只有线程id

void *threadFunc(void *param)
{
    threadParam_t *p = (threadParam_t*)param;
    int t_id = p->t_id;
	for(int k=0;k<n;k++)
	{
		int avgnum = (n-k-1)/NUM_THREADS;
    	int begin = k+1+avgnum*t_id;
	    int end = (k+1)+avgnum*(t_id+1);
	    if(t_id==NUM_THREADS - 1)
		    end = n;//如果是最后一个线程，末尾的工作会稍微少一点

		sem_wait(&sem_Divsion[t_id]);
        //等待主线程除法完成信号，进行二重循环消去工作
	    for(int i=begin;i<end;i++)
	    {
            for(int j=k+1;j<n;j++)
            {
                A[i][j] = A[i][j] - A[i][k] * A[k][j];
            }
            A[i][k] = 0;
	    }
		sem_post(&sem_leader);//发送信号给主线程
		sem_wait(&sem_Elimination[t_id]);
        //运算结束，控制权回归主线程，工作线程睡眠
	}
    pthread_exit(NULL);
    return 0;
}
int main()
{
    for(int i=0;i<n;i++)
    {
        for(int j=0;j<i;j++)
            A[i][j] = 0;
        A[i][i] = 1;
        for(int j=i+1;j<n;j++)
            A[i][j] = double(rand()%10);
    }
    for(int k=0;k<n;k++)
        for(int i=k+1;i<n;i++)
            for(int j=0;j<n;j++)
                A[i][j] += A[k][j];
    //初始化矩阵，采用三角矩阵+一系列行列变换

    auto start = chrono::high_resolution_clock::now();


	sem_init(&sem_leader,0,0);
	for(int i=0;i<NUM_THREADS;i++)
	{
		sem_init(&sem_Divsion[i],0,0);
	    sem_init(&sem_Elimination[i],0,0);
	}//主线程和工作线程初始化

	pthread_t handles[NUM_THREADS];
    threadParam_t param[NUM_THREADS];
    //创建对应的Handle和线程数据结构
    for (int t_id = 0; t_id < NUM_THREADS; t_id++)
    {
        param[t_id].t_id = t_id;
        pthread_create(&handles[t_id], NULL, threadFunc, (void*)(&param[t_id]));
    }
    for(int k=0;k<n;k++)
    {
        for(int j=k+1;j<n;j++)
            A[k][j] = A[k][j] / A[k][k];
        A[k][k] = 1.0;
		for(int i=0;i<NUM_THREADS;i++)
			sem_post(&sem_Divsion[i]);
		for(int i=0;i<NUM_THREADS;i++)
			sem_wait(&sem_leader);//主线程等待所有工作线程完成任务
        //（收到足够的信号量来解除阻塞状态）
		for(int i=0;i<NUM_THREADS;i++)
			sem_post(&sem_Elimination[i]);
    }
    //主线程进行完除法操作后，工作线程唤醒，执行消去工作（同时主线程睡眠），
    //结束运算后，主线程向每个工作线程发送信号表示当前行操作完成（下一次循环可以接着做下一行）

	for(int t_id=0;t_id<NUM_THREADS;t_id++)
            pthread_join(handles[t_id],NULL);

	sem_destroy(&sem_leader);
	for(int i=0;i<NUM_THREADS;i++)
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
