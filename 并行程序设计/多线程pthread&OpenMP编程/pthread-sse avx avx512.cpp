#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include<chrono>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <immintrin.h>

using namespace std;
//在pthead框架下加入了sse,avx,avx512指令集 见*threadFunc()部分

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

void *threadFuncAVX(void *param)
{
    threadParam_t *p = (threadParam_t*)param;
    int t_id = p->t_id;
	for(int k=0;k<n;k++)
	{
		int avgnum = (n-k-1)/NUM_THREADS;
    	int begin = k+1+avgnum*t_id;
	    int end = (k+1)+avgnum*(t_id+1);
	    if(t_id==NUM_THREADS - 1)
		    end = n;
		sem_wait(&sem_workerstart[t_id]);
	    for(int i=begin;i<end;i++)
	    {
            __m256 vaik;
            vaik = _mm256_set1_ps(A[i][k]);
            for(int j=k+1;j+8<n;j+=8)
            {
                __m256 vakj;
                __m256 vaij;
                __m256 vx;
                vakj = _mm256_loadu_ps(&A[k][j]);
                vaij = _mm256_loadu_ps(&A[i][j]);
                vx = _mm256_mul_ps(vakj,vaik);
                vaij = _mm256_sub_ps(vaij,vx);
                _mm256_storeu_ps(&A[i][j],vaij);
            }
            A[i][k] = 0.0;
	    }
		sem_post(&sem_main);
		sem_wait(&sem_workerend[t_id]);

	}
    pthread_exit(NULL);
    return 0;
}

void* threadFuncAVX512(void* param)
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
        sem_wait(&sem_workerstart[t_id]);
        for (int i = begin; i < end; i++)
        {
            __m512 vaik;
            vaik = _mm512_set1_ps(A[i][k]);
            for (int j = k + 1; j + 16 < n; j += 16)
            {
                __m512 vakj;
                __m512 vaij;
                __m512 vx;
                vakj = _mm512_loadu_ps(&A[k][j]);
                vaij = _mm512_loadu_ps(&A[i][j]);
                vx = _mm512_mul_ps(vakj, vaik);
                vaij = _mm512_sub_ps(vaij, vx);
                _mm512_storeu_ps(&A[i][j], vaij);
            }
            A[i][k] = 0.0;
        }
        sem_post(&sem_main);
        sem_wait(&sem_workerend[t_id]);

    }
    pthread_exit(NULL);
    return 0;
}

void* threadFuncSSE(void* param)
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
        sem_wait(&sem_workerstart[t_id]);
        for (int i = begin; i < end; i++)
        {
            __m128 vaik;
            vaik = _mm_set1_ps(A[i][k]);
            for (int j = k + 1; j + 4 < n; j += 4)
            {
                __m128 vakj;
                __m128 vaij;
                __m128 vx;
                vakj = _mm_loadu_ps(&A[k][j]);
                vaij = _mm_loadu_ps(&A[i][j]);
                vx = _mm_mul_ps(vakj, vaik);
                vaij = _mm_sub_ps(vaij, vx);
                _mm_storeu_ps(&A[i][j], vaij);
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
        pthread_create(&handles[t_id], NULL, threadFuncSSE, (void*)(&param[t_id]));
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
    system("pause");

    return 0;
}
