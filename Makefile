todo: serpiente borra

serpiente: serpiente.o funciones.o
	gcc -W -Wall -o serpiente serpiente.o funciones.o -lpthread
serpiente.o: serpiente.c serpiente.h funciones.h
	gcc -W -Wall -c serpiente.c 
funciones.o: funciones.c funciones.h serpiente.h
	gcc -W -Wall -c funciones.c 
borra:
	rm *.o