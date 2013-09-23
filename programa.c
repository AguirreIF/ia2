#include <argp.h>
#include <ctype.h>
#include <errno.h>
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
	{0, 0, 0, 0, "Sobre el programa:", -1},
	{0, 0, 0, 0, 0, 0}
};

/* Estructura para pasar datos de main a parse_opt */
struct args
{
	char *args[3], *entrada;
	long int poblacion, poblacion_maxima;
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
			for (int i = 0; i < (int) strlen (arg); i++)
				if (!isdigit (arg[i]))
					argp_error (state,
											"La población debe ser un número entero entre 1 y 100");

			args->poblacion = strtol (arg, NULL, 10);
			/* Verifica posibles errores  */
			if ((errno == ERANGE
					 && (args->poblacion == LONG_MAX
							 || args->poblacion == LONG_MIN))
					|| (errno != 0 && args->poblacion == 0))
				argp_failure (state, 1, errno, "Error de strtol");

			if (args->poblacion == 0)
				argp_error (state, "La población debe ser mayor a 0");
			else if (args->poblacion > 100)
				argp_error (state,
										"La población debe ser un número entero entre 1 y 100");
			break;

		case 'm':
			for (int i = 0; i < (int) strlen (arg); i++)
				if (!isdigit (arg[i]))
					argp_error (state,
											"La población máxima debe ser un número entero entre 1 y 3628800");

			args->poblacion_maxima = strtol (arg, NULL, 10);
			/* Verifica posibles errores */
			if ((errno == ERANGE
					 && (args->poblacion_maxima == LONG_MAX
							 || args->poblacion_maxima == LONG_MIN))
					|| (errno != 0 && args->poblacion_maxima == 0))
				argp_failure (state, 1, errno, "Error de strtol");

			if (args->poblacion_maxima == 0)
				argp_error (state, "La población máxima debe ser mayor a 0");
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
				char *sobrantes = NULL;
				for (int nro = state->next; nro < state->argc; nro++)
					{
						if (sobrantes == NULL)
							sobrantes = (char *) malloc (strlen (state->argv[nro]));
						sobrantes =
							realloc (sobrantes,
											 strlen (state->argv[nro]) + strlen (sobrantes) + 1);
						strcat (sobrantes, state->argv[nro]);
						strcat (sobrantes, " ");
					}
				argp_error (state, "Los últimos %d argumentos están de más: %s",
										state->argc - state->next, sobrantes);
				break;
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
	args.poblacion = -1;
	args.poblacion_maxima = 10000;
	args.entrada = NULL;

	static struct argp argp = {
		opciones, parse_opt, 0, doc, 0, 0, "es_AR"
	};
	argp_parse (&argp, argc, argv, 0, 0, &args);

	char *letras = NULL;
	char **operandos = NULL;
	char *operadores = NULL;
	int cantidad_operandos = 0;
	char *operacion = NULL;

	int salida = procesar (args.entrada, &letras, &operandos, &operadores,
												 &cantidad_operandos, &operacion);

	// todo bien
	if (salida == 0)
		{
			// calcula la cantidad de permutaciones posibles
			// según la cantidad de letras distintas
			int permutaciones = 10, indice = 1;
			while (indice < (int) strlen (letras))
				permutaciones *= (10 - indice++);

			/* si no especificó poblacion, se asigna un 10% por defecto */
			if (args.poblacion == -1)
				args.poblacion = 10;

			/* si el % representa una población > población máxima,
			 * entonces args.poblacion = args.poblacion_maxima. */
			if (((args.poblacion / 100.) * permutaciones) > args.poblacion_maxima)
				args.poblacion = args.poblacion_maxima;
			else
				args.poblacion = (args.poblacion / 100.) * permutaciones;

			printf ("Permutaciones: %d\nPoblación: %ld\n", permutaciones,
							args.poblacion);

			char *individuos = malloc (args.poblacion * 11);
			if (individuos == NULL)
				{
					printf ("Falló el malloc de individuos");
					exit (EXIT_FAILURE);
				}

			generar_poblacion_inicial ((char *) individuos, letras,
																 &args.poblacion);

			free (letras);

			/* CICLO ALGORITMO GENÉTICO  */
			while (1)
				{
					/* Verifica si es solución */
					long int individuo_solucion =
						funcion_de_parada ((char *) individuos, &args.poblacion,
															 operandos, &cantidad_operandos, operadores,
															 operacion);

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

					for (int i = 0; i < args.poblacion; i++)
						{
							long int aptitud = 0;
							aptitud = calcular_aptitud (&individuos[i * 11], operandos,
																					&cantidad_operandos, operadores,
																					operacion);
							if (aptitud == -1)
								puts ("No se pudo calcular la aptitud porque divide por 0");
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
