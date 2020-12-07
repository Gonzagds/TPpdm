/*
 * consola.c
 *
 *  Created on: Nov 25, 2020
 *      Author: gonzalo
 */

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "sapi.h"
#include "sapi_rtc.h"
#include "consola.h"
#include "semaforo.h"


#define BUFF_IN_LENGHT 100
#define BUFF_OUT_LENGHT 200

typedef enum
{
    principal = 0,
    next,
    previous,
    new,
    modify,
    list,
    full_list,
    mode_manual,
    mode_alarm,
    set_clock
} menus_1;

typedef struct
{
    menus_1 nivel1;
    uint32_t nivel2;
    uint32_t nivel3;
}estado_menu_t;

/*-------------------Local variables-------------------*/
char buffer_in[BUFF_IN_LENGHT];
char buffer_out[BUFF_OUT_LENGHT];
uint32_t indice_in = 0;

estado_menu_t estado = {0,0};
secuencia_t* sec;

/*-------------------Local prototypes -----------------*/
#define CLEAR_ESTADO() estado.nivel1 = 0 ; estado.nivel2 = 0; estado.nivel3 = 0
bool_t read_console(void);
void send_message(char* ptr);
void procesar_comando(void);

void print_help(void);
void next_cmd(void);
void previous_cmd(void);
void new_cmd(void);
uint32_t recupera_luces(void);

void list_cmd(uint8_t full);

void mode_cmd(modo_sem m);

void set_clock_cmd(void);

void print_hora(void);


/*-------------------Local functions-------------------*/
bool_t read_console()
{
    while( uartReadByte(UART_USB , (uint8_t*) buffer_in+indice_in))
    {
        indice_in++;
        //si el buffer desborda lo reinicio
        if(BUFF_IN_LENGHT == indice_in)
        {
            memset(buffer_in, 0 , sizeof(buffer_in));
            indice_in = 0;
        }

        //detecto find de un mensaje
        if(0x0A == (uint8_t) buffer_in[indice_in-1])
        {
            return true;
        }

    }

    return false;
}

void send_message(char* ptr)
{
    uartWriteString( UART_USB, ">>" );
    uartWriteString( UART_USB, ptr );
    uartWriteString( UART_USB, "\r\n" );
}

void procesar_comando()
{
    buffer_in[indice_in-1] = 0;
    buffer_in[indice_in-2] = 0;

    if(estado.nivel1 == principal)
    {
        if(0 == strcmp("help",buffer_in))
            print_help();
        if(0 == strcmp("next",buffer_in))
            estado.nivel1 = next;
        if(0 == strcmp("new",buffer_in))
            estado.nivel1 = new;
        if(0 == strcmp("list",buffer_in))
            estado.nivel1 = list;
        if(0 == strcmp("full list",buffer_in))
            estado.nivel1 = full_list;
        if(0 == strcmp("alarm on",buffer_in))
            estado.nivel1 = mode_alarm;
        if(0 == strcmp("alarm off",buffer_in))
            estado.nivel1 = mode_manual;
        if(0 == strcmp("set clock",buffer_in))
            estado.nivel1 = set_clock;
    }
    switch(estado.nivel1)
    {
        case next:
            next_cmd();
            break;
        case previous:
            previous_cmd();
            break;
        case new:
            new_cmd();
            break;
        case modify:
            break;
        case list:
            list_cmd(0);
            break;
        case full_list:
            list_cmd(1);
            break;
        case mode_alarm:
            mode_cmd(reloj);
            break;
        case mode_manual:
            mode_cmd(manual);
            break;
        case set_clock:
            set_clock_cmd();
            break;
        default:

            break;
    }



    memset(buffer_in, 0 , sizeof(buffer_in));
    indice_in = 0;
}

