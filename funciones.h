#ifndef FUNCIONES_H
#define FUNCIONES_H

#include "serpiente.h"

char mover(void);
void limpiar(void);
void imprimir_mapa(char map[][COLUMNAS]);
int puntuacion(struct jugadores * j, int N);
void premio(char m[][COLUMNAS]);
void * controlador(void * DIR);

#endif
