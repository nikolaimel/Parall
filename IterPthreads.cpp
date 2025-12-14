#include <iostream>
#include <cmath>
#include <cstdlib>
#include <pthread.h>
#include <cstdio>
#include <time.h>

using namespace std;

const int N = 50000;
const double eps = 1e-6;
const int max_iter = 10000;

double** A;
double* b;
double* x_old;
double* x_new;

int NUM_THREADS = 1;
pthread_barrier_t barrier;

double diff_global = 1.0;
int iter_global = 0;
int stop_flag = 0;

double* local_diff;

struct ThreadData {
    int tid;
    int start;
    int end;
};

void* thread_func(void* arg)
{
    ThreadData* d = (ThreadData*)arg;
    int tid = d->tid;
    int start = d->start;
    int end   = d->end;

    while (true)
    {
        // Вычисление новых x
        for (int i = start; i < end; i++) {
            double diag = A[i][i];
            double sum = 0.0;
            for (int j = 0; j < N; j++)
                if (j != i)
                    sum += A[i][j] * x_old[j];
            x_new[i] = (b[i] - sum) / diag;
        }

        pthread_barrier_wait(&barrier);

        // Локальная норма
        double loc = 0.0;
        for (int i = start; i < end; i++) {
            double dval = fabs(x_new[i] - x_old[i]);
            if (dval > loc) loc = dval;
        }
        local_diff[tid] = loc;

        pthread_barrier_wait(&barrier);

        if (tid == 0) {
            diff_global = 0.0;
            for (int k = 0; k < NUM_THREADS; k++)
                if (local_diff[k] > diff_global)
                    diff_global = local_diff[k];

            iter_global++;

            stop_flag = (diff_global <= eps || iter_global >= max_iter);
        }

        pthread_barrier_wait(&barrier);

        if (stop_flag)
            break;

        for (int i = start; i < end; i++)
            x_old[i] = x_new[i];

        pthread_barrier_wait(&barrier);
    }

    return NULL;
}

int main(int argc, char** argv)
{
    if (argc > 1)
        NUM_THREADS = atoi(argv[1]);

    A = new double*[N];
    for (int i = 0; i < N; i++) A[i] = new double[N];

    b = new double[N];
    x_old = new double[N];
    x_new = new double[N];

    for (int i = 0; i < N; i++) x_old[i] = 0.0;

    local_diff = new double[NUM_THREADS];

    FILE* fm = fopen("matrix.bin", "rb");
    FILE* ff = fopen("vector.bin", "rb");

    for (int i = 0; i < N; i++) {
        fread(A[i], sizeof(double), N, fm);
        fread(&b[i], sizeof(double), 1, ff);
    }

    fclose(fm);
    fclose(ff);

    pthread_barrier_init(&barrier, NULL, NUM_THREADS);

    pthread_t* threads = new pthread_t[NUM_THREADS];
    ThreadData* tdata  = new ThreadData[NUM_THREADS];

    int chunk = N / NUM_THREADS;
    for (int t = 0; t < NUM_THREADS; t++) {
        tdata[t].tid   = t;
        tdata[t].start = t * chunk;
        tdata[t].end   = (t == NUM_THREADS - 1 ? N : (t + 1) * chunk);
    }

    timespec ts1, ts2;
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    for (int t = 0; t < NUM_THREADS; t++)
        pthread_create(&threads[t], NULL, thread_func, &tdata[t]);

    for (int t = 0; t < NUM_THREADS; t++)
        pthread_join(threads[t], NULL);

    clock_gettime(CLOCK_MONOTONIC, &ts2);

    double elapsed =
        (ts2.tv_sec - ts1.tv_sec) +
        (ts2.tv_nsec - ts1.tv_nsec) * 1e-9;

    cout << "Threads: " << NUM_THREADS
         << "  Iter: " << iter_global
         << "  diff = " << diff_global
         << "  time = " << elapsed << endl;

    /*cout << "\nРешение x:" << endl;
    for (int i = 0; i < N; i++)
        cout << x_old[i] << endl;*/

    return 0;
}
