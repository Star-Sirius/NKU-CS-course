#include <iostream>
#include <mpi.h>
#include <sys/time.h>
#include <arm_neon.h>
#include <omp.h>
//带Final版本的是深化并行的，LU_pipe是使用流水线通信算法的
using namespace std;

static const int N = 1000;
static const int thread_count = 4;

float arr[N][N];
float A[N][N];

void init_A(float arr[][N])
{
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            arr[i][j] = 0;
        }
        arr[i][i] = 1.0;
        for (int j = i + 1; j < N; j++)
            arr[i][j] = rand() % 100;
    }

    for (int i = 0; i < N; i++)
    {
        int k1 = rand() % N;
        int k2 = rand() % N;
        for (int j = 0; j < N; j++)
        {
            arr[i][j] += arr[0][j];
            arr[k1][j] += arr[k2][j];
        }
    }
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



void LU_pipe_Final(float A[][N], int rank, int num_proc)
{
    //    计算当前进程的前一进程及下一进程
    int pre_proc = (rank + (num_proc - 1)) % num_proc;
    int next_proc = (rank + 1) % num_proc;
    for (int k = 0; k < N; k++)
    {
        //简化，默认行循环划分为1，或者不简化参见mpi-cycle
        if (int(k % num_proc) == rank)
        {
            for (int j = k + 1; j < N; j++)
                A[k][j] = A[k][j] / A[k][k];
            A[k][k] = 1.0;
            //处理完自己的任务后向下一进程发送消息
            MPI_Send(&A[k], N, MPI_FLOAT, next_proc, 2, MPI_COMM_WORLD);
        }
        else
        {
            //如果当前行不是当前进程的任务，则接收前一进程的消息
            MPI_Recv(&A[k], N, MPI_FLOAT, pre_proc, 2,
                MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            //如果当前行不是下一进程的任务，需将消息进行传递
            if (int(k % num_proc) != next_proc)
                MPI_Send(&A[k], N, MPI_FLOAT, next_proc, 2, MPI_COMM_WORLD);
        }
        for (int i = k + 1; i < N; i++)
        {
            if (int(i % num_proc) == rank)
            {
                for (int j = k + 1; j < N; j++)
                    A[i][j] = A[i][j] - A[i][k] * A[k][j];
                A[i][k] = 0.0;
            }
        }
    }
}


void mpi_pipe()
{
    timeval t_start;
    timeval t_end;

    int num_proc;
    int rank;

    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0)
    {
        reset_A(A, arr);
        gettimeofday(&t_start, NULL);
        //        在0号进程进行任务划分
        for (int i = 0; i < N; i++)
        {
            int flag = i % num_proc;
            if (flag == rank)
                continue;
            else
                MPI_Send(&A[i], N, MPI_FLOAT, flag, 0, MPI_COMM_WORLD);
        }
        LU_pipe_Final(A, rank, num_proc);
        //处理完0号进程自己的任务后需接收其他进程处理之后的结果
        for (int i = 0; i < N; i++)
        {
            int flag = i % num_proc;
            if (flag == rank)
                continue;
            else
                MPI_Recv(&A[i], N, MPI_FLOAT, flag, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        gettimeofday(&t_end, NULL);
        cout << "Pipeline(& single line cycle) MPI LU time cost: "
            << 1000 * (t_end.tv_sec - t_start.tv_sec) +
            0.001 * (t_end.tv_usec - t_start.tv_usec) << "ms" << endl;
    }
    else
    {
        //非0号进程先接收任务
        for (int i = rank; i < N; i += num_proc)
        {
            MPI_Recv(&A[i], N, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        LU_pipe_Final(A, rank, num_proc);
        //非0号进程完成任务之后，将结果传回到0号进程
        for (int i = rank; i < N; i += num_proc)
        {
            MPI_Send(&A[i], N, MPI_FLOAT, 0, 1, MPI_COMM_WORLD);
        }
    }
}



void LU_pipe_Final(float A[][N], int rank, int num_proc)
{
    float32x4_t t1, t2, t3;
    int pre_proc = (rank + (num_proc - 1)) % num_proc;
    int next_proc = (rank + 1) % num_proc;
#pragma omp parallel num_threads(thread_count)
    for (int k = 0; k < N; k++)
    {
        if (int(k % num_proc) == rank)
        {
            float temp1[4] = { A[k][k], A[k][k], A[k][k], A[k][k] };
            t1 = vld1q_f32(temp1);

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
            MPI_Send(&A[k], N, MPI_FLOAT, next_proc, 2, MPI_COMM_WORLD);
            //注意这里只向下一进程发送消息
        }
        else
        {
            //要么接收上一进程的消息，要么把消息传给下一线程
            MPI_Recv(&A[k], N, MPI_FLOAT, pre_proc, 2,
                MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (int(k % num_proc) != next_proc)
                MPI_Send(&A[k], N, MPI_FLOAT, next_proc, 2, MPI_COMM_WORLD);
        }
        for (int i = k + 1; i < N; i++)
        {
            if (int(i % num_proc) == rank)
            {
                float temp2[4] = { A[i][k], A[i][k], A[i][k], A[i][k] };
                t1 = vld1q_f32(temp2);
#pragma omp for schedule(dynamic, 20)
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

void MPI_pipe_Final()
{
    timeval t_start;
    timeval t_end;

    int num_proc;
    int rank;

    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0)
    {
        reset_A(A, arr);
        gettimeofday(&t_start, NULL);
        for (int i = 0; i < N; i++)
        {
            int flag = i % num_proc;
            if (flag == rank)
                continue;
            else
                MPI_Send(&A[i], N, MPI_FLOAT, flag, 0, MPI_COMM_WORLD);
        }
        LU_pipe_Final(A, rank, num_proc);
        for (int i = 0; i < N; i++)
        {
            int flag = i % num_proc;
            if (flag == rank)
                continue;
            else
                MPI_Recv(&A[i], N, MPI_FLOAT, flag, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        gettimeofday(&t_end, NULL);
        cout << "Pipeline MPI LU with Neon and OpenMP(dynamic) time cost: "
            << 1000 * (t_end.tv_sec - t_start.tv_sec) +
            0.001 * (t_end.tv_usec - t_start.tv_usec) << "ms" << endl;
        //Show(A);
    }
    else
    {
        for (int i = rank; i < N; i += num_proc)
        {
            MPI_Recv(&A[i], N, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        LU_pipe_Final(A, rank, num_proc);
        for (int i = rank; i < N; i += num_proc)
        {
            MPI_Send(&A[i], N, MPI_FLOAT, 0, 1, MPI_COMM_WORLD);
        }
    }
}


int main()
{
    init_A(arr);

    MPI_Init(NULL, NULL);

    mpi_pipe();
    MPI_pipe_Final();
    MPI_Finalize();


}
