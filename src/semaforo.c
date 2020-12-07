/*=============================================================================
 * Author: Gonzalo Sanchez <na.gonzalo@gmail.com>
 * Date: 2020/11/04
 * Version: 1
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include "sapi.h"
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "sapi_datatypes.h"
#include "sapi_peripheral_map.h"
#include "sapi_delay.h"
#include "sapi_rtc.h"
#include "semaforo.h"

/*=====[Definitions of private global variables]=============================*/
#define CANT_SEC_MAX 20 // cantidad máxima de secuencias que se pueden crear
#define CANT_EST_MAX 20 // cantidad máxima de estados que puede contener una secuncia.

static modo_sem modo_actual = manual;
static uint32_t last_added = 0;
static uint32_t sec_actual = 0;

static secuencia_t secuencias[CANT_SEC_MAX];

/*=====local functions==========*/

void init_array_sec(void)
{
    uint32_t i;

    for(i = 0 ; i < CANT_SEC_MAX ; i++)
    {
        secuencias[i].estado = NULL;
        secuencias[i].indice = 0;
        secuencias[i].lenght = 0;
        memset(secuencias[i].nombre, 0, sizeof(((secuencia_t *)0)->nombre));
    }

}

void apagar_leds(void)
{
    gpioWrite( LEDR, FALSE);
    gpioWrite( LEDG, FALSE);
    gpioWrite( LEDB, FALSE);
    gpioWrite( LEDB, FALSE);
    gpioWrite( LED1, FALSE);
    gpioWrite( LED2, FALSE);
    gpioWrite( LED3, FALSE);
}

void init_secuencia(secuencia_t* sec)
{
    if(NULL == sec)
        return;

    if(NULL == sec->estado)
        return;

    delayInit( &(sec->delay), sec->estado[0].pausa );
    delayRead(&(sec->delay));
    sec->indice = 0;

    apagar_leds();
    if(sec->estado[0].luces & LUZ_ROJ)
        gpioWrite( LEDR , TRUE);
    if(sec->estado[0].luces & LUZ_AMA)
        gpioWrite( LEDG , TRUE);
    if(sec->estado[0].luces & LUZ_VER)
        gpioWrite( LEDB , TRUE);
}
/*=====Global functions==========*/

secuencia_t* agregar_secuencia(uint8_t cantidad, char* nombre)
{
    uint32_t i = 0;

    /*Corroboro que haya espacion para agregar la secuencia*/
    if(CANT_SEC_MAX <= last_added )
        return NULL;
    /*corroboro que no se quiera crear una secuencia mas larga que lo permitido*/
    if(CANT_EST_MAX < cantidad)
        return NULL;

    secuencias[last_added].estado = (estado_t*) malloc(sizeof(estado_t)* cantidad);

    /*corroboro que se haya podido reservar el espacio de memoria*/
    if(NULL == secuencias[last_added].estado)
        return NULL;

    secuencias[last_added].lenght = cantidad;
    secuencias[last_added].indice = 0;
    secuencias[last_added].alarma.on = false;
    secuencias[last_added].alarma.hora = 0;
    secuencias[last_added].alarma.minuto = 0;
    strncpy(secuencias[last_added].nombre, nombre, sizeof(((secuencia_t *)0)->nombre) - 1 );

    /*inicializo estados por defecto*/
    for(i = 0; i < cantidad; i++)
    {
        secuencias[last_added].estado[i].luces = 0;
        secuencias[last_added].estado[i].pausa = 0;
    }


    last_added++;
    return &secuencias[last_added-1];
}

secuencia_t* reemplazar_secuencia(uint8_t indice, uint8_t cantidad, char* nombre)
{
    uint32_t i = 0;

    /*Corroboro que no se pretenda ir mas alla de la ultima agregada*/
    if(last_added <= indice )
        return NULL;
    /*corroboro que no se quiera crear una secuencia mas larga que lo permitido*/
    if(CANT_EST_MAX < cantidad)
        return NULL;
    /*Si la posición ya estaba utilizada libero la memoria de la secuncia anterior*/
    if(NULL != secuencias[indice].estado)
        free((void*) secuencias[indice].estado);

    secuencias[indice].estado = (estado_t*) malloc(sizeof(estado_t)* cantidad);

    /*corroboro que se haya podido reservar el espacio de memoria*/
    if(NULL == secuencias[indice].estado)
        return NULL;

    secuencias[indice].lenght = cantidad;
    secuencias[indice].indice = 0;
    secuencias[last_added].alarma.on = false;
    secuencias[last_added].alarma.hora = 0;
    secuencias[last_added].alarma.minuto = 0;
    strncpy(secuencias[indice].nombre, nombre, sizeof(((secuencia_t *)0)->nombre) - 1 );

    /*inicializo estados por defecto*/
    for(i = 0; i < cantidad; i++)
    {
        secuencias[indice].estado[i].luces = 0;
        secuencias[indice].estado[i].pausa = 0;
    }

    return &secuencias[indice];
}

