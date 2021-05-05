// CURSO 2020/2021
// SISTEMAS OPERATIVOS II
// DAVID BARRIOS PORTALES
// Victor Vacas Amigo
//      ------------------------------
//      ||       BATRACIOS          ||
//      ------------------------------ 
// github.com/Biohazard86/batracios_SO2


// Para poder compilar el programa, hay que hacerlo en el modo de 32bits
// Se compila con:
//        gcc batracios.c libbatracios.a -lm -o batracios 


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


// -------------------------------------------------------------------------------------------------------
// VARIABLES GLOBALES USADAS
    int idSemaforo,idMemoria;       //id de los semaforos y memoria
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
    fprintf(stderr,"\n\t|                GRUPO ?                 |");
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


// ------------------------------------------------------------------------------------------------------
// Funcion MAIN
// Hace las llamadas principales
// -----------------------------------
int main (int argc, char *argv[]){

    // Variables de la funcion MAIN:
    int ms, tics;
    

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

}
// FIN MAIN------------------------------------------------------------------------------------------------------