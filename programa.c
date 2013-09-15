#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "funciones.h"

int
main (int argc, char **argv)
{
	long int poblacion = 0;

	// cuando es distinto de cero, getopt() imprime sus propios mensajes
	// de error para entradas inválidas o argumentos faltantes
	opterr = 0;

	int opcion;

	static struct option long_options[] = {
		{"poblacion", required_argument, 0, 'p'},
		{"help",      no_argument,       0, 'h'},
		{0,           0,                 0,  0}
	};

	while ((opcion =
					getopt_long (argc, argv, "+:p:e:h", long_options, NULL)) != -1)
		switch (opcion)
			{
			case 'p':
				// hay que colocar {} dentro de la etiqueta case para definir variables
				// http://stackoverflow.com/questions/92396/why-cant-variables-be-declared-in-a-switch-statement
				// http://complete-concrete-concise.com/programming/c/keyword-switch-case-default
				{
					int i;
					for (i = 0; i < (int) strlen (optarg); i++)
						{
							if (!isdigit (optarg[i]))
								{
									fputs
										("La población debe ser un número entero entre 1 y 3628800\n",
										 stderr);
									exit (EXIT_FAILURE);
								}
						}

					poblacion = strtol (optarg, NULL, 10);

					/* Verifica posibles errores */
					if ((errno == ERANGE && (poblacion == LONG_MAX || poblacion
																	 == LONG_MIN)) || (errno != 0
																										 && poblacion == 0))
						{
							perror ("Error en strtol: ");
							exit (EXIT_FAILURE);
						}

					if (poblacion == 0)
						{
							fputs ("La población debe ser mayor a 0", stderr);
							exit (EXIT_FAILURE);
						}

					break;
				}
			case 'h':
				printf
					("Uso: %s [-p|--poblacion <cantidad_poblacion> -h|--help]\n",
					 argv[0]);
				exit (EXIT_SUCCESS);
			case ':':
				fprintf (stderr, "Falta argumento de la opción `-%c'\n", optopt);
				exit (EXIT_FAILURE);
			case '?':
				if (isprint (optopt))
					fprintf (stderr, "Opción desconocida `-%c'\n", optopt);
				else
					fprintf (stderr, "Carácter de opción desconocido `\\x%x'\n",
									 optopt);
				exit (EXIT_FAILURE);
				// argumentos faltantes
			default:
				fprintf (stderr, "Error desconocido `\\x%x'\n", optopt);
				exit (EXIT_FAILURE);
			}

	char *entrada = NULL;

	// quedaron argumentos sin reconocer
	if (argc > optind)
		{
			// quedó un argumento sin reconocer, lo usamos como cadena de entrada
			if ((argc - optind) == 1)
				{
					entrada = (char *) malloc (strlen (argv[optind]));
					entrada = strdup (argv[optind]);
				}
			// quedó más de uno sin reconocer, error
			else
				{
					// se incrementa porque el primer argumento se usa como entrada
					// a partir del segundo no reconocido es inválido
					optind++;
					printf ("Argumentos desconocidos: ");
					for (opcion = optind; opcion < argc; opcion++)
						printf ("%s ", argv[opcion]);
					puts ("");
					exit (EXIT_FAILURE);
				}
		}

	char *letras = NULL;
	char **operandos = NULL;
	char *operadores = NULL;
	int cantidad_operandos = 0;

	int salida =
		procesar (entrada, &letras, &operandos, &operadores, &cantidad_operandos);

	// todo bien
	if (salida == 0)
		{
			// calcula la cantidad de permutaciones posibles
			// según la cantidad de letras distintas
			int permutaciones = 10, indice = 1;
			while (indice < (int) strlen (letras))
				permutaciones *= (10 - indice++);

			if ((poblacion > permutaciones) || (poblacion == 0))
				{
					poblacion = permutaciones * .1;
					if (poblacion > 2000)
						poblacion = 2000;
				}

			printf ("Permutaciones: %d\nPoblación: %ld\n", permutaciones,
							poblacion);

			char *individuos = (char *) malloc (poblacion * 11);
			if (individuos == NULL)
				{
					printf ("Falló el malloc de individuos");
					exit (EXIT_FAILURE);
				}

			generar_poblacion_inicial ((char *) individuos, letras, &poblacion);

			free (letras);

			/* CICLO ALGORITMO GENÉTICO  */
			while (1)
				{
					/* Verifica si es solución */
					long int individuo_solucion =
						funcion_de_parada ((char *) individuos, &poblacion, operandos,
															 &cantidad_operandos, operadores);

					if (individuo_solucion != -1)
						{
							printf ("\n¡Solución! individuo: %ld\n", individuo_solucion);
							int indice;
							for (indice = 0; indice < 10; indice++)
								if (individuos[individuo_solucion * 11 + indice] != '\0')
									printf ("%c --> %d\t\t",
													individuos[individuo_solucion * 11 + indice],
													indice);
							puts ("");
						}

					/* int j; */
					/* uint32_t i; */
					/* for (i = 0; i < poblacion; i++) */
					/* { */
					/* printf ("\nindividuo[%d]: %c%c%c%c%c%c%c%c%c%c\n", i, */
					/* individuos[i * 11 + 0], individuos[i * 11 + 1], */
					/* individuos[i * 11 + 2], individuos[i * 11 + 3], */
					/* individuos[i * 11 + 4], individuos[i * 11 + 5], */
					/* individuos[i * 11 + 6], individuos[i * 11 + 7], */
					/* individuos[i * 11 + 8], individuos[i * 11 + 9]); */
					/* for (j = 0; j < 10; j++) */
					/* if (individuos[i * 11 + j] != '\0') */
					/* printf ("%c --> %d\t\t", individuos[i * 11 + j], j); */
					/* } */

					/* puts ("Operandos:"); */
					/* for (j = 0; j < cantidad_operandos; j++) */
					/* printf ("%d: %s\n", j, operandos[j]); */

					/* for (j = 0; j < (int) strlen (operadores); j++) */
					/* printf ("%c\t", operadores[j]); */

					break;
				}

			exit (EXIT_SUCCESS);
		}
	else if (salida == 1)
		{
			fputs ("Algún error en el parsing", stderr);
			exit (EXIT_FAILURE);
		}
	else if (salida == 2)
		{
			fputs ("No puede haber más de 10 letras distintas", stderr);
			exit (EXIT_FAILURE);
		}
}
