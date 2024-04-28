#include<iostream>
#include <iomanip>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <arm_neon.h>
#include <chrono>
using namespace std;
const int n[10] = {100,200,300,400,500,600,700,800,900,1000};//列数
const int m[10] = {100,200,300,400,500,600,700,800,900,1000};//被消元行行数
const int p[10] = {10,20,30,40,50,60,70,80,90,100};//消元行行数
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
		for(int j=0;j<p[k];j++)
		{
			E[i][j] = 1;
		}
    for(int i=0;i<p[k];i++)
	{
		R[i][i] = 1;
	}

	auto t1 = std::chrono::high_resolution_clock::now();
	for(int i=0;i<m[k];i++)
	{
		int IsNull = 0;
		int32x4_t IsNullHelp=vdupq_n_s32(0);
		int32x4_t IsNullHelpE; 
		for(int j=0;j+4<n[k];j+=4)
		{
			IsNullHelpE = vld1q_s32(&E[i][j]);
			IsNullHelp = vorrq_s32(IsNullHelp,IsNullHelpE);
		}
		IsNullHelp = vpaddq_s32(IsNullHelp,IsNullHelp);
		IsNullHelp = vpaddq_s32(IsNullHelp,IsNullHelp);
		IsNull = vgetq_lane_s32(IsNullHelp,0);
		while(IsNull)
		{
			int IsNull2 = 0;
			int32x4_t IsNull2Help=vdupq_n_s32(0);
			int32x4_t IsNull2HelpR;
			for(int j=0;j+4<n[k];j+=4)
			{
				IsNull2HelpR = vld1q_s32(&R[lp(E[i])][j]);
				IsNull2Help = vorrq_s32(IsNull2Help,IsNull2HelpR);
			}
			IsNull2Help = vpaddq_s32(IsNull2Help,IsNull2Help);
		    IsNull2Help = vpaddq_s32(IsNull2Help,IsNull2Help);
		    IsNull2 = vgetq_lane_s32(IsNull2Help,0);
			if(IsNull2)
			{
				int FirstNum = lp(E[i]);
				int32x4_t vtE;
				int32x4_t vtR;
				for(int j=0;j+4<n[k];j+=4)
				{
					vtE = vld1q_s32(&E[i][j]);
					vtR = vld1q_s32(&R[FirstNum][j]);
					vtE = veorq_s32(vtE,vtR);
					vst1q_s32(&E[i][j],vtE);
				}
			}
			else
			{
				int FirstNum = lp(E[i]);
				int32x4_t vtE;
				int32x4_t vtE2 = vdupq_n_s32(0);
				for(int j=0;j+4<=n[k];j+=4)
				{
					vtE = vld1q_s32(&E[i][j]);
					vst1q_s32(&R[FirstNum][j],vtE);
					vst1q_s32(&E[i][j],vtE2);
				}
				break;
			}

			IsNull = 0;
			IsNullHelp=vdupq_n_s32(0);
		    for(int j=0;j+4<n[k];j+=4)
			{
				IsNullHelpE = vld1q_s32(&E[i][j]);
			    IsNullHelp = vorrq_s32(IsNullHelp,IsNullHelpE);
			}
		    IsNullHelp = vpaddq_s32(IsNullHelp,IsNullHelp);
		    IsNullHelp = vpaddq_s32(IsNullHelp,IsNullHelp);
		    IsNull = vgetq_lane_s32(IsNullHelp,0);
		}
	}

	auto t2 = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> fp_ms = t2 - t1;
	auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
	std::chrono::duration<long, std::micro> int_usec = int_ms;
    std::cout <<m+1<<"00:"<< fp_ms.count()/1000.0 << "s";
    cout<<endl;
	}
	return 0;
}
