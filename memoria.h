#pragma once

#include "process_generator.h"
#include <pthread.h>

#define TAMANO_MEMORIA_FISICA (1 << 26)  // 64 MB = 2^26 bytes
#define CANTIDAD_MARCOS 15360            // 60MB para marcos de 4KB
#define DIR_MAX_KERNEL 4194304           // Tablas de paginas desde 0x000000 hasta 0x3FFFFF
#define TAMANO_PAGINA 4096               // Paginas de 4KB
#define TAMANO_PALABRA 4                 // Una palabra = 4 bytes (8 caracteres hexadecimales)

// Definición de estructuras
typedef struct {
    int num_pagina; // Número de página
    int num_marco;  // Número de marco en la memoria fisica
    int valida;     // Bandera de validez (0: no válido, 1: válido)
} Pagina;

typedef struct {
    int num;     // Número de marco
    int ocupado; // Indica si el marco está ocupado (0: libre, 1: ocupado)
    int pid;     // PID del proceso que ocupa este marco
} Marco;

typedef struct {
    int num_paginas; // Número de páginas de la memoria virtual
    Pagina *paginas; // Array de páginas del proceso
} TablaPaginas;

typedef struct {
    int *memoria;                  // Array que simula la memoria física (cada celda tiene 4 bytes)
    int capacidad;                 // Tamaño total de la memoria (en palabras)
    int next_marco;                // Proximo marco
    Marco marcos[CANTIDAD_MARCOS]; // Array de datos de marcos
    int dir_next_tabla;            // Posicion de la proxima tabla
    pthread_mutex_t mutex;         // Mutex para acceso concurrente
} MemoriaFisica;

// Variables externas
extern MemoriaFisica memoria_fisica;
extern int memoria_visible;

// Declaración de funciones
void inicializar_marcos();
void inicializar_memoria_fisica();
void destruir_memoria_fisica();
void crear_tabla (int paginas_totales, int *marcos_libres);
void destruir_tabla(PCB *pcb);
int buscar_marcos(int *marcos_libres, int paginas_totales);