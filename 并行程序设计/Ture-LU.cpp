#include <iostream>
#include <vector>
#include <random>

using namespace std;

// LU分解函数
void LU_Decomposition(vector<vector<double>>& A, vector<vector<double>>& L, vector<vector<double>>& U) {
    int n = A.size();

    // 初始化L和U矩阵
    L.resize(n, vector<double>(n, 0.0));
    U.resize(n, vector<double>(n, 0.0));

    for (int i = 0; i < n; i++) {
        // 计算U的第一行元素
        U[0][i] = A[0][i];

        // 计算L的第一列元素
        L[i][0] = A[i][0] / U[0][0];
    }

    for (int r = 1; r < n; r++) {
        for (int i = r; i < n; i++) {
            // 计算U的第r行元素
            U[r][i] = A[r][i];
            for (int k = 0; k < r; k++) {
                U[r][i] -= L[r][k] * U[k][i];
            }

            // 计算L的第r列元素
            L[i][r] = A[i][r];
            for (int k = 0; k < r; k++) {
                L[i][r] -= L[i][k] * U[k][r];
            }
            L[i][r] /= U[r][r];
        }
    }
}

// 打印矩阵
void printMatrix(const vector<vector<double>>& matrix) {
    int n = matrix.size();
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            cout << matrix[i][j] << " ";
        }
        cout << endl;
    }
}

int main() {
    int N = 5; // 矩阵的大小
    vector<vector<double>> A(N, vector<double>(N, 0.0)); // 输入矩阵
    vector<vector<double>> L; // L矩阵
    vector<vector<double>> U; // U矩阵

    // 随机生成矩阵A
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<double> dis(1.0, 10.0);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i][j] = dis(gen);
        }
    }

    cout << "输入矩阵A：" << endl;
    printMatrix(A);

    // 进行LU分解
    LU_Decomposition(A, L, U);

    cout << "L矩阵：" << endl;
    printMatrix(L);

    cout << "U矩阵：" << endl;
    printMatrix(U);

    return 0;
}
