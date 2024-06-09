#include <iostream>
#include <mpi.h>
#include <sys/time.h>
#include <arm_neon.h>
#include <omp.h>

using namespace std;

static const int N = 10;//change here to test different martix size
static const int thread_count = 4;//线程数，归OpenMP管
//Num_Proc is defined according to bash,change bash to test different processes

float arr[N][N];
float A[N][N];

void init_A(float arr[][N])
{
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < i; j++)
            A[i][j] = 0;
        A[i][i] = 1;
        for (int j = i + 1; j < N; j++)
            A[i][j] = double(rand() % 10);
    }
    for (int k = 0; k < N; k++)
        for (int i = k + 1; i < N; i++)
            for (int j = 0; j < N; j++)
                A[i][j] += A[k][j];


}




void reset_A(float A[][N], float arr[][N])
{
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            A[i][j] = arr[i][j];
}


void Show(float A[][N])
{
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
            cout << A[i][j] << " ";
        cout << endl;
    }
    cout << endl;
}

void Normal_Gauss()
{
    reset_A(A, arr);
    timeval t_start;
    timeval t_end;
    gettimeofday(&t_start, NULL);

    for (int k = 0; k < N; k++)
    {
        for (int j = k + 1; j < N; j++)
        {
            A[k][j] = A[k][j] * 1.0 / A[k][k];
        }
        A[k][k] = 1.0;

        for (int i = k + 1; i < N; i++)
        {
            for (int j = k + 1; j < N; j++)
            {
                A[i][j] = A[i][j] - A[i][k] * A[k][j];
            }
            A[i][k] = 0;
        }
    }
    gettimeofday(&t_end, NULL);
    cout << "ordinary time cost: "
        << 1000 * (t_end.tv_sec - t_start.tv_sec) +
        0.001 * (t_end.tv_usec - t_start.tv_usec) << "ms" << endl;
}



