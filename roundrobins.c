#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

// --- FUNGSI DOWORK ---
void doWork(int id, int burst_time) {
    int sisa = burst_time;
    while (sisa > 0) {
        // Child hanya akan berjalan jika menerima SIGCONT
        printf("  [P%d] Sedang bekerja... (Sisa: %ds)\n", id, sisa);
        sleep(1); 
        sisa--;
    }
    printf("  [P%d] Selesai mengerjakan semua tugas.\n", id);
    exit(0);
}

// --- FUNGSI SCHEDULER_HANDLER ---
void scheduler_handler(int n, pid_t pids[], int burst_times[], int quantum) {
    int sisa_waktu[10];
    int selesai = 0;
    for (int i = 0; i < n; i++) sisa_waktu[i] = burst_times[i];

    printf("\n--- Start Round Robin (Quantum: %ds) ---\n", quantum);

    while (selesai < n) {
        for (int i = 0; i < n; i++) {
            if (sisa_waktu[i] > 0) {
                printf("\n>>> Giliran P%d (Sisa Burst: %ds)\n", i + 1, sisa_waktu[i]);
                
                kill(pids[i], SIGCONT); // Lanjutkan Child

                int durasi_detik = (sisa_waktu[i] > quantum) ? quantum : sisa_waktu[i];
                
                // Beri waktu Child bekerja tepat sesuai durasi_detik
                sleep(durasi_detik); 
                
                sisa_waktu[i] -= durasi_detik;

                if (sisa_waktu[i] > 0) {
                    kill(pids[i], SIGSTOP); // Hentikan tepat setelah durasi habis
                    printf(">>> P%d di-jeda (Waktu habis).\n", i + 1);
                } else {
                    int status;
                    waitpid(pids[i], &status, 0); // Tunggu Child exit bersih
                    selesai++;
                }
            }
        }
    }
    printf("\n[Selesai] Semua proses telah diproses.\n");
}

// --- FUNGSI MAIN ---
int main() {
    int n, quantum = 5;
    printf("Masukkan jumlah proses: ");
    scanf("%d", &n);

    int burst_times[10];
    pid_t pids[10];

    for (int i = 0; i < n; i++) {
        printf("Burst time P%d: ", i + 1);
        scanf("%d", &burst_times[i]);
    }

    for (int i = 0; i < n; i++) {
        pids[i] = fork();
        if (pids[i] == 0) {
            raise(SIGSTOP); // Child langsung pause setelah lahir
            doWork(i + 1, burst_times[i]);
        }
    }

    scheduler_handler(n, pids, burst_times, quantum);
    return 0;
}