void print_help()
{
    print_hora();

    send_message("Comandos:" );
    send_message("   next: cambia a la proxima secuencia" );
    send_message("   previous: cambia a la secuencia anterior" );
    send_message("   new: crea una nueva secuencia" );
    send_message("   modify: reemplaza una secuencia por otra nueva" );
    send_message("   list: imprime las secuencias existentes" );
    send_message("   full list: imprime el detalle de cada secuencia" );
    send_message("   alarm off: pasa a modo manual" );
    send_message("   alarm on: activa modo alarma" );
    send_message("   set clock: poner en hora el equipo" );

    //send_message("next: proxima secuencia" );

}

void next_cmd()
{
    print_hora();


    if(avanzar_secuencia())
    {
        sec = get_secuencia_actual();
        send_message( sec->nombre );
    }
    else
    {
        send_message("comando ivalido, pasar a modo manual" );
    }

    CLEAR_ESTADO();
}

void previous_cmd()
{
    print_hora();

    if(retroceder_secuencia())
    {
        sec = get_secuencia_actual();
        send_message( sec->nombre );
    }
    else
    {
        send_message("comando ivalido, pasar a modo manual" );
    }

    CLEAR_ESTADO();
}

void new_cmd ()
{
    static uint32_t valor = 0;
    uint32_t aux = 0;

    print_hora();

    switch(estado.nivel2)
    {
        case 0:
            send_message("Ingrese cantidad de estados");
            estado.nivel2 = 1;
            valor = 0;
            break;
        case 1:
            valor = atoi(buffer_in);
            if(valor)
            {
                estado.nivel2 = 2;
                send_message("Ingrese nombre de la secuencia");
            }
            else
            {
                send_message("ERROR DE SINTAXIS");
                CLEAR_ESTADO();
            }
            break;
        case 2:
            sec = agregar_secuencia((uint8_t) valor, buffer_in);
            if(NULL == sec)
            {
                send_message("ERROR AL CREAR SECUENCIA");
                CLEAR_ESTADO();
            }
            else
            {
                send_message("secuencia creada:");
                send_message(sec->nombre);
                estado.nivel2 = 3;
                estado.nivel3 = 0;
                sprintf(buffer_out, "Estado %02u - Ingrese luces \r\n r-> rojo \r\n v->verde \r\n a->amarillo \r\n Ejemplo: ra prendera rojo y amarillo", estado.nivel3 + 1);
                send_message(buffer_out);
            }
            break;
        case 3:
            valor = recupera_luces();
            sprintf(buffer_out, "Estado %02u - Ingrese tiempo en milisegundos", estado.nivel3 + 1);
            send_message(buffer_out);
            estado.nivel2 = 4;
            break;
        case 4:
            aux = atoi(buffer_in);
            if(aux)
            {
                editar_estado(sec, (uint8_t) estado.nivel3,  valor ,aux);
                estado.nivel3++;
                if(estado.nivel3 >= sec->lenght)
                {
                    send_message("Desea activar hora de alarma? s/n");
                    estado.nivel2 = 5;
                    estado.nivel3 = 0;
                }
                else
                {
                    sprintf(buffer_out, "Estado %02u - Ingrese luces \r\n r-> rojo \r\n v->verde \r\n a->amarillo \r\n Ejemplo: ra prendera rojo y amarillo", estado.nivel3 + 1);
                    send_message(buffer_out);
                    estado.nivel2 = 3;
                }

            }
            else
            {
                send_message("ERROR DE SINTAXIS");
            }
            break;
        case 5:
            if(NULL != strchr(buffer_in, 'n'))
            {
                sec->alarma.on = false;
                CLEAR_ESTADO();
            }
            else if(NULL != strchr(buffer_in, 's'))
            {
                sec->alarma.on = true;
                estado.nivel2 = 6;
                send_message("Ingrese hora de inicio");
            }
            else
            {
                send_message("ERROR DE SINTAXIS");
            }
            break;
        case 6:
            valor = atoi(buffer_in);
            if(valor >= 0 & valor < 24)
            {
                estado.nivel2 = 7;
                send_message("Ingrese minutos de inicio");
            }
            else
            {
                send_message("ERROR DE SINTAXIS");
                CLEAR_ESTADO();
            }
            break;

        case 7:
            aux = atoi(buffer_in);
            if(valor >= 0 & valor < 60)
            {
                sec->alarma.hora = (uint8_t) valor;
                sec->alarma.minuto = (uint8_t) aux;

                CLEAR_ESTADO();
            }
            else
            {
                send_message("ERROR DE SINTAXIS");
            }
            break;
    }
}

