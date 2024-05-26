#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <chrono>
#include <arm_neon.h>
using namespace std;
//在pthead框架下加入了neon行内并行运算，具体参见 *threadFunc()部分
//注意编译指令 omp+neon:
//clang++ - g - march = armv8 - a - fopenmp neon_pthread.cpp - o neon_pthread.o
const int n=512;
float A[n][n];
const int NUM_THREADS=7;
sem_t sem_main;
sem_t sem_workerstart[NUM_THREADS];
sem_t sem_workerend[NUM_THREADS];
typedef struct
{
    int t_id;
}threadParam_t;

void *threadFunc(void *param)
{
    threadParam_t *p = (threadParam_t*)param;
    int t_id = p->t_id;
	for(int k=0;k<n;k++)
	{
		int avgnum = (n-k-1)/NUM_THREADS;
    	int begin = k+1+avgnum*t_id;
	    int end = (k+1)+avgnum*(t_id+1);
	    if(t_id==6)
		    end = n;
		sem_wait(&sem_workerstart[t_id]);
	    for(int i=begin;i<end;i++)
	    {
            float32x4_t vaik=vdupq_n_f32(A[i][k]);
            for(int j=k+1;j+4<n;j+=4)
            {
                float32x4_t vakj=vld1q_f32(&A[k][j]);
                float32x4_t vaij=vld1q_f32(&A[i][j]);
                float32x4_t vx;
                vx=vmulq_f32(vakj,vaik);
                vaij=vsubq_f32(vaij,vx);
                vst1q_f32(&A[i][j],vaij);
            }
            A[i][k] = 0.0;
	    }
		sem_post(&sem_main);
		sem_wait(&sem_workerend[t_id]);

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


    auto start = chrono::high_resolution_clock::now();

	sem_init(&sem_main,0,0);
	for(int i=0;i<NUM_THREADS;i++)
	{
		sem_init(&sem_workerstart[i],0,0);
	    sem_init(&sem_workerend[i],0,0);
	}
	pthread_t handles[NUM_THREADS];
    threadParam_t param[NUM_THREADS];
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
			sem_post(&sem_workerstart[i]);
		for(int i=0;i<NUM_THREADS;i++)
			sem_wait(&sem_main);
		for(int i=0;i<NUM_THREADS;i++)
			sem_post(&sem_workerend[i]);
    }

	for(int t_id=0;t_id<NUM_THREADS;t_id++)
            pthread_join(handles[t_id],NULL);

	sem_destroy(&sem_main);
	for(int i=0;i<NUM_THREADS;i++)
	{
	    sem_destroy(&sem_workerstart[i]);
	    sem_destroy(&sem_workerend[i]);
	}

    auto end = chrono::high_resolution_clock::now();
    auto diff = chrono::duration_cast<chrono::duration<double, milli>>(end - start);
    cout << "Size = " << n << ": " << diff.count() << "ms" << endl;
    cout << endl;
    return 0;
}
