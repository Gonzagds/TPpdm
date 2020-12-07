/*=============================================================================
 * Author: Gonzalo Sanchez <na.gonzalo@gmail.com>
 * Date: 2020/11/25
 * Version: 1
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include "semaforo_consola.h"

#include "sapi.h"

#include "semaforo.h"

#include "teclado.h"

#include "consola.h"

/*=====[Definition macros of private constants]==============================*/

#define NORMAL          0
#define ALARMA          1
#define DESCONECTADO    2

/*=====[Definitions of private global variables]=============================*/

// weak


/*=====[Main function, program entry point after power on or reset]==========*/

int main( void )
{
    // Inicializar y configurar la plataforma
    boardConfig();

    init_semaforo();
    consola_init();

    while( TRUE )
    {
        antender_consola();

        procesa_teclado();

        ejecutar_secuencia();
    }

    // NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa se ejecuta
    // directamenteno sobre un microcontroladore y no es llamado por ningun
    // Sistema Operativo, como en el caso de un programa para PC.
    return 0;
}

