#include <iostream>
#include <mpi.h>
#include <sys/time.h>
#include <arm_neon.h>
#include <omp.h>
//compare to mpi(block),we should change both LU part and  mpi_Cycle part 
//mpi-cycle compare with cycle-Final,only need to change LU part
using namespace std;

static const int N = 10;
static const int thread_count = 4;

static int LoopWide = 1;//划分宽度，等于1是为单行循环划分

float arr[N][N];
float A[N][N];//for reset

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



void LU_cycle(float A[][N], int rank, int Num_Proc)
{
    
    int seg = LoopWide * Num_Proc;
    for (int k = 0; k < N; k++)
    {
        //判断当前行是否是自己的任务
        if (int((k % seg) / LoopWide) == rank)
        {
            for (int j = k + 1; j < N; j++)
                A[k][j] = A[k][j] / A[k][k];
            A[k][k] = 1.0;
            //完成计算后向其他进程发送消息
            for (int j = 0; j < Num_Proc; j++)
                if (j != rank)
                    MPI_Send(&A[k], N, MPI_FLOAT, j, 2, MPI_COMM_WORLD);
        }
        else
        {
            //如果当前行不是自己的任务，接收来自当前行处理进程的消息
            MPI_Recv(&A[k], N, MPI_FLOAT, int((k % seg) / LoopWide), 2,
                MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        for (int i = k + 1; i < N; i++)
        {
            if (int((i % seg) / LoopWide) == rank)
            {
                for (int j = k + 1; j < N; j++)
                    A[i][j] = A[i][j] - A[i][k] * A[k][j];
                A[i][k] = 0.0;
            }
        }
    }
}


void mpi_Cycle()
{
    //需要修改，因为划分方式变了对应的矩阵需求也变了
    //好消息是循环划分不需要再计算remain了，少了两个if-else
    timeval t_start;
    timeval t_end;

    int Num_Proc;
    int rank;

    MPI_Comm_size(MPI_COMM_WORLD, &Num_Proc);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int seg = LoopWide * Num_Proc;
    if (rank == 0)
    {
        reset_A(A, arr);
        gettimeofday(&t_start, NULL);
        //        在0号进程进行任务划分
        for (int i = 0; i < N; i++)
        {
            int flag = (i % seg) / LoopWide;
            if (flag == rank)
                continue;
            else
                MPI_Send(&A[i], N, MPI_FLOAT, flag, 0, MPI_COMM_WORLD);
        }
        LU_cycle(A, rank, Num_Proc);
        //        处理完0号进程自己的任务后需接收其他进程处理之后的结果
        for (int i = 0; i < N; i++)
        {
            int flag = (i % seg) / LoopWide;
            if (flag == rank)
                continue;
            else
                MPI_Recv(&A[i], N, MPI_FLOAT, flag, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        gettimeofday(&t_end, NULL);
        cout << "Recycle MPI LU time cost: "
            << 1000 * (t_end.tv_sec - t_start.tv_sec) +
            0.001 * (t_end.tv_usec - t_start.tv_usec) << "ms" << endl;
        Show(A);
    }
    else
    {
        //        非0号进程先接收任务
        for (int i = LoopWide * rank; i < N; i += seg)
        {
            for (int j = 0; j < LoopWide && i + j < N; j++)
                MPI_Recv(&A[i + j], N, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        LU_cycle(A, rank, Num_Proc);
        //        处理完后向零号进程返回结果
        for (int i = LoopWide * rank; i < N; i += seg)
        {
            for (int j = 0; j < LoopWide && i + j < N; j++)
                MPI_Send(&A[i + j], N, MPI_FLOAT, 0, 1, MPI_COMM_WORLD);
        }
    }
}




void LU_Cycle_Final(float A[][N], int rank, int Num_Proc)
{
    float32x4_t t1, t2, t3;
    int seg = LoopWide * Num_Proc;
#pragma omp parallel num_threads(thread_count),private(t1, t2, t3)
    //实际上private写不写都行
    for (int k = 0; k < N; k++)
    {
        if (int((k % seg) / LoopWide) == rank)
        {
            float temp1[4] = { A[k][k], A[k][k], A[k][k], A[k][k] };
            t1 = vld1q_f32(temp1);
            int j = k + 1;
#pragma omp for schedule(dynamic,20)
            for (j; j < N - 3; j += 4)
            {
                t2 = vld1q_f32(A[k] + j);
                t3 = vdivq_f32(t2, t1);
                vst1q_f32(A[k] + j, t3);
            }
#pragma omp for schedule(dynamic,20) //可并可不并，看心情
            for (j; j < N; j++)
            {
                A[k][j] = A[k][j] / A[k][k];
            }
            A[k][k] = 1.0;

            for (int p = 0; p < Num_Proc; p++)
                if (p != rank)
                    MPI_Send(&A[k], N, MPI_FLOAT, p, 2, MPI_COMM_WORLD);
        }
        else
        {
            MPI_Recv(&A[k], N, MPI_FLOAT, int((k % seg) / LoopWide), 2,
                MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        for (int i = k + 1; i < N; i++)
        {
            if (int((i % seg) / LoopWide) == rank)
            {
                float temp2[4] = { A[i][k], A[i][k], A[i][k], A[i][k] };
                t1 = vld1q_f32(temp2);
                int j = k + 1;
#pragma omp for schedule(dynamic,20)
                for (j; j <= N - 3; j += 4)
                {
                    t2 = vld1q_f32(A[i] + j);
                    t3 = vld1q_f32(A[k] + j);
                    t3 = vmulq_f32(t1, t3);
                    t2 = vsubq_f32(t2, t3);
                    vst1q_f32(A[i] + j, t2);
                }
#pragma omp for schedule(dynamic,20)
                for (j; j < N; j++)
                    A[i][j] = A[i][j] - A[i][k] * A[k][j];
                A[i][k] = 0;
            }
        }
    }
}


void CycleFinal()
{
    //完全和 mpi_Cycle()相同，深化并行修改的是LU函数内部
    //记得把调用LU函数改成LU_Cycle_Final()
}


int main()
{
    init_A(arr);

    Normal_Gauss();

    MPI_Init(NULL, NULL);

    mpi_Cycle();
    CycleFinal();
    MPI_Finalize();

}
