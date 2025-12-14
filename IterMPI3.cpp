#include <mpi.h>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <cstdio>

using namespace std;

int main(int argc, char** argv)
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const int N = 50000;
    const double eps = 1e-6;
    const int max_iter = 10000;

    // Распределение строк по процессам
    int base = N / size;
    int rem  = N % size;

    int* counts = new int[size];
    int* displs = new int[size];

    for (int p = 0; p < size; ++p) {
        counts[p] = base + (p < rem ? 1 : 0);
        displs[p] = (p == 0 ? 0 : displs[p-1] + counts[p-1]);
    }

    int local_n = counts[rank];
    int local_start = displs[rank];

    // Блок матрицы и вектора b
    double** C_local = new double*[local_n];
    for (int i = 0; i < local_n; ++i)
        C_local[i] = new double[N];

    double* f_local = new double[local_n];

    // Загрузка данных
    FILE* fm = fopen("matrix.bin", "rb");
    FILE* ff = fopen("vector.bin", "rb");

    if (!fm || !ff) {
        if (rank == 0) cerr << "Ошибка открытия matrix.bin/vector.bin\n";
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    long long row_size_bytes = sizeof(double) * N;
    fseek(fm, (long long)local_start * row_size_bytes, SEEK_SET);
    fseek(ff, (long long)local_start * sizeof(double), SEEK_SET);

    for (int i = 0; i < local_n; i++)
        fread(C_local[i], sizeof(double), N, fm);

    fread(f_local, sizeof(double), local_n, ff);

    fclose(fm);
    fclose(ff);

    MPI_Barrier(MPI_COMM_WORLD);

    // Вектор решения и локальный буфер
    double* x_old   = new double[N];
    double* x_new   = new double[N];
    double* x_local = new double[local_n];

    for (int i = 0; i < N; i++) x_old[i] = 0.0;

    double t_start = MPI_Wtime();
    double diff = 1.0;
    int iter = 0;

    while (diff > eps && iter < max_iter)
    {
        iter++;

        // Вычисляем локальные строки
        for (int ii = 0; ii < local_n; ii++) {
            int gi = local_start + ii;
            double diag = C_local[ii][gi];
            double sum = 0.0;

            for (int j = 0; j < N; j++) {
                if (j != gi)
                    sum += C_local[ii][j] * x_old[j];
            }

            x_local[ii] = (f_local[ii] - sum) / diag;
        }

        // Сбор нового вектора x со всех процессов
        MPI_Allgatherv(x_local, local_n, MPI_DOUBLE,
                       x_new, counts, displs, MPI_DOUBLE,
                       MPI_COMM_WORLD);

        // Локальная проверка сходимости
        double local_diff = 0.0;
        for (int i = 0; i < local_n; i++) {
            int gi = local_start + i;
            double d = fabs(x_new[gi] - x_old[gi]);
            if (d > local_diff) local_diff = d;
        }

        MPI_Allreduce(&local_diff, &diff, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);

        // Обновление x_old
        for (int i = 0; i < N; i++)
            x_old[i] = x_new[i];
    }

    double t_end = MPI_Wtime();

    // Печать результатов только на корневом процессе
    if (rank == 0) {
        cout << "Сошлось за " << iter << " итераций, diff = " << diff << endl;
        cout << "Время работы: " << t_end - t_start << " секунд" << endl;
      /*  cout << "\nРешение x:" << endl;
    for (int i = 0; i < N; i++)
        cout << x_old[i] << endl;*/
        
    }

    // Очистка
    for (int i = 0; i < local_n; i++) delete[] C_local[i];
    delete[] C_local;
    delete[] f_local;
    delete[] x_local;
    delete[] x_old;
    delete[] x_new;
    delete[] counts;
    delete[] displs;

    MPI_Finalize();
    return 0;
}
