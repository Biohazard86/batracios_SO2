// CURSO 2020/2021
// SISTEMAS OPERATIVOS II
// GRUPO: G01
// DAVID BARRIOS PORTALES
// Victor Vacas Amigo
//      ------------------------------
//      ||       BATRACIOS          ||
//      ------------------------------ 
// github.com/Biohazard86/batracios_SO2


// Para poder compilar el programa, hay que hacerlo en el modo de 32bits
// Se compila con:
//        gcc batracios.c libbatracios.a -lm -o batracios 

// Si se quiere compilar en modo de 64 bits se debe hacer lo siguiente:

/*
    1 - Con la orden dpkg --print-architecture, comprobad que realmente tenéis un Linux de 64 bits. Debe aparecer, amd64.
    2 - Meted ahora la orden dpkg --print-foreign-architectures. Si entre la salida no aparece i386, debéis teclear: sudo dpkg --add-architecture i386
    3 - Ahora necesitáis tener las bibliotecas de 32 bits también instaladas. Lo lográis con: sudo apt-get install g++-multilib
    4 - Finalmente, podéis hacer una prueba para ver si todo funciona. Compilad vuestra práctica incluyendo la biblioteca de Linux de 32 bits y la 
        opción -m32 en la línea de compilación del gcc: gcc -m32...
    5 - Si la fase anterior no dio ningún error y os generó el ejecutable, probad a ejecutarlo. Si todo ha ido bien, debería ejecutarse sin problemas.
*/

// Para ejecutarlo: 
//            gcc batracios.c libbatracios.a -lm -m32 -o batracios  


// -------------------------------------------------------------------------------------------------------
// LIBRERIAS USADAS
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>

// -------------------------------------------------------------------------------------------------------
// DEFINICIONES USADAS
#define TICS_DEFECTO 50				//Si el usuario no introduce el segundo parametro
#define MAX_TOTAL_PROCESOS 35		// El total de procesos permitidos
#define MAX_RANAS_MADRE 4			// El numero de ranas madre
#define MAX_RANAS_HIJAS 30			// El numero max de ranitas

// DEFINICIONES DE SEMAFOROS
#define MAIN_PANTALLA 1				// Semaf de la pantalla principal
#define RANA_MADRE_1 2				// De la primera rana madre
#define RANA_MADRE_2 3				// De la segunda rana madre
#define RANA_MADRE_3 4				// De la tercera rana madre
#define RANA_MADRE_4 5				// De la cuarta rana madre
#define SEMAF_RANITAS_NACIDAS 6		// Semaforo que controla las ranas nacidas
#define SEMAF_RANITAS_SALVADAS 7	// Ranas salvadas
#define SEMAF_RANITAS_MUERTAS 8		// Ranas perdidas
#define SEMAF_POSICIONES 9
#define DERECHA 0					// Definimos derecha, izquierda y arriba. Para mover los troncos y las ranas
#define IZQUIERDA 1
#define ARRIBA 2
#define RANA_CRUZADA 11				// Para comprobar si la rana ha cruzado

//----------------------------------------------------------------------------------------
//Prototipos de las funciones de la biblioteca a modo de lista 

int BATR_pausa(void);
int BATR_pausita(void);
int BATR_inicio(int ret,int semAforos, int lTroncos[],int lAguas[],int dirs[],int tCriar,char *zona);
int BATR_avance_troncos(int fila);
void BATR_descansar_criar(void);
int BATR_parto_ranas(int i,int *dx,int *dy);
int BATR_puedo_saltar(int x,int y,int direcciOn);
int BATR_explotar(int x,int y);
int BATR_avance_rana_ini(int x,int y);
int BATR_avance_rana(int *x,int *y,int direcciOn);
int BATR_avance_rana_fin(int x,int y);
int BATR_comprobar_estadIsticas(int r_nacidas, int r_salvadas, int r_perdidas);
int BATR_fin(void);

//WAIT Y SIGNAL PARA LOS SEM
int semaforo_signal( int semid, int indice);
int semaforo_signal( int semid, int indice);



// -------------------------------------------------------------------------------------------------------
// Estructura para las posiciones, con un X e Y
    struct posicion_struct {int x,y;};


// VARIABLES GLOBALES USADAS
    int id_semaforo,id_memoria;       		//id de los semaforos y memoria
    char *memoria;                  		//puntero a memoria compartida para la biblioteca
    char *finalizar;                		//puntero a memoria compartida para la variable finalizar
    struct posicion_struct *posiciones;    	//puntero a memoria compartida para las posiciones


