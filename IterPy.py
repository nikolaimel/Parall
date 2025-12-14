# -*- coding: utf-8 -*-

from mpi4py import MPI
import numpy as np

comm = MPI.COMM_WORLD
rank = comm.Get_rank()
size = comm.Get_size()

N = 50000
eps = 1e-6
max_iter = 10000

# Распределение строк 
base = N // size
rem = N % size

counts = np.array([base + (1 if p < rem else 0) for p in range(size)], dtype=int)
displs = np.array([sum(counts[:p]) for p in range(size)], dtype=int)

local_n = counts[rank]
local_start = displs[rank]

# Чтение A и b из файлов
A_local = np.zeros((local_n, N), dtype=np.float64)
b_local = np.zeros(local_n, dtype=np.float64)

try:
    fm = open("matrix.bin", "rb")
    ff = open("vector.bin", "rb")
except OSError:
    if rank == 0:
        print("Ошибка: не удалось открыть matrixtest.bin или vectortest.bin")
    MPI.Abort(comm, 1)

# Переход к своим строкам
row_size_bytes = N * 8
fm.seek(local_start * row_size_bytes)
ff.seek(local_start * 8)

for i in range(local_n):
    A_local[i, :] = np.fromfile(fm, dtype=np.float64, count=N)
    b_local[i] = np.fromfile(ff, dtype=np.float64, count=1)[0]

fm.close()
ff.close()

# Векторы решения 
x_old = np.zeros(N, dtype=np.float64)
x_new = np.zeros(N, dtype=np.float64)
x_local = np.zeros(local_n, dtype=np.float64)

comm.Barrier()
t_start = MPI.Wtime()

diff = 1.0
iter_count = 0

#  Итерации 
while diff > eps and iter_count < max_iter:
    iter_count += 1

    # локальный расчёт новых компонент x
    for i in range(local_n):
        gi = local_start + i
        row = A_local[i]
        diag = row[gi]

        # сумма по j != i
        sum_off = np.dot(row, x_old) - diag * x_old[gi]

        x_local[i] = (b_local[i] - sum_off) / diag

    # сбор полного x_new
    comm.Allgatherv(
        x_local,
        [x_new, counts, displs, MPI.DOUBLE]
    )

    # локальная норма
    local_diff = np.max(
        np.abs(
            x_new[local_start:local_start + local_n] -
            x_old[local_start:local_start + local_n]
        )
    )
    # глобальная норма
    diff = comm.allreduce(local_diff, op=MPI.MAX)

    # обновляем x_old
    x_old[:] = x_new[:]

t_end = MPI.Wtime()

if rank == 0:
    print(f"Сошлось за {iter_count} итераций, diff = {diff}")
    print(f"Время работы: {t_end - t_start} секунд")
    #print("x =", x_old)

    
