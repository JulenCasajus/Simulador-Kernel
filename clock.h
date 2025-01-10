#pragma once

#include <pthread.h>

// Variables externas
extern int clock_tick;
extern int clock_activo;
extern int clock_visible;
extern pthread_mutex_t clock_mutex;

// Declaración de funciones
void *clock_thread(void *arg);