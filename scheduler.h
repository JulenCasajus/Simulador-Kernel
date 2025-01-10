#pragma once

#include <pthread.h>

// Variables externas
extern int t_max_rr;
extern int t_scheduler;
extern int scheduler_visible;
extern pthread_cond_t scheduler_cond;

// Declaración de funciones
void *scheduler_thread(void *arg);