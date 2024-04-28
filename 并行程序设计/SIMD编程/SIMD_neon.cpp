#include<iostream>
#include<arm_neon.h>
#include<time.h>
#define N 1024
using namespace std;
//注意，在本地visual studio上运行不了，去鲲鹏云服务器中运行
void normal_elimination(float** matrix)
{
    for (int k = 0; k < N; k++)
    {
        float tmp = matrix[k][k];
        for (int j = k; j < N; j++)
        {
            matrix[k][j] = matrix[k][j] / tmp;
        }
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


void NEON_elimination(float** matrix)
{
    float32x4_t t1, t2, t3, t4;
    
    float* aligned_rows[N] __attribute__((aligned(128)));// 对矩阵的行指针和列指针按照 128 位对齐存储
    float* aligned_cols[N] __attribute__((aligned(128)));
    for (int i = 0; i < N; i++) {
        aligned_rows[i] = (float*)(((unsigned long)(matrix[i]) + 127) & ~127);
    }
    for (int j = 0; j < N; j++) {
        float* orig_col_ptr = &(matrix[0][j]);
        aligned_cols[j] = (float*)(((unsigned long)(orig_col_ptr)+127) & ~127);
    }

    for (int k = 0; k < N; k++)
    {
        float tmp[4] __attribute__((aligned(128))) = { matrix[k][k], matrix[k][k], matrix[k][k], matrix[k][k] };
        t1 = vld1q_f32(tmp);

        for (int j = N - 4; j >= k; j -= 4)
        {
            t2 = vld1q_f32(aligned_rows[k] + j);
            t3 = vdivq_f32(t2, t1);
            vst1q_f32(aligned_rows[k] + j, t3);
        }

        if (k % 4 != (N % 4))
        {
            for (int j = k; j % 4 != (N % 4); j++)
            {
                matrix[k][j] = matrix[k][j] / tmp[0];
            }
        }

        for (int i = k + 1; i < N; i++)
        {
            float tmp[4] __attribute__((aligned(128))) = { matrix[i][k], matrix[i][k], matrix[i][k], matrix[i][k] };
            t1 = vld1q_f32(tmp);
            for (int j = N - 4; j > k; j -= 4)
            {
                t2 = vld1q_f32(aligned_rows[i] + j);
                t3 = vld1q_f32(aligned_rows[k] + j);
                t4 = vsubq_f32(t2, vmulq_f32(t1, t3));
                vst1q_f32(aligned_rows[i] + j, t4);
            }

            for (int j = k + 1; j % 4 != (N % 4); j++)
            {
                matrix[i][j] = matrix[i][j] - matrix[i][k] * matrix[k][j];
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
    float** matrix2 = new float* [N];



    for (int i = 0; i < N; i++)
    {
        matrix[i] = new float[N];
        matrix2[i] = new float[N];
    }//绝不能共地址



    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            matrix[i][j] = rand() % 100;
            matrix2[i][j] = matrix[i][j];
        }
    }


    cout << "串行高斯消去法" << endl;
    clock_t  clockBegin, clockEnd;
    clockBegin = clock(); //开始计时

    normal_elimination(matrix);

    clockEnd = clock();

    cout << "总共耗时： " << clockEnd - clockBegin << "ms" << endl;



    cout << endl << endl<< "使用NEON并行的高斯消去法" << endl;

    clockBegin = clock(); //开始计时

    NEON_elimination(matrix2);

    clockEnd = clock();

    cout << "总共耗时： " << clockEnd - clockBegin << "ms" << endl;


    return 0;

}

