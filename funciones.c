#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <semaphore.h>
#include <termios.h>
#include <sys/wait.h>
#include "serpiente.h"
#include "funciones.h"

/*Función encargada de leer de la terminal sin pulsar enter.*/        
char mover(){
    struct termios oldt, newt;
    char ch;
    tcgetattr( STDIN_FILENO, &oldt );
    newt = oldt;
    newt.c_lflag &= ~( ICANON | ECHO );
    tcsetattr( STDIN_FILENO, TCSANOW, &newt );
    ch = getchar();
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
    return ch;
}

/*Usando fork y execlp se limpia la pantalla, simulando el movimiento de la serpiente.*/
void limpiar(void){
	
	int estado; 
	pid_t pid; 
	pid = fork(); 
 
	switch(pid) 
	{ 
		case -1: /* error del fork() */ 
			perror("fork"); 
			break; 
		case 0: /* proceso hijo */ 
			execlp("clear","clear","-1",NULL);
			break; 
		default: /* padre */ 
			wait(&estado); 
	}
}

/*Función encargada de imprimir el mapa de juego.*/
void imprimir_mapa(char m[][COLUMNAS]){
	int i,j;
	for(i=0;i<FILAS;i++){
		for(j=0;j<COLUMNAS;j++){
		if(m[i][j]==CABEZA||m[i][j]==CUERPO)
			printf(ANSI_COLOR_GREEN "%c " ANSI_COLOR_RESET ,m[i][j]);
		else if(m[i][j]==FRUTO)
			printf(ANSI_COLOR_RED "%c " ANSI_COLOR_RESET ,m[i][j]);
		else
		 printf(ANSI_COLOR_YELLOW "%c " ANSI_COLOR_RESET ,m[i][j]);
		}
		printf("\n");
	}
}

/*Esta función recibe la tabla con las distintas puntuaciones y un entero 
 * con el número de puntos recién conseguidos. Hace dos cosas:
 * - Si el número N es negativo, imprime la tabla de puntuaciones.
 * - Si es un número positivo, también comprueba si ha superado algun 
 * record anterior. En caso de que sea así pedirá un apodo por teclado,
 * lo sustituirá en la tabla y volverá a imprimir.
 * Devuelve 1 si la tabla de puntuaciones se ha modificado, 
 * 0 si no es así y -1 si la tabla de puntuaciones estaba vacía.*/
int puntuacion(struct jugadores * j, int N)
{
	int i;
	int dev=0;
	
	//Se evalúa la tabla para ver si se ha batido algún record.							
	for(i=MAX_JUG-1;i>0 && N>=0;i--)
		if(j[i-1].puntos!=-1) {
			if(N > j[i-1].puntos){
				j[i].puntos=j[i-1].puntos;
				strcpy(j[i].nombre,j[i-1].nombre);
			}else if(N>j[i].puntos){
				printf("\tIntroduce tu nombre (3 caracteres):\n\n\t");
				scanf("%3s",j[i].nombre);
				while(getchar()!='\n');
				j[i].puntos=N;		
				N=-1;
				dev=1;
			}else{
				N=-1;
				dev=0;
			}
		}
		
	if(N>j[0].puntos){
		printf("\tIntroduce tu nombre (3 caracteres):\n\n\t");
		scanf("%3s",j[0].nombre);
		while(getchar()!='\n');
		j[0].puntos=N;		
		N=-1;
		dev=1;
	}
	
	//Se imprime.
	if(j[0].puntos>=0){
		printf(ANSI_COLOR_BLUE"\t\t--PUNTUACION--\n"ANSI_COLOR_RESET);
		for(i=0;i<MAX_JUG;i++){
			if(j[i].puntos!=-1)
				printf("\t\t%s\t%d\n",j[i].nombre,j[i].puntos);
			else
				i=MAX_JUG;
		}
		printf("\n\n");
	}
	else
		dev=-1;
					
	return dev;					
}


/*Cuando la serpiente come una fruta esta función pone otra en un lugar aleatorio*/
void premio(char m[][COLUMNAS]){
	int flag=1;
	srand(time(NULL));
	while(flag){
		int i=rand()%(FILAS-1)+1;
		int j=rand()%(COLUMNAS-1)+1;
		if(m[i][j]==VACIO){
			flag=0;
            m[i][j]=FRUTO;
        }
	}
}

/* Esta función se abrirá con un semaforo. Estará permanentemente esperando 
 * a que se indique por teclado el cambio de dirección de la serpiente, con WASD.
 * Terminará cuando sea GAMEOVER y se pase un caracter más por teclado.*/
void * controlador(void * DIR){
	int * dir=(int *)DIR;
	char d;
	
	sem_t *semID=sem_open("snaforo", 0);
	if(semID==NULL)
		printf("Error abriendo semáforo\n");
	else{
	
		//Escaneo continuo del teclado.
		while(dir[0]!=0){		
			d=mover();
			
			sem_wait(semID);
	
			if((d=='W'||d=='w') && dir[0]!=ABA)
				dir[1]=ARR;
			else if((d=='S'||d=='s') && dir[0]!=ARR)
				dir[1]=ABA;
			else if((d=='A'||d=='a') && dir[0]!=DER)
				dir[1]=IZQ;
			else if((d=='D'||d=='d') && dir[0]!=IZQ)
				dir[1]=DER;
			else if(d==' ')
				dir[1]=PAUSA;
				
			sem_post(semID);

		}
		if (sem_close(semID)==-1)
			printf("Error cerrando semáforo\n");
	}
	return 0;
}
