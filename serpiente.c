#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include <sys/mman.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include "funciones.h"
#include "serpiente.h"

int main()
{		
	//Se crea el semáforo.
	sem_t *sem=NULL;
	sem=sem_open("snaforo",O_CREAT,0600,1);
	if(sem == SEM_FAILED)
		printf("Error al crear el semaforo.\n");
	else{
     		
    	int mqid;
    	key_t clave=ftok(".",'u');
		struct msqbuf segmento;

   		//Se crea la cola de mensajes que contendrá los distintos segmentos de la serpiente.
   		if((mqid=msgget(clave,IPC_CREAT|0600))==-1 )
				printf("Error al abrir la cola\n"); 
		
		else{
			//Y ahora se crea o abre el fichero que contiene las puntuaciones:
			int fich;
			char * inicio;
			int i=0;
			char * sc;
			struct stat bstat;
			
			if((fich=open("./score.txt",O_CREAT|O_RDWR))<0)
				printf("Error al abrir el fichero de puntuaciones.\n");
			else if(fstat(fich,&bstat)<0)
				printf("Error con el bstat\n");
			else if(ftruncate(fich,11*MAX_JUG*sizeof(char))<0)
				printf("Error al truncar\n");
			else if((inicio=(char *)mmap((caddr_t)0,11*MAX_JUG*sizeof(char),PROT_READ|PROT_WRITE,MAP_SHARED,fich,0))==MAP_FAILED)
				printf("Error al mapear\n");
			else{
                system("chmod 777 score.txt");
				close(fich);
				sc=inicio;
				
				//Se crea la tabla que contendrá las puntuaciones, inicializadas a -1.
				struct jugadores jj[MAX_JUG];
				for(i=0;i<MAX_JUG;i++)
					jj[i].puntos=-1;
				
				i=0;
				//Se cargan los datos guardados en el fichero en la tabla de jugadores jj.
				while(*sc!='\0' && i<MAX_JUG){
					sscanf(sc,"%c%c%c=%d&",&jj[i].nombre[0],&jj[i].nombre[1],&jj[i].nombre[2],&jj[i].puntos);
					jj[i].nombre[3]='\0';
					while(*sc++!='&');
					i++;
				}
				
			//Ahora se empieza a preparar el juego.

			int dir[2];					//dir[0] es la dirección actual de la serpiente, dir[1] la próxima. 
			int c[2];					//Coordenadas de la cabeza.
			char mapa[FILAS][COLUMNAS];	//El tablero de juego.
			int j;						//Contador.
			int Puntos;					//Número de puntos conseguidos.
			int GAMEOVER;	 			//¿Ha chocado?
			int p;						//¿Ha cogido un punto en la última ronda? 
			unsigned int t=200000;		//Tiempo, en nanosegundos, entre movimientos de la serpiente.
			int lvl=0;					//Indicador de la dificultad seleccionada. Si es 0 es progresivo.
			pthread_t thid;				//ID del hilo ligero.
			char opcion='h';			//Variable para el menú.
			int jugar=1;				//Valdrá 0 cuando no se quiera echar otra partida.
			
			//Se imprime la presentación del juego y el menú de inicio.
			printf( ANSI_COLOR_GREEN"\n\
                    /^\\/^\\\\\n\
                  _|__|  O|\\\n\
         \\/     /~     \\_/ \\\\\n\
          \\____|__________/  \\\n\
                 \\_______      \\\n\
                         `\\     \\                 \\\n\
                           |     |                  \\\n\
                          /      /                    \\\n\
                         /     /                       \\\\\n\
                       /      /                         \\ \\\n\
                      /     /                            \\  \\\n\
                    /     /             _----_            \\   \\\n\
                   /     /           _-~      ~-_         |   |\n\
                  (      (        _-~    _--_    ~-_     _/   |\n\
                   \\      ~-____-~    _-~    ~-_    ~-_-~    /\n\
                     ~-_           _-~          ~-_       _-~\n\
                        ~--______-~                ~-___-~\n"ANSI_COLOR_RESET );
			printf("\
			SNAKE, EL VIDEOJUEGO DEL AYER\n\n");
			sleep(1);
			printf("\
            Alimenta a la serpiente dirigiéndola hacia los frutos repartidos por el tablero.\n\
            Usa WASD para elegir la dirección que seguirá Snake.\n\
            Cada fruto da cien puntos. Cada mil aumentará su velocidad.\n\
            ¡Alimenta la serpiente, consigue más puntos y sé la envidia de tus amigos!\n\n\n");
          
			while(opcion!='q'){
				printf("\
            a) Jugar\n\
            b) Ver puntuaciones\n\
            c) Seleccionar dificultad\n\
            q) Salir\n");
			
				opcion=mover();
				switch (opcion){
				case 'q':
					jugar=0;
					break;
				case 'b':
					limpiar();
					if(puntuacion(jj,-1)==-1)
						printf("\tLamentablemente, aún no hay registros.\n\n");
					mover();
					limpiar();	
					break;
				case 'c':
					limpiar();
					printf("El juego dispone de nueve dificultades, numeradas del 1 al 9. El número que selecciones corresponderá a la velocidad de la serpiente.\n Si marcas el 0 empezará a moverse a velocidad 5 y aumentará la velocidad cada 1000 puntos. Esta es la opción predeterminada.\n");
					opcion=getchar();
					while(getchar()!='\n');
					if(atoi(&opcion) < 10 || atoi(&opcion) > 0){
						lvl=atoi(&opcion);
						t=400000 - lvl*40000;
					}else{
						t=200000;
						lvl=0;
					}
					limpiar();
					opcion='c';
					break;
				case 'a':
					opcion='q';
				default:	
					limpiar();
					printf("\n\tOpción no válida. Por favor, elija otra:\n\n");
					break;
				}
			}
            
           	while(jugar){
  
				limpiar();
				
				//Se dibuja el tablero.
				for(i=0;i<FILAS;i++)
					for(j=0;j<COLUMNAS;j++){
						if(i==0 || j==0 || i==FILAS-1 || j==COLUMNAS-1)
							mapa[i][j]=MURO;
						else
						mapa[i][j]=VACIO;
				}
						
				//Se dibuja la serpiente.
				mapa[6][1]=CUERPO;
				mapa[6][2]=CUERPO;
				mapa[6][3]=CUERPO;
				mapa[6][4]=CABEZA;
			
				//El punto va a estar ahí.
				premio(mapa);
				
				//Se vacía la cola:
				while(msgrcv(mqid,&segmento,2*sizeof(int),1,IPC_NOWAIT) != -1);

				//Se envían los cuatro primeros segmentos a la cola, empezando por el último:
				segmento.s[0]=6;
				segmento.mtype=1;
			
				segmento.s[1]=1;
				msgsnd(mqid, &segmento, 2*sizeof(int), IPC_NOWAIT);
				segmento.s[1]=2;
				msgsnd(mqid, &segmento, 2*sizeof(int), IPC_NOWAIT);
				segmento.s[1]=3;
				msgsnd(mqid, &segmento, 2*sizeof(int), IPC_NOWAIT);
				segmento.s[1]=4;
				msgsnd(mqid, &segmento, 2*sizeof(int), IPC_NOWAIT);
				
				//Se inicia el localizador de la cabeza de la serpiente y otras variables.		
				c[0]=6;
				c[1]=4;
				p=0;
				Puntos=0;
				GAMEOVER=0;
				
				//La serpiente inicialmente se dirige hacia la derecha.
				dir[0]=DER;	
				dir[1]=DER;	
	
				imprimir_mapa(mapa);
			
				//Se inicializa el controlador, en un hilo ligero.
				pthread_create(&thid, NULL, controlador, dir); 

				printf("Listos... ");
				sleep(2);
	
				//En cada ejecución del bucle se mueve la serpiente. Para ellos se cambia el dibujo del mapa,
				//los segmentos en la cola de mensajes y las coordenadas de la cabeza, así como el número de
				//segmentos, la posición del fruto y el número de puntos, si fuese necesario.
				while(!GAMEOVER){
					limpiar();
					//Se usa el semáforo para que la dirección no cambie mientras se analiza.
					sem_wait(sem);
			
					//El último segmento desaparece del mapa, ya que se ha movido.
					//Si hay que aumentar el tamaño, se salta esta parte.
					if(p==0){
						if(msgrcv(mqid,&segmento,2*sizeof(int), 1,IPC_NOWAIT)==-1)
							printf("Error al recibir de cola.\n");
						mapa[segmento.s[0]][segmento.s[1]]=VACIO;
					}
				
					p=0;	
					
					//Se actualiza el sentido:
					if(dir[1]!=PAUSA)
						dir[0]=dir[1];
	
					//La cabeza se moverá en una dirección distinta según hacia donde se mueva.
					if(dir[0]==DER){
			
						//¿Se va a chocar con un muro o su cola?
						if(mapa[c[0]][c[1]+1]==MURO || mapa[c[0]][c[1]+1]==CUERPO){
							GAMEOVER=1;
							dir[0]=0;
						}
						else{ 
							//¿Hay un punto delante?
							if(mapa[c[0]][c[1]+1]==FRUTO){
								Puntos+=100; 
								p=1;
							}				
							//Se dibuja el movimiento en el mapa y se cambia la coordenada de la cabeza.
							mapa[c[0]][c[1]]=CUERPO;
							c[1]+=1;
							mapa[c[0]][c[1]]=CABEZA;
							segmento.s[0]=c[0];
							segmento.s[1]=c[1];
							msgsnd(mqid, &segmento, 2*sizeof(int),IPC_NOWAIT);
						}	
					
					}else if(dir[0]==IZQ){
				
						//¿Se va a chocar con un muro o su cola?
						if(mapa[c[0]][c[1]-1]==MURO || mapa[c[0]][c[1]-1]==CUERPO){
							GAMEOVER=1;
							dir[0]=0;
						}
						else{ 
							//¿Hay un punto delante?
							if(mapa[c[0]][c[1]-1]==FRUTO){
								Puntos+=100; 
								p=1;
							}				
							//Se dibuja el movimiento en el mapa y se cambia la coordenada de la cabeza.
							mapa[c[0]][c[1]]=CUERPO;
							c[1]-=1;
							mapa[c[0]][c[1]]=CABEZA;
							segmento.s[0]=c[0];
							segmento.s[1]=c[1];
							msgsnd(mqid, &segmento, 2*sizeof(int),IPC_NOWAIT);
						}		
				
					}else if(dir[0]==ABA){
				
						//¿Se va a chocar con un muro o su cola?
						if(mapa[c[0]+1][c[1]]==MURO || mapa[c[0]+1][c[1]]==CUERPO){
							GAMEOVER=1;
							dir[0]=0;
						}
						else{ 
							//¿Hay un punto delante?
							if(mapa[c[0]+1][c[1]]==FRUTO){
								Puntos+=100; 
								p=1;
							}				
							//Se dibuja el movimiento en el mapa y se cambia la coordenada de la cabeza.
							mapa[c[0]][c[1]]=CUERPO;
							c[0]+=1;
							mapa[c[0]][c[1]]=CABEZA;
							segmento.s[0]=c[0];
							segmento.s[1]=c[1];
							msgsnd(mqid, &segmento, 2*sizeof(int),IPC_NOWAIT);
						}	
				
					}else if(dir[0]==ARR){
				
						//¿Se va a chocar con un muro o su cola?
						if(mapa[c[0]-1][c[1]]==MURO || mapa[c[0]-1][c[1]]==CUERPO){
							GAMEOVER=1;
							dir[0]=0;
						}
						else{ 
							//¿Hay un punto delante?
							if(mapa[c[0]-1][c[1]]==FRUTO){
								Puntos+=100;
								p=1;
							}				
							//Se dibuja el movimiento en el mapa y se cambia la coordenada de la cabeza.
							mapa[c[0]][c[1]]=CUERPO;
							c[0]-=1;
							mapa[c[0]][c[1]]=CABEZA;
							segmento.s[0]=c[0];
							segmento.s[1]=c[1];
							msgsnd(mqid, &segmento, 2*sizeof(int),IPC_NOWAIT);
						}
					}
				
					//Si se ha comido un fruto, se pone otro en el mapa.
					if(p==1)
						premio(mapa);
		
					//Se imprime el nuevo mapa y la puntuación.	
					if (!GAMEOVER){
						imprimir_mapa(mapa);
						printf(ANSI_COLOR_BLUE "Puntuación: %d\n" ANSI_COLOR_RESET, Puntos);
					}
						
					//Cada mil puntos se aumenta la velocidad de la serpiente.
					if(p && !lvl && !Puntos%1000)
						t*=0.8;
			
					//Ahora la dirección puede cambiar.
					sem_post(sem);
					
					//Si se ha pulsado espacio el juego se bloqueará por espera activa.
					if(dir[1]==PAUSA){
						printf("PAUSA. Pulsa cualquier tecla de dirección para continuar.\n");
						while(dir[1]==PAUSA);
					}
	
					//Tiempo de espera hasta el siguiente movimiento.
					usleep(t);
				}	
			
				//Se imprime la puntuación conseguida y un mensaje personalizado que variará según el número de puntos conseguidos.
				printf("\n\n\t ________________________/ O  \\___/\n\t<_____________________________/   \\\n\n");
				printf("\t¡Gracias por jugar!\n\n\tHas comido %d frutos, consiguiendo un total de %d puntos.\n", Puntos/100,Puntos);
			
			
				if( Puntos == 0)
					printf("\tLo que cuenta es la intención.\n");
				else if(Puntos <= 500)
					printf("\tNo está mal.\n");
				else if(Puntos <= 1000)
					printf("\tEstoy seguro de que Snake disfrutó el tentempié.\n");
				else if(Puntos <= 2000)
					printf("\tEstos frutos serán suficientes para pasar el invierno.\n");
				else if(Puntos <= 5000)
					printf("\tEstás convirtiéndote en un experto en esto. No debe de haber quedado un solo fruto en kilómetros a la redonda.\n");
				else if(Puntos < 9900)
					printf("\t¡Genial!\n");
				else if(Puntos == 9900)
					printf("\tUgh. Estuvo muy cerca.\n");
				else
					printf("\tGracias a ti ninguna serpiente del mundo volverá a pasar hambre.\n");
		
				printf("\n\n");
				
				//Cuando se pulse una tecla más el hilo terminará.
				pthread_join(thid,NULL);
				
				//Se imprime la puntuación.
				puntuacion(jj,Puntos);
			
				printf("\t¿Jugar otra vez? (y/n)  ");
				opcion=mover();
				if(!(opcion == 'y' || opcion == 'Y' || opcion == 's' || opcion == 'S'))
					jugar=0;
				else
					limpiar();	
				
			}
	
			limpiar();
			
			sc=inicio;
			i=0;
			
			//Se guarda la puntuación obtenida en la última partida en el fichero score.txt.
			while(i<MAX_JUG && jj[i].puntos!=-1){
				sprintf(sc,"%s=%d&",jj[i].nombre,jj[i].puntos);
				while(*sc++!='&');
				i++;
			}
			
			if(munmap(inicio,11*MAX_JUG*sizeof(char)))
				printf("Error al desmapear\n");     		
     		}
	
			if(msgctl(mqid,IPC_RMID,NULL) == -1)
				printf("Error al borrar la cola\n");
			
		}
		
		if(sem_close(sem)<0)
			printf("Error al cerrar el semaforo\n");
		if(sem_unlink("snaforo")<0)
			printf("Error al borrar el semaforo\n");
	}
	return 0;
}
