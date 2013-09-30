#include <argp.h>
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "funciones.h"

/* Info varia */
const char *argp_program_version = "void";
const char *argp_program_bug_address = "<void@null.org>";
static const char doc[] =
	"Resolución de problemas criptoaritméticos con AG";

/* Opciones aceptadas */
static const struct argp_option opciones[] = {
	{0, 0, 0, 0, "Opciones para la población:", 1},
	{"poblacion", 'p', "PORCENTAJE", 0,
	 "El porcentaje de permutaciones que se usará como población inicial", 0},
	{"poblacion-max", 'm', "NUM", 0,
	 "La cantidad máxima de población inicial", 0},
	{"semilla", 's', "SEMILLA", 0,
	 "Semilla para generar la población inicial", 0},
	{"generaciones", 'g', "GENERACIONES", 0,
	 "Cantidad de generaciones", 0},
	{"aptitud", 'a', "APTITUD", 0,
	 "Función de aptitud a utilizar", 0},
	{0, 0, 0, 0, "Sobre el programa:", -1},
	{0, 0, 0, 0, 0, 0}
};

/* Estructura para pasar datos de main a parse_opt */
struct args
{
	char *args[5], *entrada;
	unsigned long int *semilla, generaciones, poblacion, poblacion_maxima;
	void (*faptitud) (struct individuos_s * restrict const individuo,
										char **restrict const operandos,
										const unsigned int *restrict const cantidad_operandos,
										const char *restrict const operadores,
										char *const operacion);
};

/* Función que hace el parsing de las opciones */
static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
	/* Get the input argument from argp_parse, which we
	   know is a pointer to our arguments structure. */
	struct args *args = state->input;

	switch (key)
		{
		case 'p':
			for (unsigned int i = 0; i < (unsigned int) strlen (arg); i++)
				if (!isdigit (arg[i]))
					argp_error (state,
											"La población debe ser un número entero entre 1 y 100");

			unsigned long int n_aux = strtoul (arg, NULL, 10);

			/* Verifica posibles errores de conversión */
			if ((errno == ERANGE && n_aux == ULONG_MAX)
					|| (errno != 0 && n_aux == 0))
				argp_failure (state, 1, errno, "Error de strtoul");

			args->poblacion = n_aux;

			if (args->poblacion == 0)
				argp_error (state,
										"La población debe ser un número entero entre 1 y 100");
			else if (args->poblacion > 100)
				argp_error (state,
										"La población debe ser un número entero entre 1 y 100");
			break;

		case 'm':
			for (unsigned int i = 0; i < (unsigned int) strlen (arg); i++)
				if (!isdigit (arg[i]))
					argp_error (state,
											"La población máxima debe ser un número entero mayor a 0");

			n_aux = strtoul (arg, NULL, 10);

			/* Verifica posibles errores de conversión */
			if ((errno == ERANGE && n_aux == ULONG_MAX)
					|| (errno != 0 && n_aux == 0))
				argp_failure (state, 1, errno, "Error de strtoul");

			args->poblacion_maxima = n_aux;

			if (args->poblacion_maxima == 0)
				argp_error (state,
										"La población máxima debe ser un número entero mayor a 0");
			break;

		case 'g':
			for (unsigned int i = 0; i < (unsigned int) strlen (arg); i++)
				if (!isdigit (arg[i]))
					argp_error (state,
											"La cantidad de generaciones debe ser un número entero entre 1 y %lu",
											ULONG_MAX);

			n_aux = strtoul (arg, NULL, 10);

			/* Verifica posibles errores de conversión */
			if ((errno == ERANGE && n_aux == ULONG_MAX)
					|| (errno != 0 && n_aux == 0))
				argp_failure (state, 1, errno, "Error de strtoul");

			args->generaciones = n_aux;

			if (args->generaciones == 0)
				argp_error (state,
										"La cantidad de generaciones debe ser un número entero entre 1 y %lu",
										ULONG_MAX);
			break;

		case 's':
			for (unsigned int i = 0; i < (unsigned int) strlen (arg); i++)
				if (!isdigit (arg[i]))
					argp_error (state,
											"La semilla debe ser un número entero entre 0 y %lu",
											ULONG_MAX);

			n_aux = strtoul (arg, NULL, 10);

			/* Verifica posibles errores de conversión */
			if ((errno == ERANGE && n_aux == ULONG_MAX)
					|| (errno != 0 && n_aux == 0))
				argp_failure (state, 1, errno, "Error de strtoul");

			args->semilla = malloc (sizeof (unsigned long int));
			*args->semilla = n_aux;
			break;

		case 'a':
			for (unsigned int i = 0; i < (unsigned int) strlen (arg); i++)
				if (!isdigit (arg[i]))
					argp_error (state,
											"Debe elegir entre las funciones de aptitud 1, 2 o 3");

			unsigned int aptitud = atoi (arg);

			if (aptitud < 1 || aptitud > 3)
				argp_error (state,
										"Debe elegir entre las funciones de aptitud 1, 2 o 3");

			if (atoi (arg) == 1)
				args->faptitud = calcular_aptitud1;
			else if (atoi (arg) == 2)
				args->faptitud = calcular_aptitud2;
			else if (atoi (arg) == 3)
				args->faptitud = calcular_aptitud3;
			break;

		case ARGP_KEY_ARG:
			/* Procesa el argumento */
			if (state->arg_num > 0)
				// hace que  llame a parse_opt con ARGP_KEY_ARGS
				return ARGP_ERR_UNKNOWN;
			args->entrada = strdup (arg);
			break;

		case ARGP_KEY_ARGS:
			{
				char *sobrantes = malloc (1);
				sobrantes[0] = '\0';
				for (int nro = state->next; nro < state->argc; nro++)
					{
						sobrantes =
							realloc (sobrantes,
											 strlen (state->argv[nro]) + strlen (sobrantes) + 2);
						strcat (sobrantes, state->argv[nro]);
						strcat (sobrantes, " ");
					}
				argp_error (state, "Los últimos %d argumentos están de más: %s",
										state->argc - state->next, sobrantes);
			}

		default:
			return ARGP_ERR_UNKNOWN;
		}
	return 0;
}

