#include<iostream>
#include <iomanip>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <chrono>
using namespace std;
const int n[10] = {100,200,300,400,500,600,700,800,900,1000};
const int m[10] = {100,200,300,400,500,600,700,800,900,1000};
const int p[10] = {10,20,30,40,50,60,70,80,90,100};
//Note:the dataset is created by random 0/1,not from dataset provided.
int k;
int lp(int *A)
{
	int x=n[k]-1;
	int count=0;
	while(!A[count])
	{
		x--;
		count++;
	}
	return x;
}
int main()
{
	for(k=0;k<10;k++)
	{
	int R[n[k]][n[k]];
	int E[n[k]][n[k]];
	for(int i=0;i<n[k];i++)
		for(int j=0;j<n[k];j++)
		{
			R[i][j] = 0;
			E[i][j] = 0;
		}
	for(int i=0;i<m[k];i++)
		for(int j=n[k]-p[k];j<n[k];j++)
		{
			E[i][j] = 1;
		}
    for(int i=0;i<p[k];i++)
	{
		R[i][n[k]-i-1] = 1;
	}

	
	auto t1 = std::chrono::high_resolution_clock::now();

	int IsNull=0;
	for(int i=0;i<m[k];i++)
	{
		for(int j=0;j<n[k];j++)
			IsNull = IsNull | E[i][j];
		while(IsNull)
		{
			int IsNull2 = 0;
			for(int j=0;j<n[k];j++)
				IsNull2 = IsNull2 | R[lp(E[i])][j];
			if(IsNull2)
			{
				int FirstNum = lp(E[i]);
				for(int j=0;j<n[k];j++)
					E[i][j] = E[i][j] ^ R[FirstNum][j];
			}
			else
			{
				int FirstNum = lp(E[i]);
				for(int j=0;j<n[k];j++)
				{
				    R[FirstNum][j] = E[i][j];
					E[i][j] = 0;
				}
				break;
			}
			IsNull = 0;
			for(int j=0;j<n[k];j++)
			    IsNull = IsNull | E[i][j];
		}
	}
	auto t2 = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> fp_ms = t2 - t1;
	auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
	std::chrono::duration<long, std::micro> int_usec = int_ms;
    std::cout <<k+1<<"00:"<< fp_ms.count()/1000.0 << "s";
    cout<<endl;
	}
	return 0;
}
