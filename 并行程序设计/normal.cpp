#include <iostream>
#include <chrono>      // 时间库
#include<ctime>

using namespace std;
const int N = 1024;
void normal_elimitation(float** matrix) //串行高斯消去法
{
    for (int k = 0; k < N; k++)
    {
        float tmp = matrix[k][k];
        for (int j = k + 1; j < N; j++)
        {
            matrix[k][j] = matrix[k][j] / tmp;
        }
        matrix[k][k] = 1.0;
        for (int i = k + 1; i < N; i++)
        {
            float tmp2 = matrix[i][k];
            for (int j = k + 1; j < N; j++)
            {
                matrix[i][j] = matrix[i][j] - tmp2 * matrix[k][j];
            }
            matrix[i][k] = 0;
        }
    }
}
void print(float** matrix) //输出
{
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            cout << matrix[i][j] << " ";
        }
        cout << endl;
    }
}

int main()
{
    srand((unsigned)time(NULL));
    float** matrix = new float* [N];
    for (int i = 0; i < N; i++)
        matrix[i] = new float[N];

    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            matrix[i][j] = rand() % 100;
    //print(matrix);


    cout << "串行高斯消去法" << endl;

    clock_t  clockBegin, clockEnd;

    clockBegin = clock(); //开始计时
    auto start = std::chrono::high_resolution_clock::now();//两种时间库

    normal_elimitation(matrix);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    clockEnd = clock();

    std::cout << "代码执行时间: " << duration.count() << " ms" << std::endl;

    cout << "总共耗时： " << clockEnd - clockBegin << "ms" << endl;
    //print(matrix);
}

