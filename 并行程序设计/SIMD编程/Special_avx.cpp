#include<iostream>
#include <iomanip>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <pmmintrin.h>
#include <emmintrin.h>
#include <tmmintrin.h>
#include <immintrin.h>
#include <chrono>
using namespace std;
const int n[10] = {100,200,300,400,500,600,700,800,900,1000};
const int m[10] = {100,200,300,400,500,600,700,800,900,1000};
const int p[10] = {10,20,30,40,50,60,70,80,90,100};
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
	int R[n[k]*n[k]];
	int E[n[k]*n[k]];
	for(int i=0;i<n[k];i++)
		for(int j=0;j<n[k];j++)
		{
			R[i*n[k]+j] = 0;
			E[i*n[k]+j] = 0;
		}
	for(int i=0;i<m[k];i++)
		for(int j=n[k]-p[k];j<n[k];j++)
		{
			E[i*n[k]+j] = 1;
		}
    for(int i=0;i<p[k];i++)
	{
		R[i*n[k]+n[k]-i-1] = 1;
	}

	
	auto t1 = std::chrono::high_resolution_clock::now();

	int IsNull=0;
	for(int i=0;i<m[k];i++)
	{
		int IsNull = 0;
		__m256i IsNullHelp=_mm256_set1_epi32(0);
		__m256i IsNullHelpE;
		__m128i IsNullHelp1;
		__m128i IsNullHelp2;
		for(int j=0;j+8<n[k];j+=8)
		{
			IsNullHelpE = _mm256_loadu_si256((__m256i_u*)&E[i*n[k]+j]);
			IsNullHelp = _mm256_or_si256(IsNullHelp,IsNullHelpE);
		}
		IsNullHelp1 = _mm256_extractf128_si256(IsNullHelp,0);
		IsNullHelp2 = _mm256_extractf128_si256(IsNullHelp,1);
		IsNullHelp1 = _mm_hadd_epi32(IsNullHelp1,IsNullHelp2);
		IsNullHelp1 = _mm_hadd_epi32(IsNullHelp1,IsNullHelp1);
		IsNullHelp1 = _mm_hadd_epi32(IsNullHelp1,IsNullHelp1);
		_mm_store_ss((float*)&IsNull,(__m128)IsNullHelp1);
		while(IsNull)
		{
			int IsNull2 = 0;
			__m256i IsNull2Help=_mm256_set1_epi32(0);
			__m256i IsNull2HelpR;
			__m128i IsNull2Help1;
			__m128i IsNull2Help2;
			for(int j=0;j+8<n[k];j+=8)
			{
				IsNull2HelpR = _mm256_loadu_si256((__m256i_u*)&R[lp(&E[i*n[k]])*n[k]+j]);
				IsNull2Help = _mm256_or_si256(IsNull2Help,IsNull2HelpR);
			}
			IsNull2Help1 = _mm256_extractf128_si256(IsNull2Help,0);
		    IsNull2Help2 = _mm256_extractf128_si256(IsNull2Help,1);
		    IsNull2Help1 = _mm_hadd_epi32(IsNull2Help1,IsNull2Help2);
		    IsNull2Help1 = _mm_hadd_epi32(IsNull2Help1,IsNull2Help1);
		    IsNull2Help1 = _mm_hadd_epi32(IsNull2Help1,IsNull2Help1);
		    _mm_store_ss((float*)&IsNull2,(__m128)IsNull2Help1);
			if(IsNull2)
			{
				int FirstNum = lp(&E[i*n[k]]);
				__m256i vtE;
				__m256i vtR;
				for(int j=0;j+8<n[k];j+=8)
				{
					vtE = _mm256_loadu_si256((__m256i_u*)&E[i*n[k]+j]);
					vtR = _mm256_loadu_si256((__m256i_u*)&R[FirstNum*n[k]+j]);
					vtE = _mm256_xor_si256(vtE,vtR);
					_mm256_storeu_si256((__m256i_u*)&E[i*n[k]+j],vtE);
				}
			}
			else
			{
				int FirstNum = lp(&E[i*n[k]]);
				__m256i vtE;
				__m256i vtE2 = _mm256_set1_epi32(0);
				for(int j=0;j+8<n[k];j+=8)
				{
					vtE = _mm256_loadu_si256((__m256i_u*)&E[i*n[k]+j]);
					_mm256_storeu_si256((__m256i_u*)&R[FirstNum*n[k]+j],vtE);
					_mm256_storeu_si256((__m256i_u*)&E[i*n[k]+j],vtE2);
				}
				break;
			}

			IsNull = 0;
			IsNullHelp=_mm256_set1_epi32(0);
		    for(int j=0;j+8<n[k];j+=8)
			{
				IsNullHelpE = _mm256_loadu_si256((__m256i_u*)&E[i*n[k]+j]);
			    IsNullHelp = _mm256_or_si256(IsNullHelp,IsNullHelpE);
			}
		    IsNullHelp1 = _mm256_extractf128_si256(IsNullHelp,0);
		    IsNullHelp2 = _mm256_extractf128_si256(IsNullHelp,1);
		    IsNullHelp1 = _mm_hadd_epi32(IsNullHelp1,IsNullHelp2);
		    IsNullHelp1 = _mm_hadd_epi32(IsNullHelp1,IsNullHelp1);
		    IsNullHelp1 = _mm_hadd_epi32(IsNullHelp1,IsNullHelp1);
		    _mm_store_ss((float*)&IsNull,(__m128)IsNullHelp1);
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
