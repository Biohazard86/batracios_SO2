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

// Para compilarlo: 
//          gcc batracios.c libbatracios.a -lm -m32 -o batracios  
// Para ejecutarlo, por ejemplo:
//			./batracios 0
//			./batracios 0 1
//			./batracios 5


// -------------------------------------------------------------------------------------------------------
// LIBRERIAS USADAS
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <ctype.h>

// -------------------------------------------------------------------------------------------------------
// DEFINICIONES USADAS
#define TICS_DEFECTO 50				//Si el usuario no introduce el segundo parametro
#define MAX_TOTAL_PROCESOS 35		// El total de procesos permitidos
#define MAX_RANAS_MADRE 4			// El numero de ranas madre
#define MAX_RANAS_HIJAS 30			// El numero max de ranitas

// DEFINICIONES DE SEMAFOROS, aquí sólo definimos el numero dentro del conjunto de semaforos que van a tomar
#define MAIN_PANTALLA 1				// Semaf de la pantalla principal, controla las ranitas maximas que se pueden generar. En este caso el maimo es un total de MAX_RANAS_HIJAS (30 ranitas)
#define RANA_MADRE_1 2				// De la primera rana madre
#define RANA_MADRE_2 3				// De la segunda rana madre
#define RANA_MADRE_3 4				// De la tercera rana madre
#define RANA_MADRE_4 5				// De la cuarta rana madre
#define SEMAF_RANITAS_NACIDAS 6		// Semaforo que controla el acceso a la memoria compartida donde vamosa a guardar las ranas nacidas
#define SEMAF_RANITAS_SALVADAS 7	// idem para las ranas salvadas
#define SEMAF_RANITAS_MUERTAS 8		// idem para las ranas perdidas
#define SEMAF_POSICIONES 9			// Este sem va a controlar el acceso a las posiciones (mem. compartida)



// Definiciones de constantes
#define DERECHA 0					// Definimos derecha, izquierda y arriba. Para mover los troncos y las ranas
#define IZQUIERDA 1					// Definimos derecha, izquierda y arriba. Para mover los troncos y las ranas
#define ARRIBA 2					// Definimos derecha, izquierda y arriba. Para mover los troncos y las ranas
#define RANA_CRUZADA 11				// Para comprobar si la rana ha cruzado, ha llegado a la posicion 11 de la pantalla
#define RANDOM_MAX 12				// Para generar el agua y los troncos, los valores min y max
#define RANDOM_MIN 1				// Para generar el agua y los troncos, los valores min y max
#define FIN_FALLO 1    	 			// Lo que retornamos si hay fallo
#define FIN_EXITO 0     			// Si no hay fallo

//----------------------------------------------------------------------------------------
//Prototipos de las funciones de la biblioteca a modo de lista, en el PDF est'an todas descritas
int BATR_pausa(void);
int BATR_pausita(void);
int BATR_inicio(int ret,int semAforos, int long_troncos[],int long_agua[],int dirs[],int tCriar,char *zona);
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
// Estructura para las posiciones, con una X e Y, sirve para almacenar las posicinoes de cada ranita 
// No uso la definida en la librería ya que está definida como "posiciOn", con un cero en medio, más dificil de escribir varias veces
    struct posicion_struct {int x,y;};

// VARIABLES GLOBALES USADAS
	int global_control = 1;					//Cambia a 0 cuando queremos finalizar el programa AKA ctl+c
    int id_semaforo,id_memoria;       		//id de los semaforos y memoria
    char *memoria;                  		//puntero a memoria compartida para la biblioteca
    char *finalizar;                		//puntero a memoria compartida para la variable finalizar
    struct posicion_struct *posiciones;    	//Almacenamos las posiciones de las ranitas aqui


// -------------------------------------------------------------------------------------------------------
// Funcion PRESENTACION
// Hace una pequenia presentacion del programa. No tiene parametros, tampoco retorna nada.
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
// FIN PRESENTACION --------------------------------------------------------------------------------------


