#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <pmmintrin.h>
#include <omp.h>
#include <chrono>
using namespace std;

const int N = 1024;

float A[N][N];
float re_A[N][N];

const int NUM_THREADS = 6;


void init_mat(float re_A[][N])
{
	srand((unsigned)time(NULL));
	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
			re_A[i][j] = rand() / 100;
}

void reset_matrix(float A[][N], float re_A[][N])
{
	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
			A[i][j] = re_A[i][j];
}

void Normal_Gauss(float A[][N])
{
	for (int k = 0; k < N; k++)
	{
		for (int j = k + 1; j < N; j++)
			A[k][j] = A[k][j] / A[k][k];
		A[k][k] = 1.0;
		for (int i = k + 1; i < N; i++)
		{
			for (int j = k + 1; j < N; j++)
				A[i][j] = A[i][j] - A[i][k] * A[k][j];
			A[i][k] = 0;
		}
	}
}

void omp_static(float A[][N])
{
#pragma omp parallel num_threads(NUM_THREADS)
	for (int k = 0; k < N; k++)
	{
#pragma omp for schedule(static)
		for (int j = k + 1; j < N; j++)
		{
			A[k][j] = A[k][j] / A[k][k];
		}
		A[k][k] = 1.0;
#pragma omp for schedule(static)
		for (int i = k + 1; i < N; i++)
		{
			for (int j = k + 1; j < N; j++)
				A[i][j] = A[i][j] - A[i][k] * A[k][j];
			A[i][k] = 0;
		}
	}
}

void omp_dynamic(float A[][N])
{
#pragma omp parallel num_threads(NUM_THREADS)
	for (int k = 0; k < N; k++)
	{
#pragma omp for schedule(dynamic, 24)
		for (int j = k + 1; j < N; j++)
		{
			A[k][j] = A[k][j] / A[k][k];
		}
		A[k][k] = 1.0;
#pragma omp for schedule(dynamic, 24)
		for (int i = k + 1; i < N; i++)
		{
			for (int j = k + 1; j < N; j++)
				A[i][j] = A[i][j] - A[i][k] * A[k][j];
			A[i][k] = 0;
		}
	}
}

void omp_guided(float mat[][N])
{
#pragma omp parallel num_threads(NUM_THREADS)
	for (int k = 0; k < N; k++)
	{
#pragma omp for schedule(guided, 24)
		for (int j = k + 1; j < N; j++)
		{
			mat[k][j] = mat[k][j] / mat[k][k];
		}
		mat[k][k] = 1.0;
#pragma omp for schedule(guided, 24)
		for (int i = k + 1; i < N; i++)
		{
			for (int j = k + 1; j < N; j++)
				mat[i][j] = mat[i][j] - mat[i][k] * mat[k][j];
			mat[i][k] = 0;
		}
	}
}

void omp_dynamicSSE(float A[][N])
{
	__m128 t1, t2, t3;
#pragma omp parallel num_threads(NUM_THREADS)
	for (int k = 0; k < N; k++)
	{
		//除法部分使用SIMD中的SSE指令集部分实现，单个线程
		//由于除法会有剩余几个元素出现，而#pragma omp for schedule(dynamic, 24)
		//控制私有变量的出现，因此使用SIMD+Pthread无法处理剩余部分，该函数后附有一报错代码为例
		t1 = _mm_load1_ps(&A[k][k]);
		int j = k + 1;
		for (j; j < N - 3; j += 4)
		{
			t2 = _mm_loadu_ps(A[k] + j);
			t3 = _mm_div_ps(t2, t1);
			_mm_storeu_ps(A[k] + j, t3);
		}
		for (j; j < N; j++)
		{
			A[k][j] = A[k][j] / A[k][k];
		}
		A[k][k] = 1.0;


#pragma omp for schedule(dynamic, 24)
		for (int i = k + 1; i < N; i++)
		{
			t1= _mm_load1_ps(&A[i][k]);
			j = k + 1;
			for (j; j <= N - 3; j += 4)
			{
				t2 = _mm_loadu_ps(A[i] + j);
				t3 = _mm_loadu_ps(A[k] + j);
				t3 = _mm_mul_ps(t1, t3);
				t2 = _mm_sub_ps(t2, t3);
				_mm_storeu_ps(A[i] + j, t2);
			}
			for (j; j < N; j++)
				A[i][j] = A[i][j] - A[i][k] * A[k][j];
			A[i][k] = 0;
		}
	}
}
/*报错部分如下：
int j = k + 1;
for (j; j < N - 3; j += 4)
{
	t2 = _mm_loadu_ps(mat[k] + j);
	t3 = _mm_div_ps(t2, t1);
	_mm_storeu_ps(mat[k] + j, t3);
}
#pragma omp for schedule(dynamic, 24)
for (j; j < N; j++)
{
	mat[k][j] = mat[k][j] / mat[k][k];
}
mat[k][k] = 1.0;
报错原因：expected iteration declaration or initialization for (; j < N; j++)
*/

void print_mat(float A[][N])
{
	if (N > 16)
		return;
	cout << endl;
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
			cout << A[i][j] << " ";
		cout << endl;
	}
	cout << endl;
}

int main()
{
	init_mat(re_A);
	reset_matrix(A, re_A);
	//将矩阵re_A中的元素赋值给A，以下消去时用矩阵A进行消去
	auto start = chrono::high_resolution_clock::now();

	Normal_Gauss(A);

	auto end = chrono::high_resolution_clock::now();
	auto diff = chrono::duration_cast<chrono::duration<double, milli>>(end - start);
	cout << "normal_Guass ,size= " << N << ": " << diff.count() << "ms" << endl;
	cout << endl;

	print_mat(A);

	reset_matrix(A, re_A);
	start = chrono::high_resolution_clock::now();

	omp_static(A);

	end = chrono::high_resolution_clock::now();
	diff = chrono::duration_cast<chrono::duration<double, milli>>(end - start);
	cout << "omp by static ,size= " << N << ": " << diff.count() << "ms" << endl;
	cout << endl;

	print_mat(A);

	reset_matrix(A, re_A);

	start = chrono::high_resolution_clock::now();
	omp_dynamic(A);

	end = chrono::high_resolution_clock::now();
	diff = chrono::duration_cast<chrono::duration<double, milli>>(end - start);
	cout << "omp by dynamic ,size= " << N << ": " << diff.count() << "ms" << endl;
	cout << endl;

	print_mat(A);

	reset_matrix(A, re_A);

	start = chrono::high_resolution_clock::now();

	omp_guided(A);

	end = chrono::high_resolution_clock::now();
	diff = chrono::duration_cast<chrono::duration<double, milli>>(end - start);
	cout << "omp by guided ,size= " << N << ": " << diff.count() << "ms" << endl;
	cout << endl;

	print_mat(A);

	reset_matrix(A, re_A);

	start = chrono::high_resolution_clock::now();

	omp_dynamicSSE(A);

	end = chrono::high_resolution_clock::now();
	diff = chrono::duration_cast<chrono::duration<double, milli>>(end - start);
	cout << "omp by dynamic+SSE ,size= " << N << ": " << diff.count() << "ms" << endl;
	cout << endl;

	print_mat(A);

	system("pause");
	cout << endl;
	return 0;
}
