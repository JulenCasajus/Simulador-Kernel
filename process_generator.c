#include "process_generator.h"
#include "scheduler.h"
#include "memoria.h"
#include "loader.h"
#include "clock.h"
#include <stdio.h>
#include <stdlib.h>

// Variables
int cola_dinamica;
ProcessQueue pQ;
PoliticaPlanificacion politica;
pthread_cond_t generator_cond = PTHREAD_COND_INITIALIZER;

// Función para inicializar la cola de procesos
void iniciar_process_queue(int capacidad) {
    pQ.procesos = malloc(capacidad * sizeof(PCB *));
    for (int i = 0; i < capacidad; i++) {
        pQ.procesos[i] = NULL;
    }
    pQ.cara = 0;
    pQ.culo = 0;
    pQ.capacidad = capacidad;
    pQ.num_procesos = 0;
    pthread_mutex_init(&pQ.mutex, NULL);
    pthread_cond_init(&pQ.condicion, NULL);
}

// Función para destruir la cola de procesos
void destruir_process_queue() {
    free(pQ.procesos);
    pthread_mutex_destroy(&pQ.mutex);
    pthread_cond_destroy(&pQ.condicion);
}

// Función para encolar un proceso en la cola
void encolar_proceso(PCB *pcb) {
    int next_culo = (pQ.culo + 1) % pQ.capacidad;
    int desechado = 0;

    if (next_culo == pQ.cara) { // Cola llena
        if (cola_dinamica) {
            int nueva_capacidad = pQ.capacidad + 1;
            printf("\n/!\\ COLA LLENA: aumentando capacidad a %d\n\n", nueva_capacidad - 1);
            PCB **nueva_lista = malloc(nueva_capacidad * sizeof(PCB *));
            for (int i = 0, idx = pQ.cara; i < pQ.capacidad - 1; i++) {
                nueva_lista[i] = pQ.procesos[idx];
                idx = (idx + 1) % pQ.capacidad;
            }
            free(pQ.procesos);
            pQ.procesos = nueva_lista;
            pQ.cara = 0;
            pQ.culo = pQ.capacidad - 1;
            pQ.capacidad = nueva_capacidad;
            next_culo = nueva_capacidad - 1;
        } else {
            printf("\n/!\\ COLA LLENA: desechando proceso PID = %d\n\n", pcb->pid);
            destruir_tabla(pcb);
            desechado = 1;
        }
    }

    if (!desechado) {
        int idx = pQ.culo;
        if ((politica == SJF || politica == PRIO)) { // Insertar en orden (SJF / PRIO)
            int i = 0;
            int encontrado = 0;
            while (i < pQ.num_procesos && !encontrado) {
                int idx_prev = ((idx - 1) + pQ.capacidad) % pQ.capacidad;
                if ((politica == SJF && (pcb->t_vida < pQ.procesos[idx_prev]->t_vida)) ||
                    (politica == PRIO && (pcb->prioridad < pQ.procesos[idx_prev]->prioridad))) {
                    pQ.procesos[idx] = pQ.procesos[idx_prev];
                    idx = idx_prev;
                    i++;
                } else {
                    encontrado = 1;
                }
            }
        }
        pQ.procesos[idx] = pcb;
        printf("ENCOLADO PID = %d en posición %d\n", pcb->pid, ((idx - pQ.cara + pQ.capacidad) % pQ.capacidad) + 1);

        pQ.culo = next_culo;
        pQ.num_procesos = pQ.num_procesos + 1;

        printf("\nCOLA ACTUAL (PID): [ %d", pQ.procesos[pQ.cara]->pid);
        for (int i = 1; i < pQ.num_procesos; i++) {
            printf(", %d", pQ.procesos[(pQ.cara + i) % pQ.capacidad]->pid);
        }
        printf(" ]\n\n");
    }
}

// Función para desencolar un proceso de la cola
void desencolar_procesos() {
    int pos = pQ.cara;
    int i = 0;
    while (i < pQ.num_procesos) {
        if (pQ.procesos[(pos + i) % pQ.capacidad] != NULL && pQ.procesos[(pos + i) % pQ.capacidad]->estado == 2) {
            if (i == 0) {
                pQ.cara = (pQ.cara + 1) % pQ.capacidad;
                i++;
            } else {
                for (int j = i; j < pQ.num_procesos - 1; j++) {
                    pQ.procesos[(pos + j) % pQ.capacidad] = pQ.procesos[(pos + j + 1) % pQ.capacidad];
                }
            }
            pQ.num_procesos--;
            pQ.procesos[(pos + pQ.num_procesos) % pQ.capacidad] = NULL;
        } else {
            i++;
        }
    }
}

// Devuelve el siguiente proceso READY
PCB *siguiente_proceso() {
    if (pQ.cara == pQ.culo && clock_activo) {
        printf("\n/!\\ COLA DE PROCESOS VACIA\n\n");
        return NULL;
    }
    PCB *pcb = NULL;
    int pos = pQ.cara;
    for (int i = 0; i < pQ.num_procesos; i++) {
        if (pQ.procesos[(pos + i) % pQ.capacidad]->estado == 0) {
            pcb = pQ.procesos[(pos + i) % pQ.capacidad];
            pcb->estado = 1;
            return pcb;
        }        
    }
    return NULL;
}

//----------------------------------//
// Función de Generador de Procesos //
//----------------------------------//
void *generator_thread(void *arg) {
    int pid_counter = 1;
    while (clock_activo) {
        pthread_mutex_lock(&clock_mutex);
        pthread_cond_wait(&generator_cond, &clock_mutex);
        pthread_mutex_unlock(&clock_mutex);
        desencolar_procesos(); // Desencolar procesos terminados
        PCB *new_pcb = malloc(sizeof(PCB));
        if (cargar_programa(carpeta_procesos, new_pcb) == 0) {
            new_pcb->prioridad = rand() % 10 + 1;
            new_pcb->pid = pid_counter++;
            printf("GENERADO PID = %d (t_vida = %ds, prioridad = %d)\n", new_pcb->pid, new_pcb->t_vida, new_pcb->prioridad);
            encolar_proceso(new_pcb);
        } else { // Error al cargar el programa
            free(new_pcb);
        }
    }
    return 0;
}