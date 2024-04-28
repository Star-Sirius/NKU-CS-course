#include<iostream>
#include <iomanip>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <pmmintrin.h>
#include <emmintrin.h>
#include <tmmintrin.h>
#include <chrono>
using namespace std;
void* aligned_malloc(size_t size, size_t alignment)
{
	size_t offset = alignment - 1 + sizeof(void*);
	void * originalP = malloc(size + offset);
	size_t originalLocation = reinterpret_cast<size_t>(originalP);
	size_t realLocation = (originalLocation + offset) & ~(alignment - 1);
	void * realP = reinterpret_cast<void*>(realLocation);
	size_t originalPStorage = realLocation - sizeof(void*);
	*reinterpret_cast<void**>(originalPStorage) = originalP;
	return realP;
}

void aligned_free(void* p)
{
	size_t originalPStorage = reinterpret_cast<size_t>(p) - sizeof(void*);
	free(*reinterpret_cast<void**>(originalPStorage));
}
const int n[10] = {96,192,304,400,496,608,704,800,896,992};
const int m[10] = {96,192,304,400,496,608,704,800,896,992};
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
	int *R = (int*) aligned_malloc(16*n[k]*n[k],16);
	int *E = (int*) aligned_malloc(16*n[k]*n[k],16);
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
		__m128i IsNullHelp=_mm_set1_epi32(0);
		__m128i IsNullHelpE;
		for(int j=0;j+4<n[k];j+=4)
		{
			IsNullHelpE = _mm_load_si128((__m128i_u*)&E[i*n[k]+j]);
			IsNullHelp = _mm_or_si128(IsNullHelp,IsNullHelpE);
		}
		IsNullHelp = _mm_hadd_epi32(IsNullHelp,IsNullHelp);
		IsNullHelp = _mm_hadd_epi32(IsNullHelp,IsNullHelp);
		_mm_store_ss((float*)&IsNull,(__m128)IsNullHelp);
		while(IsNull)
		{
			int IsNull2 = 0;
			__m128i IsNull2Help=_mm_set1_epi32(0);
			__m128i IsNull2HelpR;
			for(int j=0;j+4<n[k];j+=4)
			{
				IsNull2HelpR = _mm_load_si128((__m128i_u*)&R[lp(&E[i*n[k]])*n[k]+j]);
				IsNull2Help = _mm_or_si128(IsNull2Help,IsNull2HelpR);
			}
			IsNull2Help = _mm_hadd_epi32(IsNull2Help,IsNull2Help);
		    IsNull2Help = _mm_hadd_epi32(IsNull2Help,IsNull2Help);
			_mm_store_ss((float*)&IsNull2,(__m128)IsNull2Help);
			if(IsNull2)
			{
				int FirstNum = lp(&E[i*n[k]]);
				__m128i vtE;
				__m128i vtR;
				for(int j=0;j+4<n[k];j+=4)
				{
					vtE = _mm_load_si128((__m128i_u*)&E[i*n[k]+j]);
					vtR = _mm_load_si128((__m128i_u*)&R[FirstNum*n[k]+j]);
					vtE = _mm_xor_si128(vtE,vtR);
					_mm_store_si128((__m128i_u*)&E[i*n[k]+j],vtE);
				}
			}
			else
			{
				int FirstNum = lp(&E[i*n[k]]);
				__m128i vtE;
				__m128i vtE2 = _mm_set1_epi32(0);
				for(int j=0;j+4<n[k];j+=4)
				{
					vtE = _mm_load_si128((__m128i_u*)&E[i*n[k]+j]);
					_mm_store_si128((__m128i_u*)&R[FirstNum*n[k]+j],vtE);
					_mm_store_si128((__m128i_u*)&E[i*n[k]+j],vtE2);
				}
				break;
			}

			IsNull = 0;
			IsNullHelp=_mm_set1_epi32(0);
		    for(int j=0;j+4<n[k];j+=4)
			{
				IsNullHelpE = _mm_load_si128((__m128i_u*)&E[i*n[k]+j]);
			    IsNullHelp = _mm_or_si128(IsNullHelp,IsNullHelpE);
			}
		    IsNullHelp = _mm_hadd_epi32(IsNullHelp,IsNullHelp);
		    IsNullHelp = _mm_hadd_epi32(IsNullHelp,IsNullHelp);
			_mm_store_ss((float*)&IsNull,(__m128)IsNullHelp);
		}
	}

	auto t2 = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> fp_ms = t2 - t1;
	auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
	std::chrono::duration<long, std::micro> int_usec = int_ms;
    std::cout <<k+1<<"00:"<< fp_ms.count()/1000.0 << "s";
    cout<<endl;
	aligned_free(E);
	aligned_free(R);
	}
	return 0;
}