// -------------------------------------------------------------------------------------------------------
// Funcion EROR PARAMETROS
// Muestra como se usa el programa si se introducen mal los parametros. Esta funcion no tiene parametros, tampoco retorna nada.
// -----------------------------------
void error_parametros(){
    fprintf(stderr,"\n-> No se han introducido suficientes argumentos.\n");
    fprintf(stderr,"\n-> Para usar el programa son necesarios dos argumentos, el primero\n es obligatorio y el segundo es opcional\n");
    fprintf(stderr,"\n-> El primer parametro es un numero comprendido entre 0 y 1000\n");
    fprintf(stderr,"\n-> Este valor indicara la equivalencia en milisegundos de tiempo\n real de un tic de reloj (La lentitud con la que el programa\n funciona)\n");
    fprintf(stderr,"\n-> El segundo parametro es por defecto 50 y es la medida de tics\n que una rana madre necesita para descansar entre dos partos\n.");
    fprintf(stderr,"\n-> Debe ser un numero entero estrictamente mayor que 0. Si es 1\n o superior, la practica funcionaratanto mas lenta cuanto\n mayor sea el parametro y no debe consumir CPU\n");
    fprintf(stderr,"\n-> El programa esta preparado para recibir CRL+C desde el terminal\n y finalizar la ejecucion del programa\n");

}
// FIN ERROR_PARAMETROS -------------------------------------------------------------------------------------------------------


// -------------------------------------------------------------------------------------------------------
// Funcion ACABAR
// Funcion manejadora de la señal SIGINT, recibe la se;al s que es un entero

// -----------------------------------
void acabar(int s){
	if(s == SIGINT){
		*finalizar=1;	// Ponemos este puntero a 1
		global_control = 0;	// Ponemos la variable global a 
	}
	
}
// -------------------------------------------------------------------------------------------------------


// -------------------------------------------------------------------------------------------------------
// Funcion SEMAFORO_WAIT	(Funciona, reutilizadas)
// Se le tiene que pasar el id del semaforo y un indice, ambos son enteros
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
// Se le pasa la posicion y el indice, ambos son enteros.
// -----------------------------------
int ranita(int pos, int i){

	//Mientras no se pueda saltar y no hayamos acabado, se queda en el bucle.
	// Este es el primer salto
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

        // Comprobamos si la rana ha cruzado (RANA CRUZADA = 11)
		if (posiciones[pos].y == RANA_CRUZADA){
		
			//si ha sido salvada se dejan libres la posicion y el hueco en pantalla y acaba
			// Hacemos un wait
			semaforo_wait(id_semaforo, SEMAF_RANITAS_SALVADAS);
			posiciones[31].x++;	//Incrementamos el valor de la salvadas, ya que han cruzado
			// HAcemos un signal al semaforo
			semaforo_signal(id_semaforo,SEMAF_RANITAS_SALVADAS);

			posiciones[pos].x=-2;
			posiciones[pos].y=-2;
			// Si hemos llegado aqui es que podemos seguir
			semaforo_signal(id_semaforo, MAIN_PANTALLA);

			// SIGNAL A LA MEMORIA COMPARTIDA DE LAS POSICIONES
			semaforo_signal(id_semaforo, SEMAF_POSICIONES);

			return 0;
		}// END IF

		// Comprobamos que la x este entre 0 y 
		else if ( (posiciones[pos].x<=0 /*&& posiciones[pos].y!=-2*/) || posiciones[pos].x>=80) {
			//Comprobamos si no se ha salido alguna rana por un lado
			// En el caso de perder alguna, dejamos libre dicha posicion y hueco 
			
			//semaforo_wait(id_semaforo, SEMAF_RANITAS_MUERTAS);
			posiciones[32].x++;	// Aumentamos el contador de las ranas muertas
			//semaforo_signal(id_semaforo,SEMAF_RANITAS_MUERTAS);

			posiciones[pos].x=-2;
			posiciones[pos].y=-2;
			semaforo_signal(id_semaforo, MAIN_PANTALLA);

			//SIGNAL A LA MEMORIA COMPARTIDA DE LAS POSICIONES
			semaforo_signal(id_semaforo, SEMAF_POSICIONES);

			return 0;
		}// END ELSEIF


		// Si puede saltar entonces se ejecuta lo de dentro de este else if
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
			BATR_pausa();
		}
		
		// SIGNAL A LA MEMORIA COMPARTIDA DE LAS POSICIONES
		semaforo_signal(id_semaforo, SEMAF_POSICIONES);

		/*
		// No va aqui
		if(global_control == 0){
			BATR_explotar(posiciones[pos].x,posiciones[pos].y);
			posiciones[32].x++;	// Aumentamos el contador de las ranas muertas
			return 0;
		}
		*/

	}//FIN while


	// Si hemos presionado CTL+C global_control estara a 0
	if((global_control == 0) && (posiciones[pos].y > -1) && (posiciones[pos].y < 11) && (posiciones[pos].x < 80) && (posiciones[pos].x > 0)){
		// Entonces vamos a explotar esta rana, ya que si no se ha salido por un lado o
		// llegado arriba, eso significa que a'un esta atrapada en los troncos, debe ser explotada
		BATR_explotar(posiciones[pos].x,posiciones[pos].y);
		//Incrementamos el contador de ranas muertas, ya que explotar una rana es lo que tiene, que muere... Al menos eso me han dicho.
		posiciones[32].x++;	// Aumentamos el contador de las ranas muertas
		// Retornamos un valor de 0 a la funcion llamante, ya que no queda nada m'as que hacer amigos.
		return 0;
	}

	return 0;
}
// FIN RANITA ------------------------------------------------------------------------------------------------------





