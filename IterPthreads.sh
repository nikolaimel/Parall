#!/bin/bash
#SBATCH --job-name=iter_pthreads_tests
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=48
#SBATCH --time=01:00:00

cd $SLURM_SUBMIT_DIR

module load compiler/gcc/4.9.4
# pthreads встроены — модуль нужен только для компиляции

OUTFILE="pthreads_results_$(date +%Y%m%d_%H%M%S).log"

echo "===== Запуск pthreads тестов ====="           | tee -a "$OUTFILE"
echo "===== Старт: $(date) ====="                  | tee -a "$OUTFILE"
echo ""                                            | tee -a "$OUTFILE"

# Тестируем разные количества потоков
for threads in 1 2 4 8 16 32 48; do

    echo "===== TEST pthreads threads = $threads =====" | tee -a "$OUTFILE"

    /usr/bin/time -f "real %e\nuser %U\nsys %S" \
        ./IterPthreads $threads >> "$OUTFILE" 2>&1

    echo "===== END TEST $threads ====="            | tee -a "$OUTFILE"
    echo ""                                         | tee -a "$OUTFILE"

done

echo "===== Все pthreads тесты завершены: $(date) =====" | tee -a "$OUTFILE"