// -------------------------------------------------------------------------------------------------------
// Funcion PRESENTACION
// Hace una pequenia presentacion del programa
// -----------------------------------
void presentacion(){
    fprintf(stderr,"\n");
    fprintf(stderr,"\n\t|----------------------------------------|");
    fprintf(stderr,"\n\t|               BATRACIOS                |");
	fprintf(stderr,"\n\t|         SISTEMAS OPERATIVOS II         |");
	fprintf(stderr,"\n\t|               2020/2021                |");
    fprintf(stderr,"\n\t|----------------------------------------|");
    fprintf(stderr,"\n\t|               GRUPO G01                |");
    fprintf(stderr,"\n\t|         David Barrios Portales         |");
    fprintf(stderr,"\n\t|           Victor Vacas Amigo           |");
    fprintf(stderr,"\n\t|----------------------------------------|");
    fprintf(stderr,"\n\t|  github.com/Biohazard86/batracios_SO2  |");
    fprintf(stderr,"\n\t|----------------------------------------|");
    fprintf(stderr,"\n");
    fprintf(stderr,"\n");
}
// FIN PRESENTACION -------------------------------------------------------------------------------------------------------


// -------------------------------------------------------------------------------------------------------
// Funcion EROR PARAMETROS
// Muestra como se usa el programa si se introducen mal los parametros
// -----------------------------------
void error_parametros(){
    fprintf(stderr,"\nNo se han introducido suficientes argumentos.");
    fprintf(stderr,"\nPara usar el programa son necesarios dos argumentos, el primero es obligatorio y el segundo es opcional");
    fprintf(stderr,"\nEl primer parametro es un numero comprendido entre 0 y 1000");
    fprintf(stderr,"\nEste valor indicara la equivalencia en milisegundos de tiempo real de un tic de reloj (La lentitud con la que el programa funciona)");
    fprintf(stderr,"\nEl segundo parametro es por defecto 50 y es la medida de tics que una rana madre necesita para descansar entre dos partos.");
    fprintf(stderr,"\nDebe ser un numero entero estrictamente mayor que 0. Si es 1 o superior, la practica funcionaratanto mas lenta cuanto mayor sea el parametro y no debe consumir CPU");
    fprintf(stderr,"\nEl programa esta preparado para recibir CRL+C desde el terminal y finalizar la ejecucion del programa\n");

}
// FIN ERROR_PARAMETROS -------------------------------------------------------------------------------------------------------


// -------------------------------------------------------------------------------------------------------
// Funcion ACABAR
// Funcion manejadora de la señal SIGINT
// -----------------------------------
void acabar(int s){
	*finalizar=1;
}
// -------------------------------------------------------------------------------------------------------


// -------------------------------------------------------------------------------------------------------
// Funcion SEMAFORO_WAIT	(Funciona, reutilizadas)
// Se le tiene que pasar el id del semaforo y un indice 
// -----------------------------------
int semaforo_wait( int semaforo_id, int indice){
	/// Estructura
	struct sembuf oper;

	oper.sem_num = indice; 	//Sera sobre el semaforo indicado como semaforo id
	oper.sem_op = -1;
	oper.sem_flg = 0; 

	return semop(semaforo_id, &oper, 1); //Realizamos la operacion 
}
// -------------------------------------------------------------------------------------------------------


// -------------------------------------------------------------------------------------------------------
// Funcion SEMAFORO_WAIT	(Funciona, reutilizadas)
// Se le tiene que pasar el id del semaforo y un indice 
// -----------------------------------
int semaforo_signal( int semaforo_id, int indice){
	// Estructura
	struct sembuf oper;

	oper.sem_num=indice; 	//Sera sobre el semaforo indicado como semaforo id
	oper.sem_op= +1; 		//se hace un wait
	oper.sem_flg=0;			//sin indicadores adicionales

	return semop(semaforo_id, &oper, 1); //se realiza una operación 
}
// -------------------------------------------------------------------------------------------------------




