#pragma once

#include "process_generator.h"

// Definición de estructuras
typedef struct {
    int *entradas; // Array de entradas de la TLB (cada entrada almacena un par página-marco)
    int tamaño;    // Tamaño de la TLB (número de entradas)
    int contador;  // Contador de accesos, para reemplazo en la TLB
} TLB;

typedef struct {
    int *TLB; // Puntero al TLB (Translation Lookaside Buffer)
} MMU;

typedef struct {
    int id_hilo; // Identificador único para el hilo
    PCB *pcb;    // pcb 
    int PC;      // Contador de Programa (Program Counter)
    int RI;      // Registro de Instrucción (Instruction Register)
    int PTBR;   // Puntero a la tabla de páginas (Page Table Base Register)
    MMU mmu;     // Unidad de Gestión de Memoria asociada al hilo
} Hilo;

typedef struct {
    int id_core;   // Identificador del core
    int num_hilos; // Número de hilos del core
    Hilo *hilos;   // Array de hilos del core
} Core;

typedef struct {
    int id_cpu;    // Identificador de la CPU
    int num_cores; // Número de cores de la CPU
    Core *cores;   // Array de cores de la CPU
} CPU;

typedef struct {
    int num_cpus; // Número de CPUs
    CPU *cpus;    // Array de CPUs
} Maquina;

// Variables externas
extern Maquina maquina;

// Declaración de funciones
void imprimir_info_maquina();
void destruir_maquina();
Hilo* siguiente_hilo();