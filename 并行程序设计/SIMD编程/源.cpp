#include <iostream>
#include <immintrin.h> // AVX头文件
#include <immintrin.h> // AVX头文件

#include<pmmintrin.h>
#include<xmmintrin.h> //SSE头文件
#include<xmmintrin.h> //SSE头文件

#include <stdlib.h>
#include <cstdlib>

#include <chrono>      // 时间库
#include <chrono>      // 时间库
#include<ctime>

//AVX_Adv 和SSE_Adv是相应的对齐算法
using namespace std;
const int N = 100;

void normal_elimitation(float** matrix) //串行高斯消去法
void normal_elimitation(float** matrix) //串行高斯消去法
{
    for (int k = 0; k < N; k++)
    {
    }
}

void AVX_elimitation(float** matrix) //AVX并行的高斯消去法
void AVX_elimitation(float** matrix) //AVX并行的高斯消去法
{
    __m256 t1, t2, t3, t4;//数据类型float
    __m256 t1, t2, t3, t4;//数据类型float
    for (int k = 0; k < N; k++)
    {
        float tmp[8] = { matrix[k][k], matrix[k][k], matrix[k][k], matrix[k][k], matrix[k][k], matrix[k][k], matrix[k][k], matrix[k][k] };
        t1 = _mm256_loadu_ps(tmp);  //加载到寄存器中，不要求地址是32字节对齐
        for (int j = N - 8; j >= k; j -= 8) //从后向前每次取八个
        t1 = _mm256_loadu_ps(tmp);  //加载到寄存器中，不要求地址是32字节对齐
        for (int j = N - 8; j >= k; j -= 8) //从后向前每次取八个
        {
            t2 = _mm256_loadu_ps(matrix[k] + j);
            t3 = _mm256_div_ps(t2, t1);    //除法，向量化处理使每次能除八个
            _mm256_storeu_ps(matrix[k] + j, t3);   //存储到内存中，不要求地址对齐
            t3 = _mm256_div_ps(t2, t1);    //除法，向量化处理使每次能除八个
            _mm256_storeu_ps(matrix[k] + j, t3);   //存储到内存中，不要求地址对齐
        }
        if (k % 8 != (N % 8)) //处理不能被8整除的元素
        if (k % 8 != (N % 8)) //处理不能被8整除的元素
        {
            for (int j = k + 1; j % 8 != (N % 8); j++)
            {
            {
                t2 = _mm256_loadu_ps(matrix[i] + j);
                t3 = _mm256_loadu_ps(matrix[k] + j);
                t4 = _mm256_sub_ps(t2, _mm256_mul_ps(t1, t3)); //减法
                t4 = _mm256_sub_ps(t2, _mm256_mul_ps(t1, t3)); //减法
                _mm256_storeu_ps(matrix[i] + j, t4);
            }
            for (int j = k + 1; j % 8 != (N % 8); j++)
}
void AVX_elimitation_Adv(float** matrix)
{
    const int alignment = 32; // AVX指令需要32字节对齐
    const int alignment = 32; // AVX指令需要32字节对齐
    __m256 t1, t2, t3, t4;
    for (int k = 0; k < N; k++)
    {
}


void SSE_elimitation(float** matrix) //SSE并行的高斯消去法
{//修改:必须显式地设置为零，否则会因为数过小导致较大误差，主对角线上的元素不是1等问题
    __m128 t1, t2, t3, t4;//数据类型float
    //在行标准化过程中，t1用来储存对角线元素进行除法，t2用来加载矩阵中元素作为被除数，t3作为除法结果load回矩阵中
    //在消去过程中，根据原始公式 matrix[i][j] = matrix[i][j] - matrix[i][k] * matrix[k][j]，
    //t2加载matrix[i][j]，t1加载matrix[i][k]，t3加载matrix[k][j]，t4保存加法结果load回矩阵中
void SSE_elimitation(float** matrix) //SSE并行的高斯消去法
{//修改:必须显式地设置为零，否则会因为数过小导致较大误差，主对角线上的元素不是1等问题
    __m128 t1, t2, t3, t4;//数据类型float
    //在行标准化过程中，t1用来储存对角线元素进行除法，t2用来加载矩阵中元素作为被除数，t3作为除法结果load回矩阵中
    //t2加载matrix[i][j]，t1加载matrix[i][k]，t3加载matrix[k][j]，t4保存加法结果load回矩阵中
    for (int k = 0; k < N; k++)
    {
        /*float tmp[4] = { matrix[k][k], matrix[k][k], matrix[k][k], matrix[k][k] };
        t1 = _mm_loadu_ps(tmp);*/  //加载到寄存器中，不要求地址是16字节对齐
        t1 = _mm_loadu_ps(tmp);*/  //加载到寄存器中，不要求地址是16字节对齐

        t1 = _mm_load1_ps(&matrix[k][k]);
        for (int j = N - 4; j >= k; j -= 4) //从后向前每次取四个
        for (int j = N - 4; j >= k; j -= 4) //从后向前每次取四个
        {
            t2 = _mm_load_ps(matrix[k] + j);
            t3 = _mm_div_ps(t2, t1);    //除法，向量化处理使每次能除四个
            t3 = _mm_div_ps(t2, t1);    //除法，向量化处理使每次能除四个
            _mm_store_ps(matrix[k] + j, t3);   
            //存储到内存中，考虑到该循环是从后往前的，按照数组在底层地址中存储结构，这里使用对齐和不对齐都可以，实际测试也是如此
            //存储到内存中，考虑到该循环是从后往前的，按照数组在底层地址中存储结构，这里使用对齐和不对齐都可以，实际测试也是如此
        }
        if (k % 4 != (N % 4)) //处理不能被4整除的元素
        if (k % 4 != (N % 4)) //处理不能被4整除的元素
        {
            for (int j = k + 1; j % 4 != (N % 4); j++)
            {
            {
                t2 = _mm_load_ps(matrix[i] + j);
                t3 = _mm_load_ps(matrix[k] + j);
                t4 = _mm_sub_ps(t2, _mm_mul_ps(t1, t3)); //减法
                t4 = _mm_sub_ps(t2, _mm_mul_ps(t1, t3)); //减法
                _mm_store_ps(matrix[i] + j, t4);
            }
            for (int j = k + 1; j % 4 != (N % 4); j++)
        }
    }
}

//照着伪代码翻译的，比较通用
void Gauss_SSE(float **matrix) {
    for (int k = 0; k < N; k++) {
        __m128 v1 = _mm_set1_ps(matrix[k][k]);
}

void SSE_elimitation_Adv(float** A)
//对齐升级：注意到在普通的SSE中是从后往前进行并行化处理，
//但[考虑cache特性] ，这里从前往后进行，先用while判断模四剩余后的元素，再直接用load指令处理
//对齐升级：注意到在普通的SSE中是从后往前进行并行化处理，
//但[考虑cache特性] ，这里从前往后进行，先用while判断模四剩余后的元素，再直接用load指令处理
{
    __m128 t1, t2, t3, t4;
    //在行标准化过程中，t1用来储存对角线元素进行除法，t2用来加载矩阵中元素作为被除数，t3作为除法结果load回矩阵中
    //在行标准化过程中，t1用来储存对角线元素进行除法，t2用来加载矩阵中元素作为被除数，t3作为除法结果load回矩阵中
    for (int k = 0; k < N; k++) {

        t1 = _mm_load1_ps(&A[k][k]);  // 复制4份A[k][k]存进vt中
        t1 = _mm_load1_ps(&A[k][k]);  // 复制4份A[k][k]存进vt中
        int j = k + 1;


            _mm_store_ps(&A[k][j], t3);
        }

        // 处理剩下的元素，直接串行除
        // 处理剩下的元素，直接串行除
        for (; j < N; j++) {
            A[k][j] = A[k][j] / A[k][k];
        }
        A[k][k] = 1.0;
        //在消去过程中，根据原始公式 matrix[i][j] = matrix[i][j] - matrix[i][k] * matrix[k][j]，
        //t2加载matrix[i][j]，t1加载matrix[i][k]，t3加载matrix[k][j]，t4保存加法结果load回矩阵中
        //t2加载matrix[i][j]，t1加载matrix[i][k]，t3加载matrix[k][j]，t4保存加法结果load回矩阵中
        for (int i = k + 1; i < N; i++) {

             t1 = _mm_load1_ps(&A[i][k]);
            int j = k + 1;

            // A[k][j]、A[i][j]需要内存对齐
            // A[k][j]、A[i][j]需要内存对齐
            while (j % 4 != 0) {
                A[i][j] = A[i][j] - A[k][j] * A[i][k];
                j++;
            }
            for (; j + 4 <= N; j += 4) {
                // 原始公式：A[i][j] = A[i][j] - A[k][j]*A[i][k];
                // 原始公式：A[i][j] = A[i][j] - A[k][j]*A[i][k];
                 t3 = _mm_load_ps(&A[k][j]);
                 t2 = _mm_load_ps(&A[i][j]);
                 t4 = _mm_sub_ps(t2, _mm_mul_ps(t1, t3));
                _mm_store_ps(&A[i][j], t4);
            }

            // 剩下的元素
            // 剩下的元素
            for (; j < N; j++) {
                A[i][j] = A[i][j] - A[k][j] * A[i][k];
            }
}


void print(float** matrix) //输出
void print(float** matrix) //输出
{
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
        matrix3[i] = new float[N];
        matrix4[i] = new float[N];
        matrix5[i] = new float[N];
    }//绝不能共地址
    }//绝不能共地址

    for (int i = 0; i < N; i++)
    {
        }
    }

    cout <<"串行高斯消去法" << endl;
    cout <<"串行高斯消去法" << endl;

    clock_t  clockBegin, clockEnd;

    clockBegin = clock(); //开始计时
    clockBegin = clock(); //开始计时

    normal_elimitation(matrix);

    clockEnd = clock();
    //print(matrix);

    cout << "总共耗时： " << clockEnd - clockBegin << "ms" << endl;
    cout << "总共耗时： " << clockEnd - clockBegin << "ms" << endl;


    cout << endl << endl << endl << "使用AVX并行的高斯消去法" << endl;
    cout << endl << endl << endl << "使用AVX并行的高斯消去法" << endl;

    clockBegin = clock(); //开始计时
    clockBegin = clock(); //开始计时

    AVX_elimitation(matrix2);

    clockEnd = clock();
    //print(matrix2);
    cout << "总共耗时： " << clockEnd - clockBegin << "ms" << endl;
    cout << "总共耗时： " << clockEnd - clockBegin << "ms" << endl;

    cout << endl << endl << endl << "使用AVX_Adv并行的高斯消去法" << endl;
    cout << endl << endl << endl << "使用AVX_Adv并行的高斯消去法" << endl;

    clockBegin = clock(); //开始计时
    clockBegin = clock(); //开始计时

    AVX_elimitation_Adv(matrix5);

    clockEnd = clock();
    //print(matrix5);
    cout << "总共耗时： " << clockEnd - clockBegin << "ms" << endl;
    cout << "总共耗时： " << clockEnd - clockBegin << "ms" << endl;


    cout << endl << endl << endl << "使用SSE并行的高斯消去法" << endl;
    cout << endl << endl << endl << "使用SSE并行的高斯消去法" << endl;

    clockBegin = clock(); //开始计时
    clockBegin = clock(); //开始计时

    SSE_elimitation(matrix3);

    clockEnd = clock();
    //print(matrix3);
    cout << "总共耗时： " << clockEnd - clockBegin << "ms" << endl;
    cout << "总共耗时： " << clockEnd - clockBegin << "ms" << endl;

    cout << endl << endl << endl << "使用SSE指令对齐并行的高斯消去法" << endl;
    cout << endl << endl << endl << "使用SSE指令对齐并行的高斯消去法" << endl;

    clockBegin = clock(); //开始计时
    clockBegin = clock(); //开始计时

    SSE_elimitation_Adv(matrix4);

    clockEnd = clock();
    //print(matrix4);
    cout << "总共耗时： " << clockEnd - clockBegin << "ms" << endl;
    cout << "总共耗时： " << clockEnd - clockBegin << "ms" << endl;
    return 0;

}