// ------------------------------------------------------------------------------------------------------
// Funcion ranita (Acabada... Espero...)
// Se encarga de los procesos hijo, las ranitas
// Se le pasa la posicion y el indice
// -----------------------------------
int ranita(int pos, int i){

	//HAcemos un primer salto hacia adelante siempre, fuera del bucle
	// 
	while(BATR_puedo_saltar(posiciones[pos].x,posiciones[pos].y,ARRIBA)!=0 && !*finalizar);

    // Comprobamos si se puede saltar hacia arriba y si es 0 significa que si.
	if (BATR_puedo_saltar(posiciones[pos].x,posiciones[pos].y,ARRIBA) == 0){
        /*
		Una vez la rana sabe que puede avanzar, llama a estas tres funciones. Los parámetros son de significado evidente.  
		No  obstante,  fijaos  en  que  la  segunda  función  recibe  la  posición  pasada  por  referencia,  de modo que, 
		una vez realizado el avance, las nuevas coordenadas aparecen en las variables pasadas. Esas mismas nuevas coordenadas, 
		se pasan a la última función
		*/
		BATR_avance_rana_ini(posiciones[pos].x,posiciones[pos].y);
		BATR_avance_rana(&posiciones[pos].x,&posiciones[pos].y,ARRIBA);
		BATR_avance_rana_fin(posiciones[pos].x,posiciones[pos].y);
	}

	// Hacemos un signal al sem de la primera posicion de la rana madre que corresponde
	semaforo_signal(id_semaforo,(i+2));

	//Bucle en el que se avanza constantemete hacia delante mientras finalizar no sea falso
	while(!*finalizar){

		// WAIT A LA MEMORIA COMPARTIDA DE LAS POSICIONES

		if(semaforo_wait(id_semaforo, SEMAF_POSICIONES)==-1) {
            continue;
        }

        // Comprobamos si la rana ha cruzado 
		if (posiciones[pos].y == RANA_CRUZADA){
		
			//si ha sido salvada se dejan libres la posicion y el hueco en pantalla y acaba
			semaforo_wait(id_semaforo, SEMAF_RANITAS_SALVADAS);
			posiciones[31].x++;	//Incrementamos
			semaforo_signal(id_semaforo,SEMAF_RANITAS_SALVADAS);

			posiciones[pos].x=-2;
			posiciones[pos].y=-2;
			semaforo_signal(id_semaforo, MAIN_PANTALLA);

			// SIGNAL A LA MEMORIA COMPARTIDA DE LAS POSICIONES
			semaforo_signal(id_semaforo, SEMAF_POSICIONES);

			return 0;
		}// END IF

		else if ( (posiciones[pos].x<=-1 && posiciones[pos].y!=-2) || posiciones[pos].x>=80) {
			//Comprobamos si no se ha salido alguna rana por un lado
			// En el caso de perder alguna, dejamos libre dicha posicion y hueco 
			
			semaforo_wait(id_semaforo, SEMAF_RANITAS_MUERTAS);
			posiciones[32].x++;
			semaforo_signal(id_semaforo,SEMAF_RANITAS_MUERTAS);

			posiciones[pos].x=-2;
			posiciones[pos].y=-2;
			semaforo_signal(id_semaforo, MAIN_PANTALLA);

			//SIGNAL A LA MEMORIA COMPARTIDA DE LAS POSICIONES
			semaforo_signal(id_semaforo, SEMAF_POSICIONES);

			return 0;
		}// END ELSEIF

		else if (BATR_puedo_saltar(posiciones[pos].x,posiciones[pos].y,ARRIBA)==0){
			//si no ha acabado por arriba o por los lados intenta avanzar avanzar

			/*
			Una vez la rana sabe que puede avanzar, llama a estas tres funciones. Los parámetros son de significado evidente.  
			No  obstante,  fijaos  en  que  la  segunda  función  recibe  la  posición  pasada  por  referencia,  de modo que, 
			una vez realizado el avance, las nuevas coordenadas aparecen en las variables pasadas. Esas mismas nuevas coordenadas, 
			se pasan a la última función
			*/

			BATR_avance_rana_ini(posiciones[pos].x,posiciones[pos].y);
			BATR_avance_rana(&posiciones[pos].x,&posiciones[pos].y,ARRIBA);
			BATR_avance_rana_fin(posiciones[pos].x,posiciones[pos].y);
		}
		
		// SIGNAL A LA MEMORIA COMPARTIDA DE LAS POSICIONES
		semaforo_signal(id_semaforo, SEMAF_POSICIONES);


	}//FIN while

	return 0;
}
// FIN RANITA ------------------------------------------------------------------------------------------------------





