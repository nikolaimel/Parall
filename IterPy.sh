#!/bin/bash
#SBATCH --job-name=iter_mpi_tests_python
#SBATCH --nodes=4
#SBATCH --ntasks-per-node=28
#SBATCH --cpus-per-task=1
#SBATCH --time=01:00:00

module load mpi/openmpi
module load python/3.9

OUTFILE="iter_python_results_$(date +%Y%m%d_%H%M%S).log"

echo "===== Запуск тестов IterMPI Python ====="      | tee -a "$OUTFILE"
echo "===== Старт: $(date) ====="                    | tee -a "$OUTFILE"
echo ""                                              | tee -a "$OUTFILE"

# Тестирование разных чисел процессов
for np in 1 2 4 8 16 32 64 112; do

    echo "===== ТЕСТ: np = $np ====="                 | tee -a "$OUTFILE"

    /usr/bin/time -f "real %e\nuser %U\nsys %S" \
        mpirun --mca btl ^openib -np $np python3.9 IterPy.py \
        >> "$OUTFILE" 2>&1

    echo "===== ЗАВЕРШЕНО: np = $np ====="            | tee -a "$OUTFILE"
    echo ""                                          | tee -a "$OUTFILE"
done

echo "===== Все тесты завершены: $(date) ====="      | tee -a "$OUTFILE"