// -------------------------------------------------------------------------------------------------------
// Funcion RANA MADRE  (Acabada... Creo)
// Recibe un entero, que es el indice 
// Es el codigo que se ejecuta para crear los procesos hijos, es decir, las ranitas 
// -----------------------------------
int codigo_rana_madre(int i){

	int x,y;        //coordenadas iniciales de las ranitas creadas
	int posicion;   //Una de las 30 posiciones duspoibles para las ranitas 
	int j,k;        //contadores parael bucle
    pid_t id_ranita; //variable para el fork, el id de cada ranita

	while(!*finalizar){  // Bucle infinito

		//Hacemos descanasr las ranas madre
		BATR_descansar_criar();

		//Esperamos a que haya "hueco" para un nuevo proceso (ranita)
		if (semaforo_wait(id_semaforo,MAIN_PANTALLA)==-1){
            // Si hay hueco entonces continuamos
            continue;
        }
		
		//se hace un wait al semaforo que indica si la rana anterior ya se ha movido de la posicion inicial
		if (semaforo_wait(id_semaforo,(i+2))==-1){
            return 0;
        }

		//WAIT AL SEMAFORO DE POSICIONES
		if (semaforo_wait(id_semaforo, SEMAF_POSICIONES)==-1){
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

		if(global_control == 1){
			//Se llama a la funcion para crear ranitas
			BATR_parto_ranas(i, &posiciones[posicion].x,&posiciones[posicion].y);

			//SIGNAL AL SEMAFORO DE POSICIONES
			semaforo_signal(id_semaforo, SEMAF_POSICIONES);

			//Incrementamos el contador de ranas nacidas
			semaforo_wait(id_semaforo,SEMAF_RANITAS_NACIDAS);
			posiciones[30].x++;		// Incrementamos el contador de ranas nacidas
			semaforo_signal(id_semaforo,SEMAF_RANITAS_NACIDAS);


			//se crea un proceso para encargarse de la ranita a la cual se le indica la posicion y el numero de madre
			
			id_ranita=fork();
		}
		else{
			return 0;
		}
		
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

		
	}

	return 0;
}
// ------------------------------------------------------------------------------------------------------




// ------------------------------------------------------------------------------------------------------
// Funcion genera_aleatorio
// Genera los elementos de un string de forma aleatoria, recibe un puntero  que es ele string y un entero que es el numero de elementos que tenemos que generar.
// -----------------------------------

void genera_aleatorio(int *vector,int num){
	int i;

	if(num>1){
		//Recorremos todo el vector
		for(i=0; i<num;i++){
			//E introducimos los nums aleatorios
			vector[i] = rand() % (RANDOM_MAX + 1 - RANDOM_MIN) + RANDOM_MIN;
		}
		//RANDOM_MAX 
		//RANDOM_MIN
		//rand() % (max_number + 1 - minimum_number) + minimum_number

	}
	
	
}
// ------------------------------------------------------------------------------------------------------


// ------------------------------------------------------------------------------------------------------
// Funciones manejarores y misleep reutilizadas de la practica 1 de la Sesion 4

void manejadora()
{
}


// Funcion misleep sacada de una de las sesiones 
int misleep(int espera)
{
	int i;
	sigset_t mascaranueva,mascaravieja;
	struct sigaction accionNueva, acccionVieja;

	if(sigfillset(&mascaranueva)==-1)
	{
		perror("Fallo sigfillset");
		return FIN_FALLO;
	}

	accionNueva.sa_handler=manejadora;
	accionNueva.sa_mask=mascaranueva;
	accionNueva.sa_flags =0;

	if(sigaction(SIGALRM, &accionNueva, &acccionVieja)==-1)
	{
		perror("Fallo sigaction, accionNueva");
		return FIN_FALLO;
	}


	if(sigprocmask(SIG_SETMASK,&mascaranueva,&mascaravieja)==-1)
	{
		perror("Fallo sigprocmask, mascaranueva");
		return FIN_FALLO;
	}

	if(sigdelset(&mascaranueva,SIGALRM)==-1)
	{
		perror("Fallo sigdelset");
		return FIN_FALLO;
	}

	for(i=espera;i>=0;i--)
	{
		printf("%d ",i);
		alarm(1);
		sigsuspend(&mascaranueva);
	}
    printf("\n--------------\n");
	

	if(sigaction(SIGALRM, &acccionVieja, NULL)==-1)
	{
		perror("Fallo sigaction, acccionVieja ");
		return FIN_FALLO;
	}

	if(sigprocmask(SIG_SETMASK,&mascaravieja,NULL)==-1)
	{
		perror("Fallo sigprocmask, mascaravieja");
		return FIN_FALLO;
	}	
	return FIN_EXITO;
}



// ------------------------------------------------------------------------------------------------------


// ------------------------------------------------------------------------------------------------------
// Funcion MAIN
// Hace las llamadas principales
// Recibe los parametros de lanzamiento a traves de argc y argv
// ARGC es el numero de parametros y ARGV es un puntero a char con los elementos
// Recibe un entero, el numero de parametros que recibe y un puntero a char.
// -----------------------------------
int main (int argc, char *argv[]){

    // Variables de la funcion MAIN:
    int ms, tics, contador_explotadas;       // ms y tics que se pasan por parametro
    int id_posiciones;   //id de la memoria compartida de las posiciones
    int i, j, k;        // Contadores para bucles
    int long_troncos[7];    //Longitudes medias de los troncos de cada fila. Se pueden generar aleatoriamente.
	int long_agua[7];     //Longitudes medias de los espacios entre troncos de cada fila. Se pueden generar aleatoriamente.
	int sentidos_troncos[7]={1,0,1,0,1,0,1}; //Sentido en el que se mueven los troncos por la pantalla. DERECHA(0) o IZQUIERDA(1)
	int valor_devuelto;	// De la creacion de hijo de espera
	char parametro;
	

    pid_t pids_ranas_madre[4]; //guarda los pids de las ranas madre

	srand(time(NULL));   // Para generar numeros aleatorios.
	genera_aleatorio(&long_troncos[0],7);	// Genera las longitudes de los troncos
	genera_aleatorio(&long_agua[0],7);		// Genera las longitudes del agua

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
    fprintf(stdout, "\n Los datos introducidos son:\n");
    fprintf(stdout, " MS: %d\n", ms);
    fprintf(stdout, " TICS: %d\n", tics);    
    fprintf(stdout, "\n");


    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // Comenzamos a crear los recursos:semforos y memoria compartida.

    //Creamos el semaforo y guardamos su ID en la variable   
    id_semaforo = semget(IPC_PRIVATE, 10, IPC_CREAT | 0600);     // IPC_PRIVATE porque solo va a ser usado por el proceso y sus descendientes - 10 el num de semaforos
    fprintf(stdout, "Se ha creado el semaforo. Tiene una ID = %d\n", id_semaforo); 

    // Semaforo para el numero max de ranas hijas (MAX_RANAS_HIJAS, 30)
    semctl(id_semaforo, MAIN_PANTALLA, SETVAL, MAX_RANAS_HIJAS);

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

	if (id_posiciones == -1)
	{
		fprintf(stderr, "ERROR: no se ha creado la memoria compartida\n");
		return-1;
	}

    //enganchamos el proceso a los segmentos de memoria
	memoria = (char *)shmat( id_memoria, NULL, 0);
    
	posiciones =(struct posicion_struct *) shmat(id_posiciones, NULL, 0);

    //Guardamos la direccion de memoria en la var finalizar. Donde acaba la memoria de la biblioteca.
	finalizar=&memoria[2048];

    //Vamos a inicializar las posiciones a -2
	for (i = 0; i <=29; ++i){
		posiciones[i].x=-2;
		posiciones[i].y=-2;
	}

    fprintf(stdout, "Se han inicializado las posiciones\n"); 

    //se inicializa la variable compartida para finalizar a 0
	*finalizar=0;
    
    
    //La memoria para las ranitas nacidas
	semaforo_wait(id_semaforo,SEMAF_RANITAS_NACIDAS);  
	//Lo ponemos a 0 al principio
	posiciones[30].x=0;
	semaforo_signal(id_semaforo,SEMAF_RANITAS_NACIDAS);
    fprintf(stdout, "Mem. para las ranas nacidas\n"); 

    //memoria para las ranitas salvadas
	semaforo_wait(id_semaforo,SEMAF_RANITAS_SALVADAS);
	// Lo ponemos a 0 al principio
	posiciones[31].x=0;
	semaforo_signal(id_semaforo,SEMAF_RANITAS_SALVADAS);
    fprintf(stdout, "Mem. para las ranas salvadas\n");

	//memoria para las ranitas perdidas
	semaforo_wait(id_semaforo,SEMAF_RANITAS_MUERTAS);
	// Lo ponemos a 0 al principio
	posiciones[32].x=0;
	semaforo_signal(id_semaforo,SEMAF_RANITAS_MUERTAS);
    fprintf(stdout, "Mem. para las ranas perdidas\n");
    
    



	//-------------------------------------------------------------------------------------
	//registrar SIGINT para acabar correctamente CTL+C
	struct sigaction ss;
  	ss.sa_handler = acabar; 
  	ss.sa_flags = 0;
  	sigfillset(&ss.sa_mask);
  	if (sigaction(SIGINT, &ss, NULL) ==-1){
		perror("ERROR sigint");
  	    return 1;
    }

    fprintf(stdout, "------------------------------------------------------\n"); 
	fprintf(stdout, "El programa continuara en 7 segundos.\n"); 
	misleep(7);

	/*
	// Creamos un proceso hijo, para dormirlo 5 segundos y esperar por 'el
	valor_devuelto = fork();	// Creamos el hijo
	switch (valor_devuelto) {
	
	case 0: //Codigo del hijo...
		
		for(i=6;i>0;i--){
        	fprintf(stdout, "Continuara en... %d\n", i); 
        	sleep(1);
    	}
		return 1;
	
	break;
	default: //Codigo del padre...
		waitpid( -1, &valor_devuelto, 0);
		fprintf(stdout, "Se comienza\n");
		sleep(1);
	}//switch
	*/

    // Dormimos el programa 5 segundos para que el usuario pueda leer los datos mostrados por pantalla
    //sleep(5);
    //for(i=6;i>0;i--){
    //    fprintf(stdout, "Continuara en... %d\n", i); 
    //    sleep(1);
    //}


	// -------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------
    // -------------------------------------------------------------------------------------------------------------
    // -------------------------------------------------------------------------------------------------------------
    // -----------------------------------------   A POR LAS RANAS!!!   --------------------------------------------
    // -------------------------------------------------------------------------------------------------------------
    // -------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------
    

    // COMENZAMOS!
    // Le vamos a pasar a la funcion todos los parametros para que comience.
	// Los tics introducidos, el id del semaforo que hemos creado, las longitudes generadas aleatoriamente, las sentidos_troncos, los ms y la memoria compartida
    BATR_inicio(tics, id_semaforo, long_troncos, long_agua, sentidos_troncos, ms, memoria);

    // Creamos los procesos de las ranas madre, las que generan las ranitas
    // Con un for de 0 a 3 creamos los 4 procesos hijo AKA las ranas madre
    for(i=0;i<=3;i++){
		pids_ranas_madre[i]=fork(); // Guardamos el pid de cada proceso en el array de las ranas madre
		switch (pids_ranas_madre[i]){
			case -1:    // En el caso de que se produzca un error
				perror("ERROR. fork");
				return 1;// Retornamos 1
			case 0: //Codigo del hijo
				return codigo_rana_madre(i);

		}//fin switch
	}//fin for


    // Necesitamos un bucle "infinito" para ejecutar todo, que solo se acaba cuando la variable
    // finalizar sea falsa
    while (!*finalizar){
        /* code */
        if (semaforo_wait(id_semaforo, SEMAF_POSICIONES) ==-1) continue;
		

		// Vamos a recorrer las filas de troncos
		for (i = 4; i <= 10; i++)
		{
			
			//Vamos a recorrer todas las posiciones de las ranas
			for (j = 0; j <= 29; j++)
			{
				//si la coordenada y de la posicion es la del tronco que se va a mover, se cambia
				if (posiciones[j].y==i){
					if (sentidos_troncos[i-4]==DERECHA){
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
		// Hacemos un signal al semaforo que controla el acceso a las posicinoes para dejarlo "libre"
		semaforo_signal(id_semaforo, SEMAF_POSICIONES);
    }// FIN WHILE
//FOR QUE SIRVE PARA CORREGIR LAS RANITAS QUE SE HAN SALIDO POR LOS LADOS O QUE SE HAN SALVADO DESPUES DE PULSAR EL CTRL+C
	for(i=0;i<30;i++)
	{
		if ((posiciones[i].x < 0) || (posiciones[i].x >= 80))
		{
			if(posiciones[i].x != 999)
				posiciones[32].x; // MUERTAS
		}
		if (posiciones[i].y == 11){
			posiciones[31].x;	// SALVADAS
		}

	}

	// PARA FINALIZAR TRAS RECIBIR EL SIGINT CTL C
	// Esperemos a que acaben las ranas madre

	pid_t id_proceso;
	int estado;
	// Con un buble FOR recorremos todas las ranas madre
	for(i=0; i<3; i++){
		// Hacemos un wait a cada proceso rana madre
		id_proceso = waitpid(pids_ranas_madre[i], &estado, 0);
		// Vemos que ha devuelto cada wait
		if(id_proceso == -1){
			// En el caso de que haya un error mostramos el error
			perror("ERROR: Wait");
			return 1;
		}
	}


	// explotamos las ranas que hayan quedado dentro de los troncos para que se pierdan y las cuentas salgan
	/*
	for(i=0;i<30;i++){
		if((posiciones[i].x > -1) && (posiciones[i].y > -1)){

			BATR_explotar( posiciones[i].x,posiciones[i].y);
			posiciones[32].x++;
				
			
		
		}
		
	}
	

    sleep(1);

	*/


	// Vamos a realizar la comprobacion de que las ranas nacidas tiene que ser igual de ranas salvadas mas el de ranas muertas

	//printf("\n\t%d - %d - %d\n", posiciones[30].x, posiciones[31].x, posiciones[32].x);
	// Hacemos un sleep para ver que todo acabe antes de comprobar las estaditicas. (HAY QUE BORRAR ANTES DE ENTREGAR )
	sleep(3);
	BATR_comprobar_estadIsticas(posiciones[30].x, posiciones[31].x, posiciones[32].x );

	// Vamos a mandar la orden con la funcion de biblioteca de que finalice:
	BATR_fin();




	// Procedemos a limpiar semaforos y memoria compartida.
	//Mem
	shmctl(id_memoria, IPC_RMID, NULL);
	shmctl(id_posiciones, IPC_RMID, NULL);
	//Semaf
	semctl(id_semaforo, 0, IPC_RMID);


	// Ya hemos limpiado todo y por lo tanto ya podemos retrnar para que acabe el proceso padre
	return 0;

}
// FIN MAIN------------------------------------------------------------------------------------------------------
