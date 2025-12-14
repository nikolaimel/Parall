#!/bin/bash
#SBATCH --job-name=iter_mpi_tests
#SBATCH --nodes=4
#SBATCH --ntasks-per-node=28
#SBATCH --cpus-per-task=1
#SBATCH --time=01:00:00

module load mpi/openmpi

OUTFILE="iter_results_$(date +%Y%m%d_%H%M%S).log"

echo "===== Запуск тестов IterMPI3 ====="          | tee -a "$OUTFILE"
echo "===== Старт: $(date) ====="                 | tee -a "$OUTFILE"

for np in 1 2 4 8 16 32 64 112; do
    echo ""                                        | tee -a "$OUTFILE"
    echo "===== ТЕСТ: np = $np ====="              | tee -a "$OUTFILE"
    echo ""                                        | tee -a "$OUTFILE"

    # ВАЖНО: time должен быть перед mpirun!
    /usr/bin/time -f "real %e\nuser %U\nsys %S" \
        mpirun -np $np ./IterMPI3  >> "$OUTFILE" 2>&1

    echo "===== ЗАВЕРШЕНО: np = $np ====="         | tee -a "$OUTFILE"
done

echo ""                                            | tee -a "$OUTFILE"
echo "===== Все тесты завершены: $(date) ====="    | tee -a "$OUTFILE"
