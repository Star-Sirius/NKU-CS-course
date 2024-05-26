#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <chrono>
using namespace std;
const int n=512;
float A[n][n];
int NUM_THREADS=7;
//动态线程版本，还是1+7=8
typedef struct
{
    int k;
    int t_id;
}threadParam_t;

void *threadFunc(void *param)
{
    threadParam_t *p = (threadParam_t*)param;
    int k = p->k;
    int t_id = p->t_id;
    int i = k + t_id + 1;
	int avgnum = (n-k-1)/NUM_THREADS;
	int begin = k+1+avgnum*t_id;
	int end = (k+1)+avgnum*(t_id+1);
	if(t_id==6)
		end = n;
	for(int i=begin;i<end;i++)
	{
        for(int j=k+1;j<n;j++)
        {
            A[i][j] = A[i][j] - A[i][k] * A[k][j];
        }
        A[i][k] = 0;
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

    for(int k=0;k<n;k++)
    {
        for(int j=k+1;j<n;j++)
            A[k][j] = A[k][j] / A[k][k];
        A[k][k] = 1.0;
        int worker_count = 7;
        pthread_t handles[NUM_THREADS];
        threadParam_t param[NUM_THREADS];
        for(int t_id=0;t_id<worker_count;t_id++)
        {
            param[t_id].k = k;
            param[t_id].t_id = t_id;
        }
        for(int t_id=0;t_id<worker_count;t_id++)
            pthread_create(&handles[t_id],NULL,threadFunc,(void*)(&param[t_id]));
        //构建线程：我们可以看到在伪代码中，动态线程是在for(int k=0;k<n;k++)中建立的，这意味额外开销会很大
        //而相对应的，动态线程没有信号量参与。
        /* 静态线程的create是在  
        for (int t_id = 0; t_id < NUM_THREADS; t_id++)
        {
            param[t_id].t_id = t_id;
            pthread_create(&handles[t_id], NULL, threadFunc, (void*)(&param[t_id]));
        }中构建的，需要同步量来进行控制与主线程间的互动 */
        for(int t_id=0;t_id<worker_count;t_id++)
            pthread_join(handles[t_id],NULL);
    }


    auto end = chrono::high_resolution_clock::now();
    auto diff = chrono::duration_cast<chrono::duration<double, milli>>(end - start);
    cout << "Size = " << n << ": " << diff.count() << "ms" << endl;
    cout << endl;
    system("pause");

    return 0;
}