bool_t editar_estado(secuencia_t* secuencia, uint8_t indice,  uint32_t luces ,uint32_t pausa)
{
    if(NULL == secuencia)
        return false;

    if(NULL == secuencia->estado)
        return false;

    if(indice >= secuencia->lenght)
        return false;

    secuencia->estado[indice].luces = luces;
    secuencia->estado[indice].pausa = pausa;

    return true;
}

void set_modo_sem(modo_sem modo)
{
    modo_actual = modo;
}

void init_semaforo(void)
{

    rtcInit();
    init_array_sec();

    modo_actual = manual;
    last_added = 0;
    sec_actual = 0;

    /*inicializamos una secuencia por defecto*/
    secuencia_t* sec = agregar_secuencia(2, "Por defecto");

    editar_estado(sec, 0, LUZ_AMA , 500);
    editar_estado(sec, 1, 0 , 500);

    init_secuencia(sec);
}

bool_t avanzar_secuencia(void)
{
    if (reloj == modo_actual)
        return false;

    sec_actual++;

    if(sec_actual >= last_added)
        sec_actual = 0;

    init_secuencia(secuencias + sec_actual);
    return true;
}

bool_t retroceder_secuencia(void)
{
    if (reloj == modo_actual)
        return false;

    if(sec_actual > 0)
        sec_actual--;

    sec_actual = last_added - 1;

    init_secuencia(secuencias + sec_actual);

    return true;
}

bool_t set_indice(uint32_t num)
{
    if (reloj == modo_actual)
        return false;

    if(num >= last_added)
        return false;

    sec_actual = num;

    init_secuencia(secuencias + sec_actual);

    return true;
}

uint32_t get_indice(void)
{
    return sec_actual;
}

uint32_t get_last(void)
{
    return last_added;
}

secuencia_t* get_secuencia(uint32_t num)
{
    if(CANT_SEC_MAX > num)
        return secuencias+num;

    return NULL;
}

secuencia_t* get_secuencia_actual()
{
    return secuencias+sec_actual;
}

uint32_t find_sequence_by_name(char* name, secuencia_t* sec)
{
    uint32_t i = 0;

    for(i = 0; i < CANT_SEC_MAX ; i++)
    {
        if(!strcmp(secuencias[i].nombre, name))
        {
            sec = secuencias + i;
            return  i;
        }
    }

    sec = NULL;
    return 0xFFFFFFFF;
}

void check_alarma()
{
    uint32_t j;

    rtc_t rtc;
    rtcRead(&rtc);

    for(j = 0 ; j < last_added ; j++)
    {
        if(secuencias[j].alarma.hora == rtc.hour && secuencias[j].alarma.minuto == rtc.min)
        {
            sec_actual = j;
            break;
        }
    }
}

void ejecutar_secuencia(void)
{

    secuencia_t* sec;

    if (reloj == modo_actual)
    {
        check_alarma();
    }



    sec = secuencias + sec_actual;

    if(sec == NULL)
        return;

    if(delayRead(&(sec->delay)))// 100ms * 100 = 10s
    {

       sec->indice++;
       if(sec->lenght == sec->indice)
           sec->indice = 0;

       delayInit( &(sec->delay) , sec->estado[sec->indice].pausa );
       delayRead(&(sec->delay));

       apagar_leds();
       if(sec->estado[sec->indice].luces & LUZ_ROJ)
           gpioWrite( LEDR , TRUE);
       if(sec->estado[sec->indice].luces & LUZ_AMA)
           gpioWrite( LEDG , TRUE);
       if(sec->estado[sec->indice].luces & LUZ_VER)
           gpioWrite( LEDB , TRUE);

    }
}



