// DAVID BARRIOS PORTALES


// GIISI
// SISTEMAS OPERATIVOS II
// PRACTICA WINDOWS: BATRACIOS
// 2021/2021












//LIBRER'IAS USADAS
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// DEFINICIONES
#define TICS_DEFECTO 50


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





// ------------------------------------------------------------------------------------------------------
// Funcion MAIN
// Hace las llamadas principales
// Recibe los parametros de lanzamiento a traves de argc y argv
// ARGC es el numero de parametros y ARGV es un puntero a char con los elementos
// Recibe un entero, el numero de parametros que recibe y un puntero a char.
// -----------------------------------
int main(int argc, char *argv[]) {
	
	int ms, tics;
	
	
	
	// Llamamos a la presentacion del programa
    presentacion();

    // Si no se introducen parametros, se mostrara un mensaje de error por el canal de errores estandar.
    if((argc != 2) && (argc != 3)){
        // Aqui vamos a entrar si el numero de parametros no es correcto. Recordemos, uno parametro obligatorio y otro opcional
        error_parametros();
        return -1;
    }
	
	
	// Guardamos el primer parametro en la variable ms (milisegundos)
    ms=atoi(argv[1]);       // ms de "descanso"
	
	// Realizamos la comprobaci'on de que el primer dato introducido sea entre 0 y 1000
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

	
	
	
	
	
	// FIN
	return 0;
}
