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
#define TICS_DEFECTO 50
#define MAX_TOTAL_PROCESOS 35
#define MAX_RANAS_MADRE 4
#define MAX_RANAS_HIJAS 30
// DEFINICIONES DE SEMAFOROS
#define MAIN_PANTALLA 1
#define RANA_MADRE_1 2
#define RANA_MADRE_2 3
#define RANA_MADRE_3 4
#define RANA_MADRE_4 5
#define SEMAF_RANITAS_NACIDAS 6
#define SEMAF_RANITAS_SALVADAS 7
#define SEMAF_RANITAS_MUERTAS 8
#define SEMAF_POSICIONES 9


// -------------------------------------------------------------------------------------------------------
// VARIABLES GLOBALES USADAS
    int id_semaforo,idMemoria;       //id de los semaforos y memoria
    char *memoria;                  //puntero a memoria compartida para la biblioteca
    char *finalizar;                //puntero a memoria compartida para la variable finalizar
    struct posicion *posiciones;    //puntero a memoria compartida para las posiciones

    // Estructura
    struct posicion {int x,y;};



// -------------------------------------------------------------------------------------------------------
// Funcion PRESENTACION
// Hace una pequenia presentacion de 
// -----------------------------------
void presentacion(){
    fprintf(stderr,"\n");
    fprintf(stderr,"\n\t|----------------------------------------|");
    fprintf(stderr,"\n\t|               BATRACIOS                |");
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
void acabar(int s)
{
	*finalizar=1;
}
// -------------------------------------------------------------------------------------------------------




// -------------------------------------------------------------------------------------------------------
// Funcion SEMAFORO_WAIT
// Se le tiene que pasar el id del semaforo y un indice 
// -----------------------------------
int semaforo_wait( int semid, int indice)
{
	struct sembuf oper;

	oper.sem_num=indice; //sobre el semaforo indicado
	oper.sem_op= -1; //se hace un wait
	oper.sem_flg=0; //sin indicadores adicionales

	return semop(semid, &oper, 1); //se realiza una operación 
}
// -------------------------------------------------------------------------------------------------------


// -------------------------------------------------------------------------------------------------------
// Funcion SEMAFORO_WAIT
// Se le tiene que pasar el id del semaforo y un indice 
// -----------------------------------
int sem_signal( int semid, int indice)
{

	struct sembuf oper;

	oper.sem_num=indice; //sobre el semaforo indicado
	oper.sem_op= +1; //se hace un wait
	oper.sem_flg=0; //sin indicadores adicionales

	return semop(semid, &oper, 1); //se realiza una operación 
}
// -------------------------------------------------------------------------------------------------------


// ------------------------------------------------------------------------------------------------------
// Funcion MAIN
// Hace las llamadas principales
// -----------------------------------
int main (int argc, char *argv[]){

    // Variables de la funcion MAIN:
    int ms, tics;       // ms y tics que se pasan por parametro
    int idposiciones;   //id de la memoria compartida de las posiciones
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
    fprintf(stdout, "\n\nLos datos introducidos son:");
    fprintf(stdout, "\nMS: %d", ms);
    fprintf(stdout, "\nTICS: %d", tics);    
    fprintf(stdout, "\n");


    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // Comenzamos "el programa"

    //Creamos el semaforo y guardamos su ID en la variable   
    id_semaforo = semget( IPC_PRIVATE,10, IPC_CREAT | 0600);     // IPC_PRIVATE porque solo va a ser usado por el proceso y sus descendientes - 10 el num de semaforos

    // Semaforo para el numero max de ranas hijas (MAX_RANAS_HIJAS)
    semctl(id_semaforo, MAIN_PANTALLA, SETVAL, 1);

    // Vamos a crear los semaforos que controlan las 4 ranas madre, que van a generar las ranitas
    // Uno para cada rana madre.
    semctl(id_semaforo, RANA_MADRE_1, SETVAL, 1);
    semctl(id_semaforo, RANA_MADRE_2, SETVAL, 1);
    semctl(id_semaforo, RANA_MADRE_3, SETVAL, 1);
    semctl(id_semaforo, RANA_MADRE_4, SETVAL, 1);


    // Cramos semaforos para acceder a memoria compartida
    semctl(id_semaforo, SEMAF_RANITAS_NACIDAS, SETVAL, 1);
    semctl(id_semaforo, SEMAF_RANITAS_SALVADAS, SETVAL, 1);
    semctl(id_semaforo, SEMAF_RANITAS_MUERTAS, SETVAL, 1);
    semctl(id_semaforo, SEMAF_POSICIONES, SETVAL, 1);

    // Vamos a crear la memoria compartida
    // Los primeros 2048 bytes estan reservados para la biblioteca.
    // Guardamos 
    idMemoria=shmget(IPC_PRIVATE, 2048+sizeof(int) ,IPC_CREAT | IPC_EXCL | 0600);

    //enganchamos el proceso a los segmentos de memoria
	memoria = (char *)shmat( idMemoria, NULL, 0);
	posiciones =(struct posicion *) shmat(idposiciones, NULL, 0);

    //Guardamos la direccion de memoria en la var finalizar. Donde acaba la memoria de la biblioteca.
	//finalizar=&memoria[2048];

    //se inicializa la variable compartida para finalizar a 0
	*finalizar=0;

    //Vamos a inicializar las posiciones a -3
	for (i = 0; i <=29; ++i)
	{
		posiciones[i].x=-3;
		posiciones[i].y=-3;
	}

    //La memoria para las ranitas nacidas
	semaforo_wait(id_semaforo,SEMAF_RANITAS_NACIDAS);  
	posiciones[30].x=0;
	sem_signal(id_semaforo,SEMAF_RANITAS_NACIDAS);

    //memoria para las ranitas salvadas
	semaforo_wait(id_semaforo,SEMAF_RANITAS_SALVADAS);
	posiciones[31].x=0;
	sem_signal(id_semaforo,SEMAF_RANITAS_SALVADAS);

	//memoria para las ranitas perdidas
	semaforo_wait(id_semaforo,SEMAF_RANITAS_MUERTAS);
	posiciones[32].x=0;
	sem_signal(id_semaforo,SEMAF_RANITAS_MUERTAS);

	//-------------------------------------------------------------------------------------
	//registrar SIGINT para acabar correctamente
	struct sigaction ss;
  	ss.sa_handler=acabar; 
  	ss.sa_flags=0;
  	sigfillset(&ss.sa_mask);
  	if (sigaction(SIGINT, &ss, NULL) ==-1){
  	    return 1;
    }


    // COMENZAMOS!
    // Le vamos a pasar a la funcion todos los parametros para que comience.
    BATR_inicio(tics, id_semaforo, lTroncos, lAguas, direcciones, ms, memoria);



}
// FIN MAIN------------------------------------------------------------------------------------------------------
