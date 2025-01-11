#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "process_generator.h"
#include "loader.h"
#include "scheduler.h"
#include "clock.h"
#include "timer.h"
#include "maquina.h"
#include "memoria.h"

int main(int argc, char *argv[]) {
    int capacidad;

    if (argc != 1) {
    fprintf(stderr, "Uso: %s\n", argv[0]);
        return -1;
    }

    printf("Indica el número de CPUs: ");
    scanf("%d", &maquina.num_cpus);

    maquina.cpus = (CPU *)malloc(maquina.num_cpus * sizeof(CPU));

    for (int i = 0; i < maquina.num_cpus; i++) {
        printf(" Indica el número de cores para la CPU %d: ", i + 1);
        scanf("%d", &maquina.cpus[i].num_cores);
        
        // Asignar memoria para los cores de la CPU
        maquina.cpus[i].cores = (Core *)malloc(maquina.cpus[i].num_cores * sizeof(Core));

        // Pedir el número de hilos por core
        for (int j = 0; j < maquina.cpus[i].num_cores; j++) {
            printf("  Indica el número de hilos para el core %d: ", j + 1);
            scanf("%d", &maquina.cpus[i].cores[j].num_hilos);

            // Asignar memoria para los hilos del core
            maquina.cpus[i].cores[j].hilos = (Hilo *)malloc(maquina.cpus[i].cores[j].num_hilos * sizeof(Hilo));

            // Inicializar los hilos
            for (int k = 0; k < maquina.cpus[i].cores[j].num_hilos; k++) {
                maquina.cpus[i].cores[j].hilos[k].id_hilo = k;     // Asignar un ID único para cada hilo
                maquina.cpus[i].cores[j].hilos[k].PC = 0;          // Inicializar el contador de programa
                maquina.cpus[i].cores[j].hilos[k].RI = 0;          // Inicializar el registro de instrucción
                maquina.cpus[i].cores[j].hilos[k].PTBR = -1;       // Inicializar el puntero de la tabla de páginas
            }
        }
    }

    printf("Indica cada cuantos ciclos debe interrumpir el timer: ");
    scanf("%d", &t_periodo);

    printf("Indica la política de planificación (FCFS=0, RR=1, SJF=2, PRIO=3): ");
    scanf("%d", &politica);

    // Validar la política seleccionada
    if (politica < 0 || politica > 3) {
        fprintf(stderr, "Política desconocida. Usando FCFS por defecto.\n");
        politica = FCFS;
    }

    if (politica == RR) {
        printf("Indica el tiempo máximo de ejecución por tarea para Round Robin (segundos): ");
        scanf("%d", &t_max_rr);
    }

    printf("Indica la capacidad inicial de la cola de procesos: ");
    scanf("%d", &capacidad);

    printf("¿Cola dinamica? 0(No)/1(Sí): ");
    scanf("%d", &cola_dinamica);

    printf("¿Enseñar ciclos del reloj? 0(No)/1(Sí): ");
    scanf("%d", &clock_visible);

    printf("¿Enseñar interrupciones del timer? 0(No)/1(Sí): ");
    scanf("%d", &timer_visible);

    printf("¿Enseñar el manejo de la memoria? 0(No)/1(Sí): ");
    scanf("%d", &memoria_visible);

    printf("¿Enseñar el manejo del scheduler? 0(No)/1(Sí): ");
    scanf("%d", &scheduler_visible);

    strcpy(carpeta_procesos, "procesos"); // Asignar directamente el nombre de la carpeta fija

    // Confirmación de los parámetros ingresados
    printf("\n----------CONFIGURACION----------\n");
    imprimir_info_maquina();

    switch (politica) {
        case FCFS:
            printf("Política de planificación: FCFS\n");
            break;
        case RR:
            printf("Política de planificación: Round Robin\n");
            printf("Tiempo máximo RR: %d segundos\n", t_max_rr);
            break;
        case SJF:
            printf("Política de planificación: SJF\n");
            break;
        case PRIO:
            printf("Política de planificación: PRIO\n");
            break;
    }

    printf("Capacidad inicial de la cola de procesos: %d\n", capacidad);
    printf("Cola dinamica: %s\n", cola_dinamica ? "Sí" : "No");
    printf("Mostrar ciclos del reloj: %s\n", clock_visible ? "Sí" : "No");
    printf("Ciclos por interrupcion del timer: %d\n", t_periodo);
    printf("Mostrar interrupciones del timer: %s\n", timer_visible ? "Sí" : "No");
    printf("Mostrar el manejo de memoria: %s\n", memoria_visible ? "Sí" : "No");
    printf("Mostrar el manejo del scheduler: %s\n", scheduler_visible ? "Sí" : "No");
    printf("Carpeta de procesos: %s\n", carpeta_procesos); 

    printf("\n-----------EJECUCION-----------\n");

    srand(time(NULL)); // Inicializa la semilla aleatoria
    clock_activo = 1;

    // Inicialización de la memoria física
    inicializar_memoria_fisica();

    // Inicializa la cola de procesos
    iniciar_process_queue(capacidad + 1); // Se usa un hueco de la cola para gestion

    // Crea hilos
    pthread_t clock_tid, timer_tid, process_generator_tid, scheduler_tid;
    pthread_create(&clock_tid, NULL, clock_thread, NULL);
    pthread_create(&timer_tid, NULL, timer_thread, NULL);
    pthread_create(&process_generator_tid, NULL, generator_thread, NULL);
    pthread_create(&scheduler_tid, NULL, scheduler_thread, NULL);

    // Espera que termine la simulación
    sleep(60); // Segundos de ejecucion antes de parar
    clock_activo = 0;

    // Enviar señal a todos los hilos que estan esperando
    pthread_cond_broadcast(&generator_cond);
    pthread_cond_broadcast(&scheduler_cond);
    pthread_cond_broadcast(&timer_cond);

    // Espera que terminen los hilos
    pthread_join(clock_tid, NULL);
    pthread_join(timer_tid, NULL);
    pthread_join(process_generator_tid, NULL);
    pthread_join(scheduler_tid, NULL);

    // Limpia recursos
    destruir_process_queue();
    destruir_memoria_fisica();
    destruir_maquina();
    pthread_mutex_destroy(&clock_mutex);
    pthread_cond_destroy(&timer_cond);
    pthread_cond_destroy(&generator_cond);

    printf("\nSIMULACION FINALIZADA\n\n");

    return 0;
}