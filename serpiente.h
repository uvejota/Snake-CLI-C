#ifndef SERPIENTE_H
#define SERPIENTE_H

//Estos códigos serviran para colorear la salida por pantalla.
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

//Tamaño del tablero.
#define FILAS 20
#define COLUMNAS 20

//Número máximo de jugadores que se guardarán en las puntuaciones.
#define MAX_JUG 3

//Códigos para la dirección de la serpiente. 
#define IZQ 1
#define DER 2
#define ARR 3
#define ABA 4

//Código para la pausa.
#define PAUSA 5

//Caracteres usados en el juego:
#define CABEZA 'o'
#define CUERPO '+'
#define MURO '#'
#define VACIO ' '
#define FRUTO '*'


//struct necesario para la cola de mensajes.
struct msqbuf{
	long mtype;
	int s[2];
};

//struct para las puntuaciones.
struct jugadores{
	char nombre[4];
	int puntos;
};

#endif
