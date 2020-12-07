/*
 * teclado.h
 *
 *  Created on: Nov 18, 2020
 *      Author: gonzalo
 */

#ifndef MYPROGRAMS_SEMAFORO_INC_TECLADO_H_
#define MYPROGRAMS_SEMAFORO_INC_TECLADO_H_

typedef struct
{
    bool_t tec_next   :1;
    bool_t tec_prev :1;
    bool_t tec_alarm :1;
    bool_t tec_manu:1;
}teclado_t;

bool_t leer_teclado(teclado_t* teclado);

#endif /* MYPROGRAMS_SEMAFORO_INC_TECLADO_H_ */
