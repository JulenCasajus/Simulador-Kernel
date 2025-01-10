#include "maquina.h"
#include <stdio.h>
#include <stdlib.h>

// Variables
Maquina maquina;

// Imprime la configuracion de la maquina
void imprimir_info_maquina() {
    // Imprimir número de CPUs
    printf("Número de CPUs: %d\n", maquina.num_cpus);

    // Recorrer cada CPU
    for (int i = 0; i < maquina.num_cpus; i++) {
        CPU cpu = maquina.cpus[i];
        printf("CPU %d:\n", cpu.id_cpu);
        
        // Imprimir número de cores por CPU
        printf("  Número de cores: %d\n", cpu.num_cores);
        
        // Recorrer cada core de la CPU
        for (int j = 0; j < cpu.num_cores; j++) {
            Core core = cpu.cores[j];
            printf("  Core %d:\n", core.id_core);
            
            // Imprimir número de hilos por core
            printf("    Número de hilos: %d\n", core.num_hilos);
            
            // Recorrer cada hilo del core
            for (int k = 0; k < core.num_hilos; k++) {
                Hilo hilo = core.hilos[k];
                printf("    Hilo %d\n", hilo.id_hilo);
            }
        }
    }
}

// Libera la maquina
void destruir_maquina() {
    for (int i = 0; i < maquina.num_cpus; i++) {
        for (int j = 0; j < maquina.cpus[i].num_cores; j++) {
            free(maquina.cpus[i].cores[j].hilos);
        }
        free(maquina.cpus[i].cores);
    }
    free(maquina.cpus);
    maquina.num_cpus = 0;
}

// Devuelve el siguiente hilo ocioso
Hilo* siguiente_hilo() {
    // Iterar sobre los cores de la máquina
    for (int i = 0; i < maquina.num_cpus; i++) {
        CPU cpu = maquina.cpus[i];
        // Iterar sobre los hilos de cada core
        for (int j = 0; j < cpu.num_cores; j++) {
            Core core = cpu.cores[j];
            // Iterar sobre los hilos de cada core
            for (int k = 0; k < core.num_hilos; k++) {
                Hilo* hilo = &core.hilos[k];
                // Verificar si el hilo no tiene PCB
                if (hilo->pcb == NULL) {
                    return hilo; // Devolver el primer hilo que no tiene PCB
                }
            }
        }
    }
    return NULL;
}