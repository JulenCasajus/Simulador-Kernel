#include "loader.h"
#include "process_generator.h"
#include "clock.h"
#include "memoria.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

// Variables
char carpeta_procesos[256];
int archivo_actual = 0;

//-----------------------------------------------//
// Función para buscar el siguiente archivo .elf //
//-----------------------------------------------//
FILE *siguiente_archivo(const char *carpeta_procesos) {
    DIR *dir = opendir(carpeta_procesos);
    if (!dir) {
        perror("Error al abrir la carpeta");
        return NULL;
    }
    struct dirent *entry;
    int contador = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG && strstr(entry->d_name, ".elf")) {
            if (contador == archivo_actual) {
                char ruta_completa[256];
                snprintf(ruta_completa, sizeof(ruta_completa), "%s/%s", carpeta_procesos, entry->d_name);
                archivo_actual++;
                closedir(dir);
                FILE *file = fopen(ruta_completa, "rb");
                if (!file) {
                    perror("Error al abrir el archivo");
                    return NULL;
                }
                if (memoria_visible) {
                    printf("\nDATOS DE MANEJO DE MEMORIA\n");
                    printf("Archivo abierto: %s\n", ruta_completa);
                }
                return file;
            }
            contador++;
        }
    }
    closedir(dir);
    return NULL;
}

//---------------------------------//
// Función para Cargar un Programa //
//---------------------------------//
int cargar_programa(const char *carpeta_procesos, PCB *pcb) {
    
    FILE *file = siguiente_archivo(carpeta_procesos);
    if (file == NULL) {
        printf("\n/!\\ No hay mas procesos a ejecutar\n\n");
        return -1;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);  // Obtener el tamaño del archivo
    fseek(file, 0, SEEK_SET); // Volver al inicio del archivo
    if (memoria_visible) {
        printf("Tamaño del archivo: %ld\n", size);
    }
    char *buffer = malloc(size);
    if (!buffer) {
        perror("Error al asignar memoria para el buffer");
        fclose(file);
        return -1;
    }

    fread(buffer, 1, size, file); // Leer el archivo completo en el buffer

    int escrito = 0;                             // Cuantos bytes se han escrito dentro del marco/pagina
    int comienzo_text;                           // En qué byte comienza text
    int comienzo_data;                           // En qué byte comienza data
    int tamano_text;                             // Tamaño en bytes de text
    int tamano_data;                             // Tamaño en bytes de data
    int paginas_text;                            // Paginas necesarias para text
    int paginas_data;                            // Paginas necesarias para data
    int paginas_totales;                         // Paginas totales necesarias
    unsigned char palabra[TAMANO_PALABRA] = {0}; // Buffer para escribir cada linea en memoria

    char *linea = strtok(buffer, "\n");

    sscanf(linea + 6, "%x", &comienzo_text);
    if (memoria_visible) {
        printf(".text: comienzo_text = %d (0x%06X)\n", comienzo_text, comienzo_text);
    }
    linea = strtok(NULL, "\n");

    sscanf(linea + 6, "%x", &comienzo_data);
    if (memoria_visible) {
        printf(".data: comienzo_data = %d (0x%06X)\n", comienzo_data, comienzo_data);
    }
    linea = strtok(NULL, "\n");

    tamano_text = comienzo_data - comienzo_text;
    paginas_text = (tamano_text + TAMANO_PAGINA - 1) / TAMANO_PAGINA;

    tamano_data = ((size-26)*4)/9 - comienzo_data;
    paginas_data = (tamano_data + TAMANO_PAGINA - 1) / TAMANO_PAGINA;

    paginas_totales = paginas_text + paginas_data;

    int *marcos_libres = malloc(paginas_totales * sizeof(int));

    if(buscar_marcos(marcos_libres, paginas_totales) == -1) {
        fclose(file);
        return -1;
    }

    pcb->mm.code = comienzo_text;
    pcb->mm.data = comienzo_data;
    pcb->mm.pgb = memoria_fisica.dir_next_tabla;
    crear_tabla(paginas_totales, marcos_libres);

    for (int i = 0; i < paginas_text; i++) {
        while (linea != NULL) {
            for (int j = 0; j < TAMANO_PALABRA; j++) {
                sscanf(linea + (j * 2), "%2hhx", &palabra[j]);
            }
            memcpy(memoria_fisica.memoria + DIR_MAX_KERNEL + (marcos_libres[i] * TAMANO_PAGINA) + escrito, palabra, TAMANO_PALABRA);
            if (memoria_visible) {
                printf("Instrucción guardada: %02X %02X %02X %02X\n", palabra[0], palabra[1], palabra[2], palabra[3]);
            }
            escrito += TAMANO_PALABRA;
            if (memoria_visible) {
                printf("Escrito: %d\n", escrito);
            }
            linea = strtok(NULL, "\n");
            if (escrito == TAMANO_PAGINA || escrito == tamano_text) {
                escrito = 0;
                break;
            }
        }
    } 
    if (memoria_visible) {
        printf("Parte de text completada: %d bytes\n", tamano_text);
    }
    for (int i = 0; i < paginas_data; i++) {
        while (linea != NULL) {
            for (int j = 0; j < TAMANO_PALABRA; j++) {
                sscanf(linea + (j * 2), "%2hhx", &palabra[j]);
            }
            memcpy(memoria_fisica.memoria + DIR_MAX_KERNEL + (marcos_libres[i + paginas_text] * TAMANO_PAGINA) + escrito, palabra, TAMANO_PALABRA);
            if (memoria_visible) {
                printf("Instrucción guardada: %02X %02X %02X %02X\n", palabra[0], palabra[1], palabra[2], palabra[3]);
            }
            escrito += TAMANO_PALABRA;
            if (memoria_visible) {
                printf("Escrito: %d\n", escrito);
            }
            linea = strtok(NULL, "\n");
            if (escrito == TAMANO_PAGINA) {
                escrito = 0;
                break;
            }
        }
    }
    if (memoria_visible) {
        printf("Parte de data completada: %d bytes\n", escrito);
        printf("Numero de instrucciones de text: %d\n", tamano_text/4);
        printf("Numero de instrucciones de data: %d\n", tamano_data/4);
        printf("Páginas para el código: %d\n", paginas_text);
        printf("Páginas para los datos: %d\n", paginas_data);
        printf("Total de páginas: %d\n\n", paginas_totales);
    }

    pcb->t_vida = (tamano_text/4); // 1 ciclo por instruccion

    free(buffer);
    free(marcos_libres);
    fclose(file);
    return 0;
}