void LU(float A[][N], int rank, int Num_Proc)
{
    int block = N / Num_Proc;
    int remain = N % Num_Proc;
    //经典块划分
    int begin = rank * block;
    int end = rank != Num_Proc - 1 ? begin + block : begin + block + remain;

    for (int k = 0; k < N; k++)
    {

        if (k >= begin && k < end)
        {
            //在块内则运算，不在块内则接收信息
            for (int j = k + 1; j < N; j++)
                A[k][j] = A[k][j] / A[k][k];
            A[k][k] = 1.0;


            for (int i = 0; i < Num_Proc; i++)
                if (i != rank)
                    MPI_Send(&A[k], N, MPI_FLOAT, i, 2, MPI_COMM_WORLD);
                //把该行数据全部广播，标签注意不要和mpi_Block()中重复，保证正确匹配

        }
        else
        {
            int My_Proc = k / block;
            MPI_Recv(&A[k], N, MPI_FLOAT, My_Proc, 2,
            MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }


        for (int i = begin; i < end && i < N; i++)
        {
            if (i >= k + 1)
            {
                for (int j = k + 1; j < N; j++)
                    A[i][j] = A[i][j] - A[i][k] * A[k][j];
                A[i][k] = 0.0;
            }
        }
    }
}


void mpi_Block()
{

    timeval t_start;
    timeval t_end;

    int Num_Proc;
    int rank;

    MPI_Comm_size(MPI_COMM_WORLD, &Num_Proc);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int block = N / Num_Proc;
    int remain = N % Num_Proc;
    //块划分，如果是主线程则进行任务分配并传输信息
    //如果是其他线程接收任务，即接收信息
    if (rank == 0)
    {
        reset_A(A, arr);
        gettimeofday(&t_start, NULL);

        for (int i = 1; i < Num_Proc; i++)
        {
            if (i != Num_Proc - 1)
            {
                for (int j = 0; j < block; j++)
                    MPI_Send(&A[i * block + j], N, MPI_FLOAT, i, 0, MPI_COMM_WORLD);
            }//需要判断是不是最后一个进程，有剩余行 i表示接收方进程标识，0是标签
            else
            {
                for (int j = 0; j < block + remain; j++)
                    MPI_Send(&A[i * block + j], N, MPI_FLOAT, i, 0, MPI_COMM_WORLD);
            }
        }
        LU(A, rank, Num_Proc);

        for (int i = 1; i < Num_Proc; i++)
        {   //计算结束后接收来自其他进程的任务
            if (i != Num_Proc - 1)
            {
                for (int j = 0; j < block; j++)
                    MPI_Recv(&A[i * block + j], N, MPI_FLOAT, i, 1,
                        MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            else
            {
                for (int j = 0; j < block + remain; j++)
                    MPI_Recv(&A[i * block + j], N, MPI_FLOAT, i, 1,
                        MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }
        //主进程的任务量还是比其他进程稍大，需要接收所有进程信息。
        //而如果不接收只输出的话也没有意义，因此程序运算的结果必然需要整合
        gettimeofday(&t_end, NULL);
        cout << "Block Normal MPI  time cost: "
            << 1000 * (t_end.tv_sec - t_start.tv_sec) +
            0.001 * (t_end.tv_usec - t_start.tv_usec) << "ms" << endl;
        Show(A);
    }

    //其他线程，接收任务，在运算结束后回发任务
    else
    {

        if (rank != Num_Proc - 1)
        {
            for (int j = 0; j < block; j++)
                MPI_Recv(&A[rank * block + j], N, MPI_FLOAT, 0, 0,
                    MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        else
        {
            for (int j = 0; j < block + remain; j++)
                MPI_Recv(&A[rank * block + j], N, MPI_FLOAT, 0, 0,
                    MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        LU(A, rank, Num_Proc);

        if (rank != Num_Proc - 1)
        {
            for (int j = 0; j < block; j++)
                MPI_Send(&A[rank * block + j], N, MPI_FLOAT, 0, 1, MPI_COMM_WORLD);
        }
        else
        {
            for (int j = 0; j < block + remain; j++)
                MPI_Send(&A[rank * block + j], N, MPI_FLOAT, 0, 1, MPI_COMM_WORLD);
        }
    }
}


void Block_Final(float A[][N], int rank, int num_proc)
{
    float32x4_t t1, t2, t3;
    //t123命名及操作和SIMD完全相同
    int block = N / num_proc;
    int remain = N % num_proc;
    int begin = rank * block;
    int end = rank != num_proc - 1 ? begin + block : begin + block + remain;
#pragma omp parallel num_threads(thread_count),private(t1, t2, t3)
    //声明t123为线程内部的私有变量
    for (int k = 0; k < N; k++)
    {
        if (k >= begin && k < end)
        {
            float akk[4] = { A[k][k], A[k][k], A[k][k], A[k][k] };
            t1 = vld1q_f32(akk);
#pragma omp for schedule(dynamic,20)
            for (int j = k + 1; j < N - 3; j += 4)
            {
                t2 = vld1q_f32(A[k] + j);
                t3 = vdivq_f32(t2, t1);
                vst1q_f32(A[k] + j, t3);
            }
            for (int j = N - N % 4; j < N; j++)
            {
                A[k][j] = A[k][j] / A[k][k];
            }
            A[k][k] = 1.0;
            for (int i = rank + 1; i < num_proc; i++)
                MPI_Send(&A[k], N, MPI_FLOAT, i, 2, MPI_COMM_WORLD);
        }
        else
        {
            int My_Proc = k / block;
            if (My_Proc < rank)
                MPI_Recv(&A[k], N, MPI_FLOAT, My_Proc, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        for (int i = begin; i < end && i < N; i++)
        {
            if (i >= k + 1)
            {
                float temp2[4] = { A[i][k], A[i][k], A[i][k], A[i][k] };
                t1 = vld1q_f32(temp2);
#pragma omp for schedule(dynamic,20)
                for (int j = k + 1; j <= N - 3; j += 4)
                {
                    t2 = vld1q_f32(A[i] + j);
                    t3 = vld1q_f32(A[k] + j);
                    t3 = vmulq_f32(t1, t3);
                    t2 = vsubq_f32(t2, t3);
                    vst1q_f32(A[i] + j, t2);
                }
                for (int j = N - N % 4; j < N; j++)
                    A[i][j] = A[i][j] - A[i][k] * A[k][j];
                A[i][k] = 0;
            }
        }
    }
}


void Block_Final()
{
    //完全和mpi_Block()相同，因为深化并行修改的是LU函数内部
    timeval t_start;
    timeval t_end;

    int Num_Proc;
    int rank;

    MPI_Comm_size(MPI_COMM_WORLD, &Num_Proc);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int block = N / Num_Proc;
    int remain = N % Num_Proc;

    if (rank == 0)
    {
        reset_A(A, arr);
        gettimeofday(&t_start, NULL);

        for (int i = 1; i < Num_Proc; i++)
        {
            if (i != Num_Proc - 1)
            {
                for (int j = 0; j < block; j++)
                    MPI_Send(&A[i * block + j], N, MPI_FLOAT, i, 0, MPI_COMM_WORLD);
            }
            else
            {
                for (int j = 0; j < block + remain; j++)
                    MPI_Send(&A[i * block + j], N, MPI_FLOAT, i, 0, MPI_COMM_WORLD);
            }
        }
        Block_Final(A, rank, Num_Proc);

        for (int i = 1; i < Num_Proc; i++)
        {
            if (i != Num_Proc - 1)
            {
                for (int j = 0; j < block; j++)
                    MPI_Recv(&A[i * block + j], N, MPI_FLOAT, i, 1,
                        MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            else
            {
                for (int j = 0; j < block + remain; j++)
                    MPI_Recv(&A[i * block + j], N, MPI_FLOAT, i, 1,
                        MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }
        gettimeofday(&t_end, NULL);
        cout << "Block MPI LU with Neon and OpenMP dynamic time cost: "
            << 1000 * (t_end.tv_sec - t_start.tv_sec) +
            0.001 * (t_end.tv_usec - t_start.tv_usec) << "ms" << endl;
        Show(A);
    }
    //主进程的任务量还是比其他进程稍大，需要接收所有进程信息。
    //而如果不接收只输出的话也没有意义，因此程序运算的结果必然需要整合

    else
    {

        if (rank != Num_Proc - 1)
        {
            for (int j = 0; j < block; j++)
                MPI_Recv(&A[rank * block + j], N, MPI_FLOAT, 0, 0,
                    MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        else
        {
            for (int j = 0; j < block + remain; j++)
                MPI_Recv(&A[rank * block + j], N, MPI_FLOAT, 0, 0,
                    MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        Block_Final(A, rank, Num_Proc);

        if (rank != Num_Proc - 1)
        {
            for (int j = 0; j < block; j++)
                MPI_Send(&A[rank * block + j], N, MPI_FLOAT, 0, 1, MPI_COMM_WORLD);
        }
        else
        {
            for (int j = 0; j < block + remain; j++)
                MPI_Send(&A[rank * block + j], N, MPI_FLOAT, 0, 1, MPI_COMM_WORLD);
        }
    }
}


int main()
{
    init_A(arr);

    Normal_Gauss();

    MPI_Init(NULL, NULL);

    mpi_Block();
    Block_Final();
    MPI_Finalize();

}
