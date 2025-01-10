#include "scheduler.h"
#include "process_generator.h"
#include "memoria.h"
#include "maquina.h"
#include "clock.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

// Variables
int t_max_rr;
int t_scheduler = 2;
int scheduler_visible;
pthread_cond_t scheduler_cond = PTHREAD_COND_INITIALIZER;

// Reducir tiempo de vida de los procesos en ejecucion
void reducir_t_vida_procesos() {
    for (int i = 0; i < maquina.num_cpus; i++) {
        CPU cpu = maquina.cpus[i];
        for (int j = 0; j < cpu.num_cores; j++) {
            Core core = cpu.cores[j];
            for (int k = 0; k < core.num_hilos; k++) {
                Hilo* hilo = &core.hilos[k];
                if (hilo->pcb != NULL) {
                    if (politica == RR && hilo->pcb->t_vida > t_max_rr) {
                        hilo->pcb->estado = 0;
                        hilo->pcb->t_vida -= t_max_rr;
                        hilo->pcb = NULL;
                    } else {
                        if(hilo->pcb->t_vida > t_scheduler) {
                            hilo->pcb->t_vida -= t_scheduler;
                        } else {
                            hilo->pcb->estado = 2;
                            hilo->pcb->t_vida = 0;
                            hilo->pcb = NULL;
                        }
                    }
                    if (scheduler_visible) {
                        printf("Hilo %d que ejecuta proceso pid = %d, nuevo t_vida: %d\n", hilo->id_hilo, hilo->pcb->pid, hilo->pcb->t_vida);
                    }
                }
            }
        }
    }
}

// Asigna los procesos READY a hilos
void asignar_procesos() {
    if (scheduler_visible) {
        printf("\n");
        printf("SCHEDULER ACTIVADO\n");
    }
    for (int i = 0; i < pQ.num_procesos; i++) {
        PCB *proceso = siguiente_proceso();
        if (proceso == NULL) {
            printf("\n/!\\ NO HAY MAS PROCESOS READY\n\n");
            return;
        }
        Hilo *hilo_libre = siguiente_hilo();
        if (hilo_libre == NULL) {
            printf("\n/!\\ NO HAY MAS HILOS LIBRES\n\n");
            return;
        }
        hilo_libre->pcb = proceso;
        hilo_libre->PTBR = proceso->mm.pgb;
        if (scheduler_visible) {
            printf("Hilo %d ejecutando proceso pid = %d\n", hilo_libre->id_hilo, hilo_libre->pcb->pid);
        }
    }
    if (scheduler_visible) {
        printf("\n");
    }
}

//---------------------------------//
// Función de Scheduler/Dispatcher //
//---------------------------------//
void *scheduler_thread(void *arg) {
    while (clock_activo) {
        pthread_mutex_lock(&clock_mutex);
        pthread_cond_wait(&scheduler_cond, &clock_mutex);
        pthread_mutex_unlock(&clock_mutex);
        reducir_t_vida_procesos();
        asignar_procesos();
    }
    return 0;
}