// -------------------------------------------------------------------------------------------------------
// Funcion RANA MADRE  (Acabada... Creo)
// Recibe un entero
// Es el codigo que se ejecuta para crear los procesos hijos, es decir, las ranitas 
// -----------------------------------
int codigo_rana_madre(int i){

	int x,y;        //coordenadas iniciales de las ranitas creadas
	int posicion;   //Una de las 30 posiciones duspoibles para las ranitas 
	int j,k;        //contadores parael bucle
    pid_t id_ranita; //variable para el fork, el id de cada ranita

	while(!*finalizar){  // Bucle infinito
		//Esperamos a que haya "hueco" para un nuevo proceso (ranita)
		if (semaforo_signal(id_semaforo,MAIN_PANTALLA)==-1){
            // Si hay hueco entonces continuamos
            continue;
        }
		
		//se hace un wait al semaforo que indica si la rana anterior ya se ha movido de la posicion inicial
		if (semaforo_signal(id_semaforo,(i+2))==-1){
            return 0;
        }

		//WAIT AL SEMAFORO DE POSICIONES
		if (semaforo_signal(id_semaforo, SEMAF_POSICIONES)==-1){
			continue;
		}

		//COMPROBAR ALGUNA POSICION DE LA MEMORIA COMPARTIDA QUE ESTE LIBRE
		for (j = 0; j <=29; ++j)
		{
			if (posiciones[j].x==-2)
			{
				posicion=j;
				break;
			}
		}


		//Se llama a la funcion para crear ranitas
		BATR_parto_ranas(i, &posiciones[posicion].x,&posiciones[posicion].y);

		//SIGNAL AL SEMAFORO DE POSICIONES
		semaforo_signal(id_semaforo, SEMAF_POSICIONES);

		//Incrementamos el contador de ranas nacidas
		semaforo_signal(id_semaforo,SEMAF_RANITAS_NACIDAS);
		posiciones[30].x++;		// Incrementamos
		semaforo_signal(id_semaforo,SEMAF_RANITAS_NACIDAS);


		//se crea un proceso para encargarse de la ranita a la cual se le indica la posicion y el numero de madre
		id_ranita=fork();
		// Comprobamos si se ha creado
		switch(id_ranita){
            // Si hay error entonces:
			case -1:
				perror("ERROR. Fork");
				return 1;
                // Retornamos un 1
			case 0: //Codigo del proceso hijo
                //
				return ranita(posicion,i);
		}

		//Hacemos descanasr las ranas madre
		BATR_descansar_criar();
	}


	return 0;
}
// ------------------------------------------------------------------------------------------------------