uint32_t recupera_luces()
{
    uint32_t luces = 0;
    if(NULL != strchr(buffer_in, 'r'))
        luces |= LUZ_ROJ;
    if(NULL != strchr(buffer_in, 'v'))
        luces |= LUZ_VER;
    if(NULL != strchr(buffer_in, 'a'))
        luces |= LUZ_AMA;

    return luces;
}

void list_cmd(uint8_t full)
{
    uint32_t i,j;
    uint32_t last = get_last();

    print_hora();

    for(i = 0 ; i < last ; i++)
    {
        sec = get_secuencia(i);
        send_message( sec->nombre );
        if(full)
        {
            for(j = 0 ; j < sec->lenght ; j++)
            {
                sprintf(buffer_out,"estado %02u \n\r   Tiempo: %u",j,sec->estado[j].pausa);

                strcat(buffer_out,"\n\r   Luces: ");
                if(sec->estado[j].luces & LUZ_ROJ)
                    strcat(buffer_out,"ROJO ");
                if(sec->estado[j].luces & LUZ_VER)
                    strcat(buffer_out,"VERDE ");
                if(sec->estado[j].luces & LUZ_AMA)
                    strcat(buffer_out,"AMARILLO ");

                send_message(buffer_out);
            }
        }
    }

    CLEAR_ESTADO();
}

void mode_cmd(modo_sem m)
{
    print_hora();

    set_modo_sem(m);

    if(manual == m)
        send_message("paso a modo manual" );
    else
        send_message("paso a modo alarma" );

    CLEAR_ESTADO();
}

void set_clock_cmd ()
{
    rtc_t rtc;
    static uint32_t valor = 0;
    uint32_t aux = 0;

    if(!rtcRead( &rtc ))
    {
        send_message("Error reloj no Iniciado");
        CLEAR_ESTADO();
        return;
    }



    switch(estado.nivel2)
    {
        case 0:
            send_message("Ingrese hora");
            estado.nivel2 = 1;
            valor = 0;
            break;
        case 1:
            valor = atoi(buffer_in);
            if(valor >= 0 & valor < 24)
            {
                estado.nivel2 = 2;
                send_message("Ingrese minutos");
            }
            else
            {
                send_message("ERROR DE SINTAXIS");
                CLEAR_ESTADO();
            }
            break;

        case 2:
            aux = atoi(buffer_in);
            if(valor >= 0 & valor < 60)
            {
                rtc.hour = (uint8_t) valor;
                rtc.min = (uint8_t) aux;

                rtcWrite( &rtc );
                print_hora();
            }
            else
            {
                send_message("ERROR DE SINTAXIS");
            }
            CLEAR_ESTADO();
            break;
    }


}

void print_hora()
{
    rtc_t rtc;
    if(rtcRead(&rtc ))
    {
        sprintf(buffer_out, "hora: %02u:%02u", rtc.hour,rtc.min);
        send_message(buffer_out);
    }
}
/*-------------------Global funtions ------------------*/
void consola_init()
{
    uartConfig(UART_USB, 115200);
    memset(buffer_in, 0 , sizeof(buffer_in));
    memset(buffer_out, 0 , sizeof(buffer_out));
    send_message( "Sistema iniciado" );
}

void antender_consola()
{
    if(read_console())
    {
        procesar_comando();
    }
}

