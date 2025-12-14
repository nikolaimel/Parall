#!/bin/bash
#SBATCH --job-name=iter_omp_tests
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=48
#SBATCH --time=01:00:00

cd $SLURM_SUBMIT_DIR

module load gcc    

OUTFILE="omp_results_$(date +%Y%m%d_%H%M%S).log"

echo "===== Запуск OpenMP тестов ====="          | tee -a "$OUTFILE"
echo "===== Старт: $(date) ====="               | tee -a "$OUTFILE"


# набор потоков для тестов
for threads in 1 2 4 8 16 32 48; do

    echo "===== TEST OMP_NUM_THREADS=$threads =====" | tee -a "$OUTFILE"
    export OMP_NUM_THREADS=$threads

    /usr/bin/time -f "real %e\nuser %U\nsys %S\n" ./IterOpenMP >> "$OUTFILE" 2>&1

    echo "===== END TEST $threads =====" | tee -a "$OUTFILE"
    echo ""                               | tee -a "$OUTFILE"

done

echo "===== Все OpenMP тесты завершены: $(date) =====" | tee -a "$OUTFILE"