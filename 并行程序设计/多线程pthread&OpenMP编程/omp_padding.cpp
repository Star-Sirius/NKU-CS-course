#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <pmmintrin.h>
#include <omp.h>

using namespace std;

#define CACHE_LINE_SIZE 64

const int N = 1024;

float A[N][N];
float re_A[N][N];

const int thread_count = 6;

void init_mat(float re_A[][N])
{
    srand((unsigned)time(NULL));
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            re_A[i][j] = rand() / 100;
}

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

struct Padding {
    char _padding[CACHE_LINE_SIZE];
};

void padding_omp(float A[][N])
{
#pragma omp parallel num_threads(thread_count)
    {
        struct Padding padding; // 在共享访问的变量间填充结构体，大小等于cache行

        for (int k = 0; k < N; k++)
        {
#pragma omp for schedule(dynamic, 24)
            for (int j = k + 1; j < N; j++)
            {
                float temp = A[k][k];
                A[k][j] = A[k][j] / temp;
            }

#pragma omp barrier

            A[k][k] = 1.0;

#pragma omp for schedule(dynamic, 24)
            for (int i = k + 1; i < N; i++)
            {
                for (int j = k + 1; j < N; j++)
                {
                    float temp = A[i][k];
                    A[i][j] = A[i][j] - temp * A[k][j];
                }

                A[i][k] = 0;
            }
        }
    }
}
int main()
{
    init_mat(re_A);
    start = chrono::high_resolution_clock::now();
    padding_omp(re_A);

    end = chrono::high_resolution_clock::now();
    diff = chrono::duration_cast<chrono::duration<double, milli>>(end - start);
    cout << "omp by dynamic,add padding ,size= " << N << ": " << diff.count() << "ms" << endl;
    cout << endl;

    system("pause");
    return 0;
}
