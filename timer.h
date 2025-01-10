#pragma once

#include <pthread.h>

// Variables externas
extern int t_periodo;
extern int timer_visible;
extern pthread_cond_t timer_cond;

// Declaración de funciones
void *timer_thread(void *arg);