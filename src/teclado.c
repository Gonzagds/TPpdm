/*
 * teclado.c
 *
 *  Created on: Nov 18, 2020
 *      Author: gonzalo
 */
#include "sapi.h"
#include <stdint.h>
#include <stddef.h>
#include "sapi_datatypes.h"
#include "sapi_peripheral_map.h"
#include "sapi_delay.h"
#include "semaforo.h"
#include "teclado.h"

#define DELAY_REBOTE 40


typedef enum
{
    up = 0,
    falling,
    down,
    rising
}est_tec;

typedef enum
{
    key_undefined = 0,
    key_down,
    key_up,
    key_released,
    key_pressed
}evento_tec;

typedef struct
{
    delay_t delay;
    est_tec estado;
    gpioMap_t tecla;
    evento_tec evento;

}tecla_t;

tecla_t tecla1 =
{
    .estado = up,
    .tecla = TEC1,
    .evento = key_undefined
};

tecla_t tecla2 =
{
    .estado = up,
    .tecla = TEC2,
    .evento = key_undefined
};

tecla_t tecla3 =
{
    .estado = up,
    .tecla = TEC3,
    .evento = key_undefined
};

tecla_t tecla4 =
{
    .estado = up,
    .tecla = TEC4,
    .evento = key_undefined
};

void lee_tecla(tecla_t* tecla)
{
    bool_t lectura;

    if((void*) tecla == NULL)
        return;

    switch (tecla->estado)
    {
    case up:
        lectura = !gpioRead( tecla->tecla );
        if(!lectura)
        {
            tecla->evento = key_up;
        }
        else
        {
            tecla->estado = falling;
            delayInit( &(tecla->delay) , DELAY_REBOTE );
            delayRead(&(tecla->delay));
            tecla->evento = key_undefined;
        }
        break;
    case down:
        lectura = !gpioRead( tecla->tecla );
        if(lectura)
        {
            tecla->evento = key_down;
        }
        else
        {
            tecla->estado = rising;
            delayInit( &(tecla->delay) , DELAY_REBOTE );
            delayRead(&(tecla->delay));
            tecla->evento = key_undefined;
        }
        break;
    case falling:
        if(delayRead(&(tecla->delay)))
        {
            lectura = !gpioRead( tecla->tecla );

           if(lectura)
           {
               tecla->estado = down;
               tecla->evento = key_pressed;
           }
           else
           {
               tecla->estado = up;
               tecla->evento = key_up;
           }
        }
        else
            tecla->evento = key_undefined;
        break;
    case rising:
        if(delayRead(&(tecla->delay)))
        {
            lectura = !gpioRead( tecla->tecla );

           if(!lectura)
           {
               tecla->estado = up;
               tecla->evento = key_released;
           }
           else
           {
               tecla->estado = down;
               tecla->evento = key_down;
           }
        }
        else
            tecla->evento = key_undefined;
        break;
    }
}

bool_t leer_teclado(teclado_t* teclado)
{
      memset((void*) teclado, 0, sizeof(teclado_t));

      lee_tecla( &tecla1 );
      lee_tecla( &tecla2 );
      lee_tecla( &tecla3 );
      lee_tecla( &tecla4 );
      if( tecla1.evento == key_pressed)
      {
          teclado->tec_next = true;
      }
      else
      {
          teclado->tec_next = false;
      }

      if(tecla2.evento == key_pressed)
      {
          teclado->tec_prev = true;
      }
      else
      {
          teclado->tec_prev = false;
      }

      if(tecla3.evento == key_pressed)
      {
          teclado->tec_manu = true;
      }
      else
      {
          teclado->tec_manu = false;
      }

      if(tecla3.evento == key_pressed)
      {
          teclado->tec_alarm = true;
      }
      else
      {
          teclado->tec_alarm = false;
      }

      return true;
}

void procesa_teclado(void)
{
    teclado_t teclado;

    leer_teclado(&teclado);

    if(teclado.tec_next)
    {
        avanzar_secuencia();
    }
    if(teclado.tec_prev)
    {
        retroceder_secuencia();
    }
    if(teclado.tec_manu)
    {
        set_modo_sem(manual);
    }
    if(teclado.tec_manu)
    {
        set_modo_sem(reloj);
    }
}