int
main (int argc, char **argv)
{
	struct args args;
	args.poblacion = 0;
	args.poblacion_maxima = 10000;
	args.entrada = NULL;
	args.semilla = NULL;
	args.faptitud = calcular_aptitud1;

	static struct argp argp = {
		opciones, parse_opt, 0, doc, 0, 0, "es_AR"
	};
	argp_parse (&argp, argc, argv, 0, 0, &args);

	char *letras = NULL;
	char **operandos = NULL;
	char *operadores = NULL;
	unsigned int cantidad_operandos = 0;
	char *operacion = NULL;

	int salida = procesar (args.entrada, &letras, &operandos, &operadores,
												 &cantidad_operandos, &operacion);

	// todo bien
	if (salida == 0)
		{
			// calcula la cantidad de permutaciones posibles
			// según la cantidad de letras distintas
			unsigned long int permutaciones = 10;
			for (unsigned int indice = 1; indice < (unsigned int) strlen (letras);
					 indice++)
				permutaciones *= (10 - indice);

			/* si no especificó poblacion, se asigna un 10% por defecto */
			if (args.poblacion == 0)
				args.poblacion = 10;

			/* si el % representa una población > población máxima,
			 * entonces args.poblacion = args.poblacion_maxima. */
			if (((args.poblacion / 100.) * permutaciones) > args.poblacion_maxima)
				args.poblacion = args.poblacion_maxima;
			else
				args.poblacion = (args.poblacion / 100.) * permutaciones;

			printf ("Permutaciones: %lu\nPoblación: %lu\n", permutaciones,
							args.poblacion);

			struct individuos_s *individuos =
				malloc (args.poblacion * sizeof (struct individuos_s));

			generar_poblacion_inicial (&individuos, letras, &args.poblacion,
																 args.semilla);

			if (args.semilla != NULL)
				free (args.semilla);

			free (letras);

			/* Calcula aptitud de la población */
			for (unsigned long int i = 0; i < args.poblacion; i++)
				args.faptitud (&individuos[i], operandos,
											 &cantidad_operandos, operadores, operacion);

			/* Ordena los individuos por aptitud */
			qsort (individuos, args.poblacion, sizeof (struct individuos_s),
						 individuos_cmp);

			/* Verifica si hay algún individuo solución */
			if (individuos[0].aptitud == 0)
				{
					puts ("\n¡Solución!");
					for (unsigned int indice = 0; indice < 10; indice++)
						if (individuos[0].letras[indice] != '\0')
							printf ("%c --> %u\t\t", individuos[0].letras[indice], indice);
					puts ("");
					exit (EXIT_SUCCESS);
				}

			/* Se toma el 5% como selección elitista
			 * Serían las estructuras
			 * individuos[0] a individuos[cantidad_elite - 1] */
			unsigned long int cantidad_elite = args.poblacion * .05;

			/* Cantidad de individuos restantes */
			unsigned long int cantidad_restantes = args.poblacion - cantidad_elite;

			/* CICLO ALGORITMO GENÉTICO  */
			for (unsigned long int generacion = 0; generacion < args.generaciones;
					 generacion++)
				{
					puts ("\n=================================");
					printf ("       GENERACIÓN %lu\n", generacion + 1);
					puts ("=================================\n");

					puts ("Población (ordenada por aptitud)");
					puts ("--------------------------------");
					for (unsigned long int i = 0; i < args.poblacion; i++)
						printf
							("Individuo[%2lu]: %c%c%c%c%c%c%c%c%c%c = %llu\n",
							 i + 1, individuos[i].letras[0], individuos[i].letras[1],
							 individuos[i].letras[2], individuos[i].letras[3],
							 individuos[i].letras[4], individuos[i].letras[5],
							 individuos[i].letras[6], individuos[i].letras[7],
							 individuos[i].letras[8], individuos[i].letras[9],
							 individuos[i].aptitud);
					/* for (int indice = 0; indice < 10; indice++) */
					/* if (individuos[i].letras[indice] != '\0') */
					/* printf ("%c --> %d\t\t", individuos[i].letras[indice], indice); */

					printf ("\nCantidad elite: %lu\n", cantidad_elite);
					printf ("Cantidad por copias esperadas: %lu\n\n",
									cantidad_restantes);

					seleccion_por_ranking_con_ce (&individuos, &cantidad_elite,
																				&cantidad_restantes, 0);

					puts ("Individuos después de selección elitista y por ranking");
					puts ("------------------------------------------------------");
					for (unsigned long int i = 0; i < args.poblacion; i++)
						printf
							("Aptitud individuo[%2lu]: %c%c%c%c%c%c%c%c%c%c = %llu\n",
							 i + 1, individuos[i].letras[0], individuos[i].letras[1],
							 individuos[i].letras[2], individuos[i].letras[3],
							 individuos[i].letras[4], individuos[i].letras[5],
							 individuos[i].letras[6], individuos[i].letras[7],
							 individuos[i].letras[8], individuos[i].letras[9],
							 individuos[i].aptitud);

					/* Se toma como punto/s de cruza un número aleatorio entre 1 y 4 */
					unsigned int punto =
						(unsigned int) (1 + rand () / (RAND_MAX / (5 - 1) + 1));
					/* No se cruzan los individuos elite */
					cruza (&individuos[cantidad_elite], &cantidad_restantes, punto);

					for (unsigned long int i = cantidad_elite; i < cantidad_restantes;
							 i++)
						args.faptitud (&individuos[i], operandos,
													 &cantidad_operandos, operadores, operacion);

					puts ("\nIndividuos después de cruzar");
					puts ("----------------------------");
					for (unsigned long int i = cantidad_elite; i < args.poblacion; i++)
						printf
							("Aptitud individuo[%2lu]: %c%c%c%c%c%c%c%c%c%c = %llu\n",
							 i + 1, individuos[i].letras[0], individuos[i].letras[1],
							 individuos[i].letras[2], individuos[i].letras[3],
							 individuos[i].letras[4], individuos[i].letras[5],
							 individuos[i].letras[6], individuos[i].letras[7],
							 individuos[i].letras[8], individuos[i].letras[9],
							 individuos[i].aptitud);

					/* Ordena los individuos por aptitud */
					qsort (individuos, args.poblacion, sizeof (struct individuos_s),
								 individuos_cmp);

					/* Verifica si hay algún individuo solución */
					if (individuos[0].aptitud == 0)
						{
							puts ("\n¡Solución!");
							for (unsigned int indice = 0; indice < 10; indice++)
								if (individuos[0].letras[indice] != '\0')
									printf ("%c --> %u\t\t",
													individuos[0].letras[indice], indice);
							puts ("");
							exit (EXIT_SUCCESS);
						}
				}

			/* No ejecuta cuando encuentra solución */
			while (args.poblacion-- > 0)
				free (individuos[args.poblacion].letras);
			free (individuos);
			while (cantidad_operandos-- > 0)
				free (operandos[cantidad_operandos]);
			free (operandos);
			free (operadores);
			free (operacion);
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
