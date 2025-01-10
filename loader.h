#pragma once

#include "process_generator.h"

// Variables externas
extern char carpeta_procesos[];

// Declaración de funciones
void *loader_thread(void *arg);
int cargar_programa(const char *archivo, PCB *pcb);