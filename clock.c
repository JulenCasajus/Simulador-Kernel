#include "clock.h"
#include "timer.h"
#include "scheduler.h"
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

// Variables
int clock_tick = 0;
int clock_activo;
int clock_visible;
pthread_mutex_t clock_mutex = PTHREAD_MUTEX_INITIALIZER;

//------------------//
// Función de Clock //
//------------------//
void *clock_thread(void *arg) {
    while (clock_activo) {
        sleep(1); // 1 segundo por ciclo
        pthread_mutex_lock(&clock_mutex);
        clock_tick++;
        if (clock_visible) {
            printf("Clock tick: %d ciclos\n", clock_tick);
        }
        if (clock_tick % t_periodo == 0) {
            pthread_cond_signal(&timer_cond);
        }
        pthread_mutex_unlock(&clock_mutex);
    }
    return 0;
}