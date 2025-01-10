#include "memoria.h"
#include "process_generator.h"
#include <stdlib.h>
#include <stdio.h>

MemoriaFisica memoria_fisica;
int memoria_visible;

//--------------------------------------------------------------//
// Funciones para inicializar y destruir estructuras de memoria // 
//--------------------------------------------------------------//
void inicializar_marcos() {
    for (int i = 0; i < CANTIDAD_MARCOS; i++) {
        memoria_fisica.marcos[i].num = i;             // Número de marco
        memoria_fisica.marcos[i].ocupado = 0;         // Libre
        memoria_fisica.marcos[i].pid = -1;            // Ningún proceso
    }
}

void inicializar_memoria_fisica() {
    memoria_fisica.capacidad = TAMANO_MEMORIA_FISICA;
    memoria_fisica.dir_next_tabla = 0;
    memoria_fisica.next_marco = 0;
    memoria_fisica.memoria = malloc(TAMANO_MEMORIA_FISICA); // Tamaño palabra (4B) y 24 bits de bus de direcciones = 64 MB
    pthread_mutex_init(&memoria_fisica.mutex, NULL);        // Inicializar el mutex
    inicializar_marcos();                                   // Inizializar marcos
}

void destruir_memoria_fisica() {
    free(memoria_fisica.memoria);
    pthread_mutex_destroy(&memoria_fisica.mutex);
}

void crear_tabla (int paginas_totales, int *marcos_libres) {                                           //             Tabla:
    memoria_fisica.memoria[memoria_fisica.dir_next_tabla] = paginas_totales;                           //         Paginas_totales
    for (int i = 0; i < paginas_totales; i++) {                                                        //  Num_Pagina, Num_Marco, Valida
        memoria_fisica.dir_next_tabla = (memoria_fisica.dir_next_tabla + 1) % DIR_MAX_KERNEL;          // [Num_Pagina, Num_Marco, Valida]
        memoria_fisica.memoria[memoria_fisica.dir_next_tabla] = i;                                     // [Num_Pagina, Num_Marco, Valida]
        memoria_fisica.dir_next_tabla = (memoria_fisica.dir_next_tabla + 1) % DIR_MAX_KERNEL;          //               ...
        memoria_fisica.memoria[memoria_fisica.dir_next_tabla] = marcos_libres[i];
        memoria_fisica.dir_next_tabla = (memoria_fisica.dir_next_tabla + 1) % DIR_MAX_KERNEL;
        memoria_fisica.memoria[memoria_fisica.dir_next_tabla] = 1;
        if (memoria_visible) {
            printf("Página %d asignada al marco %d\n", i, marcos_libres[i]);
        }
    }
    memoria_fisica.dir_next_tabla += (memoria_fisica.dir_next_tabla + 1) % DIR_MAX_KERNEL;
}

void destruir_tabla (PCB *pcb) {
    int paginas_totales = memoria_fisica.memoria[pcb->mm.pgb];
    if (memoria_visible) {
        printf("\nDATOS DE MANEJO DE MEMORIA\n");
        printf("Paginas totales TABLA: %d\n", paginas_totales);
    }
    for (int i = 0; i < paginas_totales; i++) {
        int direccion_pagina = pcb->mm.pgb + 1 + i * 3;
        memoria_fisica.memoria[direccion_pagina + 2] = 0;
        memoria_fisica.marcos[memoria_fisica.memoria[direccion_pagina + 1]].ocupado = 0;
        if (memoria_visible) {
            printf("Marco %d desocupado\n", memoria_fisica.memoria[direccion_pagina + 1]);
        }
    }
    if (memoria_visible) {
        printf("\n");
    }
    pcb->mm.pgb = -1;
}

//----------------------------------------------//
// Función para buscar el siguiente marco libre //
//----------------------------------------------//
int buscar_marcos(int *marcos_libres, int paginas_totales) {
    int next_marco = memoria_fisica.next_marco;
    int encontradas= 0;
    int i = 0;
    while (i < CANTIDAD_MARCOS && encontradas < paginas_totales) {
        if (!memoria_fisica.marcos[next_marco].ocupado) {
            marcos_libres[encontradas] = memoria_fisica.marcos[next_marco].num;
            encontradas++;
        }
        next_marco = (next_marco + 1) % CANTIDAD_MARCOS;
    }
    if (encontradas != paginas_totales) {
        printf("\n/!\\ NO HAY SUFICIENTES MARCOS LIBRES EN LA RAM\n\n");
        return -1;
    }
    memoria_fisica.next_marco = next_marco;
    return 0;
}