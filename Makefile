# Nombre del ejecutable
TARGET = kernel

# Archivos fuente
SRCS = main.c \
       process_generator.c \
       maquina.c \
       memoria.c \
       loader.c \
       clock.c \
       timer.c \
       scheduler.c

# Opciones del compilador
CFLAGS = -O2 -pthread

# Reglas
all: $(TARGET)

$(TARGET): $(SRCS)
	gcc $(CFLAGS) $(SRCS) -o $(TARGET)

# Limpieza
clean:
	rm -f $(TARGET)
