#include <iostream>
#include <Windows.h>
using namespace std;


int main()
{
    const int n=500;
    // 初始化矩阵，向量和内积数组
    int a[n][n];
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            a[i][j] = i + j;
        }
    }
    int v[n];
    for (int i = 0; i < n; i++)
    {
        v[i] = i+1;
    }
    int sum1[n],sum2[n];//sum1 for parallel ,sum2 for normal
    for (int i = 0; i < n; i++) {
        sum1[i] = 0;
        sum2[i] = 0;
    }

    // 使用 QueryPerformanceCounter 函数计时
    LARGE_INTEGER freq, start, end;
    double time;
    QueryPerformanceFrequency(&freq); // 获取计时器频率
    QueryPerformanceCounter(&start); // 获取开始时间

    // 求和_parallel
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            sum1[j] += a[i][j] * v[i];
        }
    }

    // 获取结束时间并计算时间差
    QueryPerformanceCounter(&end);
    time = static_cast<double>(end.QuadPart - start.QuadPart) / freq.QuadPart;
    cout << "Time elapsed by parallel: " << time *1000<< endl;
    double temp = time;


    QueryPerformanceCounter(&start); // 获取开始时间
    // 求和_normal
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            sum2[i] += a[j][i] * v[j];
        }
    }

    QueryPerformanceCounter(&end);
    time = static_cast<double>(end.QuadPart - start.QuadPart) / freq.QuadPart;
    cout << "Time elapsed by normal: " << time*1000 << endl;
    cout << temp / time;
    return 0;
}
