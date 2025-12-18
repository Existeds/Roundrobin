#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

// --- FUNGSI DOWORK (Anak) ---
void doWork(int id, int burst_time) {
    int sisa = burst_time;
    while (sisa > 0) {
        // Menunggu sinyal untuk lanjut
        printf("  [P%d] Bekerja... (Sisa: %ds)\n", id, sisa);
        fflush(stdout); // Pastikan teks langsung muncul
        sleep(1); 
        sisa--;
    }
    printf("  [P%d] SELESAI.\n", id);
    exit(0);
}

// --- FUNGSI SCHEDULER_HANDLER (Induk) ---
void scheduler_handler(int n, pid_t pids[], int burst_times[], int quantum) {
    int sisa_waktu[10];
    int selesai = 0;
    for (int i = 0; i < n; i++) sisa_waktu[i] = burst_times[i];

    printf("\n--- Start Round Robin (Quantum: %ds) ---\n", quantum);

    while (selesai < n) {
        for (int i = 0; i < n; i++) {
            if (sisa_waktu[i] > 0) {
                printf("\n>>> Giliran P%d (Sisa: %ds)\n", i + 1, sisa_waktu[i]);
                
                // Bangunkan proses anak
                kill(pids[i], SIGCONT);

                int durasi = (sisa_waktu[i] > quantum) ? quantum : sisa_waktu[i];
                
                // Menunggu anak bekerja selama durasi quantum
                sleep(durasi); 
                
                sisa_waktu[i] -= durasi;

                if (sisa_waktu[i] > 0) {
                    // Berhentikan (pause) jika waktu habis
                    kill(pids[i], SIGSTOP);
                    printf(">>> P%d di-jeda.\n", i + 1);
                } else {
                    // Pastikan proses benar-benar selesai
                    waitpid(pids[i], NULL, 0);
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
    if (scanf("%d", &n) != 1) return 1;

    int burst_times[10];
    pid_t pids[10];

    for (int i = 0; i < n; i++) {
        printf("Burst time P%d: ", i + 1);
        scanf("%d", &burst_times[i]);
    }

    for (int i = 0; i < n; i++) {
        pids[i] = fork();
        if (pids[i] == 0) {
            // Anak langsung berhenti begitu lahir
            raise(SIGSTOP); 
            doWork(i + 1, burst_times[i]);
        }
    }

    // Beri jeda kecil agar semua fork selesai dengan sempurna sebelum diatur scheduler
    usleep(100000); 

    scheduler_handler(n, pids, burst_times, quantum);

    return 0;
}
