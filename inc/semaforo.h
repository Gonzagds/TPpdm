/*
 * semaforo.h
 *
 *  Created on: Nov 4, 2020
 *      Author: gonzalo
 */
#include <stdint.h>

#define LUZ_OFF 0
#define LUZ_ROJ (1 << 0)
#define LUZ_AMA (1 << 1)
#define LUZ_VER (1 << 2)

typedef struct
{
    uint32_t luces;
    uint32_t pausa;
}estado_t;

typedef struct
{
    bool_t on;
    uint8_t hora;
    uint8_t minuto;
}alarma_t;

typedef struct
{
    delay_t delay;
    estado_t *estado;
    uint8_t lenght;
    uint8_t indice;
    alarma_t alarma;
    char nombre[20];
}secuencia_t;

typedef enum
{
    manual = 0,
    reloj
}modo_sem;



void ejecutar_secuencia(void);

void set_modo_sem(modo_sem modo);

/* agregar_secuncia() reserva el espacio de memoria necesario para un secuencia de una cantidad dada de estado.
 * La secuncia se agrega al final de la pila de secuncias. Por defecto se inicializan todas las
 * pausas en cero y las luces apagadas en todos los estados.
 *
 * cantidad: cantidad de estados que tendrá la secuencia
 * nombre: nombre de fantasia de la nueva secuencia.
 *
 * return puntero a la secunecia agregada en caso de exito
 * return NULL si sucede que:
 *  1 - se alcanzó la cantidad máxima de secuncias.
 *  2 - la cantidad de estados es mayor a la admitida
 *  3- no se pudo reservar memoria para los estados.
 *
 * */
secuencia_t* agregar_secuencia(uint8_t cantidad, char* nombre);

/* reemplazar_secuencia() reemplaza una secuencia existente. Primero libera la
 * memoria de la secuencia anterior y luego reserva memoria para la nueva secuncia
 * Por defecto se inicializan todas las
 * pausas en cero y las luces apagadas en todos los estados.
 *
 * indice: indice dentro de la pila de secuncias creadas
 * cantidad: cantidad de estados que tendrá la secuencia
 * nombre: nombre de fantasia de la nueva secuencia.
 *
 * return puntero a la secunecia reemplazada en caso de exito
 * return NULL si sucede que:
 *  1 - si indice es mayor que el máximo admitido o apunta a una sec no inicializada.
 *  2 - la cantidad de estados es mayor a la admitida
 *  3- no se pudo reservar memoria para los estados.
 *
 * */
secuencia_t* reemplazar_secuencia(uint8_t indice, uint8_t cantidad, char* nombre);


/* edita_estado() edita los atributos de un estado dado de la secuncia provista.
 *
 * secuencia: puntero a la secuncia cuyo estado se quiere editar
 * indice: numero de estado a editar dentro de la secuncia
 * luces: luces a encender en el estado
 * pausa: duracion del estado en milisegundos
 *
 * return true en caso de exito
 * return false si
 *  1- el estado apunta a un NULL
 *  2- el indice es mayor a la cantidad de estados
 *
 * */
bool_t editar_estado(secuencia_t* secuencia, uint8_t indice,  uint32_t luces ,uint32_t pausa);

void init_semaforo(void);

bool_t avanzar_secuencia(void);

bool_t retroceder_secuencia(void);

bool_t set_indice(uint32_t num);

uint32_t get_indice(void);

uint32_t get_last(void);

secuencia_t* get_secuencia(uint32_t num);

secuencia_t* get_secuencia_actual(void);

uint32_t find_sequence_by_name(char* name, secuencia_t* sec);


