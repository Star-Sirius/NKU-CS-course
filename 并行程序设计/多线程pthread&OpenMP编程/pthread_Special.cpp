#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <arm_neon.h>

#include <random>
#include <chrono>
//并行特殊高斯消去+neon版本
using namespace std;
const int n = 512;//列数
const int m = 512;//被消元行行数
const int p = 64;//消元行行数
const int NUM_THREADS = 6;
int global_i;
int R[n][n];
int E[n][n];
int FirstNum;
sem_t sem_main;
sem_t sem_worker[NUM_THREADS];
typedef struct
{
    int t_id;
}threadParam_t;

int lp(int *A)
{
	int x=n-1;
	int count=0;
	while(!A[count])
	{
		x--;
		count++;
	}
	return x;
}

void *threadFunc(void *param)
{
	threadParam_t *p = (threadParam_t*)param;
	int t_id = p->t_id;
	while(global_i<m)
	{
	    sem_wait(&sem_worker[t_id]);
	    int avgnum = n/NUM_THREADS;
	    int begin = t_id * avgnum;
	    int end = (t_id+1) * avgnum;
	    if(t_id==NUM_THREADS-1)
		    end = n;
		int32x4_t vtE;
		int32x4_t vtR;
		int j;
		for(j=begin;j+4<end;j+=4)
		{//采用穿插列处理
			vtE = vld1q_s32(&E[global_i][j]);
			vtR = vld1q_s32(&R[FirstNum][j]);
			vtE = veorq_s32(vtE,vtR);
			vst1q_s32(&E[global_i][j],vtE);
		}
		for(;j<end;j++)//对剩余列进行处理
			E[global_i][j] = E[global_i][j] ^ R[FirstNum][j];
		sem_post(&sem_main);
	}
	pthread_exit(nullptr);
	return 0;
}
int main()
{
	//初始化
	srand((unsigned int)time(NULL));  // 产生随机种子  把0换成NULL也行

	for(int i=0;i<n;i++)
		for(int j=0;j<n;j++)
		{
			R[i][j] = 0;
			E[i][j] = 0;
		}
	for(int i=0;i<m;i++)
		for(int j=0;j<n;j++)
			E[i][j] = rand() & 1;
		
        for(int i=0;i<p;i++)
  	    {
		    int NowRow = rand()%n;
		    R[NowRow][n-NowRow-1] = 1;
		    for(int j=n-NowRow;j<n;j++)
			R[NowRow][j] = rand() & 1;
	    }
   
	auto start = chrono::high_resolution_clock::now();


	sem_init(&sem_main,0,0);
	for(int i=0;i<NUM_THREADS;i++)
	{
		sem_init(&sem_worker[i],0,0);
	}
	pthread_t handles[NUM_THREADS];
    threadParam_t param[NUM_THREADS];

	for(int t_id=0;t_id<NUM_THREADS;t_id++)
        {
            param[t_id].t_id = t_id;
        }

    for(int t_id=0;t_id<NUM_THREADS;t_id++)
        pthread_create(&handles[t_id],NULL,threadFunc,(void*)(&param[t_id]));

	int IsNull=0;
	for(global_i=0;global_i<m;global_i++)
	{
		int IsNull = 0;
		int32x4_t IsNullHelp=vdupq_n_s32(0);
		int32x4_t IsNullHelpE; 
		for(int j=0;j+4<n;j+=4)
		{
			IsNullHelpE = vld1q_s32(&E[global_i][j]);
			IsNullHelp = vorrq_s32(IsNullHelp,IsNullHelpE);
		}
		IsNullHelp = vpaddq_s32(IsNullHelp,IsNullHelp);
		IsNullHelp = vpaddq_s32(IsNullHelp,IsNullHelp);
		IsNull = vgetq_lane_s32(IsNullHelp,0);
		while(IsNull)
		{
			FirstNum = lp(E[global_i]);
			int IsNull2 = 0;
			int32x4_t IsNull2Help=vdupq_n_s32(0);
			int32x4_t IsNull2HelpR;
			for(int j=0;j+4<n;j+=4)
			{
				IsNull2HelpR = vld1q_s32(&R[FirstNum][j]);
				IsNull2Help = vorrq_s32(IsNull2Help,IsNull2HelpR);
			}
			IsNull2Help = vpaddq_s32(IsNull2Help,IsNull2Help);
		    IsNull2Help = vpaddq_s32(IsNull2Help,IsNull2Help);
		    IsNull2 = vgetq_lane_s32(IsNull2Help,0);
			if(IsNull2)
			{
				for(int i=0;i<NUM_THREADS;i++)
					sem_post(&sem_worker[i]);
				for(int i=0;i<NUM_THREADS;i++)
					sem_wait(&sem_main);
			}
			else
			{
				int FirstNum = lp(E[global_i]);
				int32x4_t vtE;
				int32x4_t vtE2 = vdupq_n_s32(0);
				for(int j=0;j+4<n;j+=4)
				{
					vtE = vld1q_s32(&E[global_i][j]);
					vst1q_s32(&R[FirstNum][j],vtE);
					vst1q_s32(&E[global_i][j],vtE2);
				}
				break;
			}

			IsNull = 0;
			IsNullHelp=vdupq_n_s32(0);
		    for(int j=0;j+4<n;j+=4)
			{
				IsNullHelpE = vld1q_s32(&E[global_i][j]);
			    IsNullHelp = vorrq_s32(IsNullHelp,IsNullHelpE);
			}
		    IsNullHelp = vpaddq_s32(IsNullHelp,IsNullHelp);
		    IsNullHelp = vpaddq_s32(IsNullHelp,IsNullHelp);
		    IsNull = vgetq_lane_s32(IsNullHelp,0);
		}
		
	}
	for(int t_id=0;t_id<NUM_THREADS;t_id++)
	    pthread_cancel(handles[t_id]);
	for(int t_id=0;t_id<NUM_THREADS;t_id++)
            pthread_join(handles[t_id],NULL);

	sem_destroy(&sem_main);
	for(int i=0;i<NUM_THREADS;i++)
		sem_destroy(&sem_worker[i]);

	auto end = chrono::high_resolution_clock::now();
	auto diff = chrono::duration_cast<chrono::duration<double, milli>>(end - start);
	cout << "Size = " << n << ": " << diff.count() << "ms" << endl;
	cout << endl;
	system("pause");
    
    return 0;
}
