#include <iostream>
#include <Windows.h>
using namespace std;


int main()
{
    const int n=1000 //后续通过修改n的值测试得到不同n下的实验数据
    // 初始化数组
    int A[n];
    int sum = 0;
    for (int i = 0; i < n; i++)
    {
      A[i] = i;
    }
  
  // 并行计算n个数的和
#pragma omp parallel
{
  int id = omp_get_thread_num () ; // 获取线程编号
  int num_threads = omp_get_num_threads () ; // 获取线程数量
  int chunk_size = N / num_threads ; // 每个线程处理的数据块大小
  int start = id ∗ chunk_size ; // 计算起始位置
  int end = start + chunk_size ; // 计算结束位置
  int local_sum = 0; // 定义局部和变量
 

    // 使用 QueryPerformanceCounter 函数计时
    LARGE_INTEGER freq, start, end;
    double time;
    QueryPerformanceFrequency(&freq); // 获取计时器频率
    QueryPerformanceCounter(&start); // 获取开始时间
    
    for ( int i = 0; i < N; i++)
        sum += A[ i ] ;
}

    // 获取结束时间并计算时间差
    QueryPerformanceCounter(&end);
    time = static_cast<double>(end.QuadPart - start.QuadPart) / freq.QuadPart;
    cout << "Time elapsed by parallel: " << time *1000<< endl;
    double temp = time;


    QueryPerformanceCounter(&start); // 获取开始时间
    // 计算每个线程分配的数据块中的和
   for ( int i = start ; i < end ; i++) 
  {
       local_sum += A[i] ;
  }
   // 合并每个线程计算的局部和
 #pragma omp critical
 sum1 += local_sum ;
 

    QueryPerformanceCounter(&end);
    time = static_cast<double>(end.QuadPart - start.QuadPart) / freq.QuadPart;
    cout << "Time elapsed by normal: " << time*1000 << endl;
    cout << temp / time;
    return 0;
}