// ------------------------------------------------------------------------------------------------------
// Funcion MAIN
// Hace las llamadas principales
// -----------------------------------
int main (int argc, char *argv[]){

    // Variables de la funcion MAIN:
    int ms, tics;       // ms y tics que se pasan por parametro
    int id_posiciones;   //id de la memoria compartida de las posiciones
    int i, j, k;        // Contadores para bucles
    int lTroncos[7]={2,3,4,5,6,2,2};    //Longitudes medias de los troncos de cada fila. Se pueden generar aleatoriamente.
	int lAguas[7]={5,9,11,2,6,7,8};     //Longitudes medias de los espacios entre troncos de cada fila. Se pueden generar aleatoriamente.
	int direcciones[7]={1,0,1,0,1,0,1}; //Sentido en el que se mueven los troncos por la pantalla. DERECHA(0) o IZQUIERDA(1)

    pid_t pids_ranas_madre[4]; //guarda los pids de las ranas madre
    

    // Llamamos a la presentacion del programa
    presentacion();

    // Si no se introducen parametros, se mostrara un mensaje de error por el canal de errores estandar.
    if((argc != 2) && (argc != 3)){
        // Aqui vamos a entrar si el numero de parametros no es correcto. Recordemos, uno parametro obligatorio y otro opcional
        error_parametros();
        return -1;
    }

    // Realizamos el control de los parametros introducidos
    // Guardamos el primer parametro en la variable ms (milisegundos)
    ms=atoi(argv[1]);       // ms de "descanso"
    

    if((ms >1000) || (ms <0)){
        //Si el primer parametro esta fuera del rango
        fprintf(stderr, "\n ERROR. El primer parametro esta fuera del rango.\n Saliendo.\n");
        error_parametros();
        return -1;
    }
    // Si hay segundo parametro
    if(argc == 3){
        // Leemos el segundo parametro
        tics=atoi(argv[2]);     // numero de tics
        // Y realizamos la comprobacion
        if(tics < 1){
            // Si el numero es 0 o menor entonces mostramos un error y el mensaje de como se debe ejecutar
            fprintf(stderr, "\n ERROR. El segundo parametro esta fuera del rango.\n Saliendo.\n");
            error_parametros();
            return -1;
        }

    }// Si no hay argumento opcional
    else{
        // Guardamos el valor por defecto de TICS
        tics = TICS_DEFECTO;
    }

    // Mostramos los datos obtenidos al usuario
    fprintf(stdout, "\nLos datos introducidos son:\n");
    fprintf(stdout, "MS: %d\n", ms);
    fprintf(stdout, "TICS: %d\n", tics);    
    fprintf(stdout, "\n");


    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // Comenzamos a crear los recursos, semforos, etc.

    //Creamos el semaforo y guardamos su ID en la variable   
    id_semaforo = semget(IPC_PRIVATE, 10, IPC_CREAT | 0600);     // IPC_PRIVATE porque solo va a ser usado por el proceso y sus descendientes - 10 el num de semaforos
    fprintf(stdout, "Se ha creado el semaforo. Tiene una ID = %d\n", id_semaforo); 

    // Semaforo para el numero max de ranas hijas (MAX_RANAS_HIJAS)
    semctl(id_semaforo, MAIN_PANTALLA, SETVAL, 30);

    // Vamos a crear los semaforos que controlan las 4 ranas madre, que van a generar las ranitas
    // Uno para cada rana madre.
    semctl(id_semaforo, RANA_MADRE_1, SETVAL, 1);
    semctl(id_semaforo, RANA_MADRE_2, SETVAL, 1);
    semctl(id_semaforo, RANA_MADRE_3, SETVAL, 1);
    semctl(id_semaforo, RANA_MADRE_4, SETVAL, 1);
    fprintf(stdout, "Se han creado los semaforos que van a controlar las 4 ranas madre.\n"); 

    // Cramos semaforos para acceder a memoria compartida
    semctl(id_semaforo, SEMAF_RANITAS_NACIDAS, SETVAL, 1);
    semctl(id_semaforo, SEMAF_RANITAS_SALVADAS, SETVAL, 1);
    semctl(id_semaforo, SEMAF_RANITAS_MUERTAS, SETVAL, 1);
    semctl(id_semaforo, SEMAF_POSICIONES, SETVAL, 1);
    fprintf(stdout, "Se han creado los semaforos que van a controlar los accesos a memoria compartida\n"); 
    
    // Vamos a crear la memoria compartida
    // Los primeros 2048 bytes estan reservados para la biblioteca.
    // Guardamos 
    id_memoria=shmget(IPC_PRIVATE, 2048+sizeof(int) ,IPC_CREAT | IPC_EXCL | 0600);

    //se crea la memoria compartida para las posiciones
	id_posiciones=shmget(IPC_PRIVATE, 33*sizeof(struct posicion_struct),IPC_CREAT | IPC_EXCL | 0600);

    
    //enganchamos el proceso a los segmentos de memoria
	memoria = (char *)shmat( id_memoria, NULL, 0);
    
	posiciones =(struct posicion_struct *) shmat(id_posiciones, NULL, 0);

    //Guardamos la direccion de memoria en la var finalizar. Donde acaba la memoria de la biblioteca.
	finalizar=&memoria[2048];

    

    
    
    //Vamos a inicializar las posiciones a -2
	for (i = 0; i <=29; ++i)
	{
		posiciones[i].x=-2;
		posiciones[i].y=-2;
	}
    fprintf(stdout, "Se han inicializado las posiciones\n"); 

    //se inicializa la variable compartida para finalizar a 0
	*finalizar=0;
    
    
    //La memoria para las ranitas nacidas
	semaforo_wait(id_semaforo,SEMAF_RANITAS_NACIDAS);  
	posiciones[30].x=0;
	semaforo_signal(id_semaforo,SEMAF_RANITAS_NACIDAS);
    fprintf(stdout, "Mem. para las ranas nacidas\n"); 

    //memoria para las ranitas salvadas
	semaforo_wait(id_semaforo,SEMAF_RANITAS_SALVADAS);
	posiciones[31].x=0;
	semaforo_signal(id_semaforo,SEMAF_RANITAS_SALVADAS);
    fprintf(stdout, "Mem. para las ranas salvadas\n");

	//memoria para las ranitas perdidas
	semaforo_wait(id_semaforo,SEMAF_RANITAS_MUERTAS);
	posiciones[32].x=0;
	semaforo_signal(id_semaforo,SEMAF_RANITAS_MUERTAS);
    fprintf(stdout, "Mem. para las ranas perdidas\n");
    
    



	//-------------------------------------------------------------------------------------
	//registrar SIGINT para acabar correctamente
	struct sigaction ss;
  	ss.sa_handler = acabar; 
  	ss.sa_flags = 0;
  	sigfillset(&ss.sa_mask);
  	if (sigaction(SIGINT, &ss, NULL) ==-1){
  	    return 1;
    }

    fprintf(stdout, "------------------------------------------------------\n"); 

    // Dormimos el programa 5 segundos para que el usuario pueda leer los datos mostrados por pantalla
    //sleep(5);
    for(i=6;i>0;i--){
        fprintf(stdout, "Continuara en... %d\n", i); 
        sleep(1);
    }

    // -------------------------------------------------------------------------------------------------------------
    // -------------------------------------------------------------------------------------------------------------
    // -----------------------------------------   A POR LAS RANAS!!!   --------------------------------------------
    // -------------------------------------------------------------------------------------------------------------
    // -------------------------------------------------------------------------------------------------------------
    


    // COMENZAMOS!
    // Le vamos a pasar a la funcion todos los parametros para que comience.
    BATR_inicio(tics, id_semaforo, lTroncos, lAguas, direcciones, ms, memoria);

    // Creamos los procesos de las ranas madre, las que generan las ranitas
    // Con un for de 0 a 3 creamos los 4 procesos hijo
    for(i=0;i<=3;i++){
		pids_ranas_madre[i]=fork(); // Guardamos el pid de cada proceso en el array hijo
		switch (pids_ranas_madre[i]){
			case -1:    // En el caso de que se produzca un error
				perror("ERROR. fork");
				return 1;// Retornamos 1
			case 0: //Codigo del hijo
				return codigo_rana_madre(i);

		}//fin switch
	}//fin for


    // Necesitamos un bucle infinito para ejecutar todo, que solo se acaba cuando la variable
    // finalizar sea falsa
    while (!*finalizar){
        /* code */
        if (semaforo_signal(id_semaforo, SEMAF_POSICIONES) ==-1){
			continue;
		}

		// Vamos a recorrer las filas de troncos
		for (i = 4; i <= 10; i++)
		{
			
			//Vamos a recorrer todas las posiciones de las ranas
			for (j = 0; j <= 29; j++)
			{
				//si la coordenada y de la posicion es la del tronco que se va a mover, se cambia
				if (posiciones[j].y==i){
					if (direcciones[i-4]==DERECHA){
						(posiciones[j].x)=posiciones[j].x+1;
					}
					else{
						(posiciones[j].x)=posiciones[j].x-1;
					}
				}
			}

			BATR_avance_troncos(i-4);
			// Realizamos una pausita
			BATR_pausita();

		}
		semaforo_signal(id_semaforo, SEMAF_POSICIONES);
    }// FIN WHILE


	// PARA FINALIZAR TRAS RECIBIR EL SIGINT CTL C

	// Esperemos a que acaben las ranas madre

	pid_t id_proceso;
	int estado;
	// Con un buble FOR recorremos todas las ranas madre
	for(i=0; i<3; i++){
		id_proceso = waitpid(pids_ranas_madre[i], &estado, 0);
		if(id_proceso == -1){
			// En el caso de que haya un error mostramos el error
			perror("ERROR: Wait");
			return 1;
		}
	}
    
	// Vamos a realizar la comprobacion de que las ranas nacidas tiene que ser igual de ranas salvadas mas el de ranas muertas
	BATR_comprobar_estadIsticas(posiciones[30].x, posiciones[31].x, posiciones[32].x);

	// Vamos a mandar la orden con la funcion de biblioteca de que finalice:
	BATR_fin();




	// Procedemos a limpiar semaforos y memoria compartida.
	//Mem
	shmctl(id_memoria, IPC_RMID, NULL);
	//Semaf
	semctl(id_semaforo, 0, IPC_RMID);


	// Ya hemos limpiado todo y por lo tanto ya podemos retrnar para que acabe el proceso padre
	return 0;

}
// FIN MAIN------------------------------------------------------------------------------------------------------
