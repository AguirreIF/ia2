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
	long int poblacion = -1;
	long int poblacion_maxima = 10000;

	// cuando es distinto de cero, getopt() imprime sus propios mensajes
	// de error para entradas inválidas o argumentos faltantes
	opterr = 0;

	int opcion;

	static struct option long_options[] = {
		{"poblacion", required_argument, 0, 'p'},
		{"poblacion-max", required_argument, 0, 'm'},
		{"help",      no_argument,       0, 'h'},
		{0,           0,                 0,  0}
	};

	while ((opcion =
					getopt_long (argc, argv, "+:p:m:h", long_options, NULL)) != -1)
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
										("La población debe ser un número entero entre 1 y 100\n",
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
					else if (poblacion > 100)
						{
							fputs
								("La población debe ser un número entero entre 1 y 100\n",
								 stderr);
							exit (EXIT_FAILURE);
						}

					break;
				}
			case 'm':
				{
					int i;
					for (i = 0; i < (int) strlen (optarg); i++)
						{
							if (!isdigit (optarg[i]))
								{
									fputs
										("La población máxima debe ser un número entero entre 1 y 3628800\n",
										 stderr);
									exit (EXIT_FAILURE);
								}
						}

					poblacion_maxima = strtol (optarg, NULL, 10);

					/* Verifica posibles errores */
					if ((errno == ERANGE
							 && (poblacion_maxima == LONG_MAX
									 || poblacion_maxima == LONG_MIN)) || (errno != 0
																												 && poblacion_maxima
																												 == 0))
						{
							perror ("Error en strtol: ");
							exit (EXIT_FAILURE);
						}

					if (poblacion == 0)
						{
							fputs ("La población máxima debe ser mayor a 0", stderr);
							exit (EXIT_FAILURE);
						}

					break;
				}
			case 'h':
				printf
					("Uso: %s [-p|--poblacion <cantidad_poblacion> -m|--poblacion-max <poblacion_maxima> -h|--help]\n",
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

			/* si no especificó poblacion, se asigna un 10% por defecto */
			if (poblacion == -1)
				poblacion = 10;

			/* si el % representa una población > poblacion_maxima, entonces poblacion = poblacion_maxima. */
			if (((poblacion / 100.) * permutaciones) > poblacion_maxima)
				poblacion = poblacion_maxima;
			else
				poblacion = (poblacion / 100.) * permutaciones;

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
							printf ("\n¡Solución! individuo: %ld\n",
											individuo_solucion + 1);
							int indice;
							for (indice = 0; indice < 10; indice++)
								if (individuos[individuo_solucion * 11 + indice] != '\0')
									printf ("%c --> %d\t\t",
													individuos[individuo_solucion * 11 + indice],
													indice);
							puts ("");
						}

					int i = 0;

					do
						{
							long int aptitud = calcular_aptitud (&individuos[i * 11],
																									 operandos,
																									 &cantidad_operandos,
																									 operadores);
							printf ("Aptitud individuo[%d]: %c%c%c%c%c%c%c%c%c%c = %ld\n",
											i + 1, individuos[i * 11 + 0], individuos[i * 11 + 1],
											individuos[i * 11 + 2], individuos[i * 11 + 3],
											individuos[i * 11 + 4], individuos[i * 11 + 5],
											individuos[i * 11 + 6], individuos[i * 11 + 7],
											individuos[i * 11 + 8], individuos[i * 11 + 9],
											aptitud);

							for (indice = 0; indice < 10; indice++)
								if (individuos[i * 11 + indice] != '\0')
									printf ("%c --> %d\t\t", individuos[i * 11 + indice],
													indice);

							puts ("\n");
						}
					while (++i < poblacion);

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
