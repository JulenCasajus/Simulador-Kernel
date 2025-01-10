#pragma once

#include <pthread.h>

// Definición de estructuras
typedef struct {
    int code; // Dirección virtual del segmento de código
    int data; // Dirección virtual del segmento de datos
    int pgb;  // Dirección física de la tabla de páginas
} MemoryManagement;

typedef struct {
    int pid;             // Identificador del proceso
    int t_vida;          // Tiempo de vida (aleatorio)
    int prioridad;       // Prioridad del proceso
    int estado;          // 0: Ready, 1: Ejecutando, 2: Terminado
    MemoryManagement mm; // Memoria del proceso
} PCB;

typedef enum {
    FCFS, // First Come First Serve
    RR,   // Round Robin
    SJF,  // Shortest Job First
    PRIO  // Priority Scheduling
} PoliticaPlanificacion;

typedef struct {
    PCB **procesos;           // Array dinámico de punteros a PCBs
    int cara;                 // Índice del frente de la cola
    int culo;                 // Índice trasero de la cola
    int capacidad;            // Capacidad máxima
    int num_procesos;         // Número de procesos en cola
    pthread_mutex_t mutex;    // Mutex para acceso concurrente
    pthread_cond_t condicion; // Condición para esperar procesos
} ProcessQueue;

// Variables externas
extern PoliticaPlanificacion politica;
extern ProcessQueue pQ;
extern int cola_dinamica;
extern pthread_cond_t generator_cond;

// Declaración de funciones
void *generator_thread(void *arg);
void iniciar_process_queue(int capacidad);
void destruir_process_queue();
void encolar_proceso(PCB *pcb);
PCB *siguiente_proceso();
void desencolar_procesos();