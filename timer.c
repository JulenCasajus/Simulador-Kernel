#include "timer.h"
#include "clock.h"
#include "loader.h"
#include "scheduler.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Variables
int t_periodo;
int timer_visible;
pthread_cond_t timer_cond = PTHREAD_COND_INITIALIZER;

//------------------//
// Función de Timer //
//------------------//
void *timer_thread(void *arg) {
    int tiempo = 0;
    int t_loader = rand() % 5 + 1;    // Espera aleatoria entre generacion de procesos (1-6 interrupciones del timer)
    while (clock_activo) {
        pthread_mutex_lock(&clock_mutex);
        pthread_cond_wait(&timer_cond, &clock_mutex);
        pthread_mutex_unlock(&clock_mutex);
        tiempo++;
        if (timer_visible) {
            printf("Timer: Interrupción del temporizador en %d ciclos\n", clock_tick);
        }
        if (tiempo % t_loader == 0) { // Enviar señal al process_generator
            t_loader = rand() % 5 + 1; // Nuevo tiempo aleatorio
            pthread_cond_signal(&generator_cond);
        }
        if (tiempo % t_scheduler == 0) { // Enviar señal al schedule
            pthread_cond_signal(&scheduler_cond);
        }
    }
    return 0;
}