#include <iostream>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <omp.h>

using namespace std;

int main(int argc, char** argv)
{
    const int N = 50000;  
    const double eps = 1e-6;
    const int max_iter = 10000;

    double** A = new double*[N];
    for (int i = 0; i < N; i++)
        A[i] = new double[N];

    double* b = new double[N];
    double* x_old = new double[N];
    double* x_new = new double[N];

    for (int i = 0; i < N; i++)
        x_old[i] = 0.0;

    

    FILE* fm = fopen("matrix.bin", "rb");
    FILE* ff = fopen("vector.bin", "rb");

    if (!fm || !ff) {
        cerr << "Ошибка: не удалось открыть файлы" << endl;
        return 1;
    }

    for (int i = 0; i < N; i++)
        fread(A[i], sizeof(double), N, fm);
    fread(b, sizeof(double), N, ff);

    fclose(fm);
    fclose(ff);


    double t_start = omp_get_wtime();
    double diff = 1.0;
    int iter = 0;

    while (diff > eps && iter < max_iter)
    {
        iter++;

        // Итерации
        #pragma omp parallel for
        for (int i = 0; i < N; i++)
        {
            double diag = A[i][i];
            double sum = 0.0;

            for (int j = 0; j < N; j++)
                if (j != i)
                    sum += A[i][j] * x_old[j];

            x_new[i] = (b[i] - sum) / diag;
        }

        // Проверка сходимости
        diff = 0.0;
        #pragma omp parallel for reduction(max:diff)
        for (int i = 0; i < N; i++)
            diff = max(diff, fabs(x_new[i] - x_old[i]));

        // Обновление
        #pragma omp parallel for
        for (int i = 0; i < N; i++)
            x_old[i] = x_new[i];
    }

    double t_end = omp_get_wtime();

    cout << "Сошлось за " << iter << " итераций, diff = " << diff << endl;
    cout << "Время работы: " << t_end - t_start << " секунд" << endl;
    
    
        /*cout << "\nРешение x:\n";
        for (int i = 0; i < N; i++)
            cout << x_old[i] << endl;*/
   

    // Очистка
    for (int i = 0; i < N; i++) delete[] A[i];
    delete[] A;
    delete[] b;
    delete[] x_old;
    delete[] x_new;

    return 0;
}
