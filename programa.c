#include <argp.h>
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

	{0, 0, 0, 0, "Opciones del AG", 2},

	{"corridas", 'r', "CORRIDAS", 0,
	 "Cantidad de corridas del AG", 0},

	{"semilla", 's', "SEMILLA", 0,
	 "Semilla para generar la población inicial", 0},

	{"generaciones", 'g', "GENERACIONES", 0,
	 "Cantidad de generaciones", 0},

	{"aptitud", 'a', "APTITUD", 0,
	 "Función de aptitud a utilizar", 0},

	{"elite", 'e', "PORCENTAJE", 0,
	 "Porcentaje de selección elitista", 0},

	{"cruza", 'x', "PORCENTAJE", 0,
	 "Porcentaje de población para cruza", 0},

	{"mutacion", 't', "PORCENTAJE", 0,
	 "Porcentaje de población para mutación", 0},

	{0, 0, 0, 0, "Otras opciones", 3},

	{"debug", 'd', 0, OPTION_ARG_OPTIONAL,
	 "Cuánta info mostrar", 0},

	{0, 0, 0, 0, "Sobre el programa:", -1},

	{0, 0, 0, 0, 0, 0}
};

/* Estructura para pasar datos de main a parse_opt */
struct args
{
	char *args[11], *entrada;
	unsigned long int *semilla, generaciones, poblacion, poblacion_maxima,
		cantidad_elite, cantidad_a_cruzar, cantidad_a_mutar, corridas;
	void (*faptitud) (struct individuos_s * restrict const individuo,
										char **restrict const operandos,
										const unsigned int *restrict const cantidad_operandos,
										const char *restrict const operadores,
										char *const operacion,
										const unsigned int *restrict const debug,
										const char *restrict const letras);
	unsigned int debug;
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
											"La población máxima debe ser un número entero mayor o igual a 10");

			n_aux = strtoul (arg, NULL, 10);

			/* Verifica posibles errores de conversión */
			if ((errno == ERANGE && n_aux == ULONG_MAX)
					|| (errno != 0 && n_aux == 0))
				argp_failure (state, 1, errno, "Error de strtoul");

			args->poblacion_maxima = n_aux;

			if (args->poblacion_maxima < 10)
				argp_error (state,
										"La población máxima debe ser un número entero mayor o igual a 10");
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

		case 'r':
			for (unsigned int i = 0; i < (unsigned int) strlen (arg); i++)
				if (!isdigit (arg[i]))
					argp_error (state,
											"La cantidad de corridas debe ser entero mayor a 0");

			n_aux = strtoul (arg, NULL, 10);

			/* Verifica posibles errores de conversión */
			if ((errno == ERANGE && n_aux == ULONG_MAX)
					|| (errno != 0 && n_aux == 0))
				argp_failure (state, 1, errno, "Error de strtoul");

			args->corridas = n_aux;

			if (args->poblacion_maxima < 1)
				argp_error (state,
										"La cantidad de corridas debe ser entero mayor a 0");
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
											"Debe elegir entre las funciones de aptitud 1 o 2");

			unsigned int aptitud = atoi (arg);

			if (aptitud < 1 || aptitud > 3)
				argp_error (state,
										"Debe elegir entre las funciones de aptitud 1 o 2");

			if (atoi (arg) == 1)
				args->faptitud = calcular_aptitud1;
			else
				args->faptitud = calcular_aptitud2;
			break;

		case 'e':
			for (unsigned int i = 0; i < (unsigned int) strlen (arg); i++)
				if (!isdigit (arg[i]))
					argp_error (state,
											"El porcentaje de población elitista debe ser un número entero entre 1 y 100");

			n_aux = strtoul (arg, NULL, 10);

			/* Verifica posibles errores de conversión */
			if ((errno == ERANGE && n_aux == ULONG_MAX)
					|| (errno != 0 && n_aux == 0))
				argp_failure (state, 1, errno, "Error de strtoul");

			args->cantidad_elite = n_aux;

			if (args->poblacion == 0)
				argp_error (state,
										"El porcentaje de población elitista debe ser un número entero entre 1 y 100");
			else if (args->poblacion > 100)
				argp_error (state,
										"El porcentaje de población elitista debe ser un número entero entre 1 y 100");
			break;

		case 'x':
			for (unsigned int i = 0; i < (unsigned int) strlen (arg); i++)
				if (!isdigit (arg[i]))
					argp_error (state,
											"El porcentaje para cruza debe ser un número entero entre 1 y 100");

			n_aux = strtoul (arg, NULL, 10);

			/* Verifica posibles errores de conversión */
			if ((errno == ERANGE && n_aux == ULONG_MAX)
					|| (errno != 0 && n_aux == 0))
				argp_failure (state, 1, errno, "Error de strtoul");

			args->cantidad_a_cruzar = n_aux;

			if (args->poblacion == 0)
				argp_error (state,
										"El porcentaje para cruza debe ser un número entero entre 1 y 100");
			else if (args->poblacion > 100)
				argp_error (state,
										"El porcentaje para cruza debe ser un número entero entre 1 y 100");
			break;

		case 't':
			for (unsigned int i = 0; i < (unsigned int) strlen (arg); i++)
				if (!isdigit (arg[i]))
					argp_error (state,
											"El porcentaje para mutación debe ser un número entero entre 1 y 100");

			n_aux = strtoul (arg, NULL, 10);

			/* Verifica posibles errores de conversión */
			if ((errno == ERANGE && n_aux == ULONG_MAX)
					|| (errno != 0 && n_aux == 0))
				argp_failure (state, 1, errno, "Error de strtoul");

			args->cantidad_a_mutar = n_aux;

			if (args->poblacion == 0)
				argp_error (state,
										"El porcentaje para mutación debe ser un número entero entre 1 y 100");
			else if (args->poblacion > 100)
				argp_error (state,
										"El porcentaje para mutación debe ser un número entero entre 1 y 100");
			break;

		case 'd':
			if (args->debug < 4)
				args->debug++;
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
	args.poblacion = 10;
	args.poblacion_maxima = 10000;
	args.entrada = NULL;
	args.semilla = NULL;
	args.cantidad_elite = 0;
	args.cantidad_a_mutar = 0;
	args.cantidad_a_cruzar = 0;
	args.generaciones = 10;
	args.corridas = 1;
	args.faptitud = calcular_aptitud1;
	args.debug = 0;

	static struct argp argp = {
		opciones, parse_opt, 0, doc, 0, 0, "es_AR"
	};
	argp_parse (&argp, argc, argv, 0, 0, &args);

	if ((args.cantidad_elite == 0) && (args.cantidad_a_cruzar == 0)
			&& (args.cantidad_a_mutar == 0))
		{
			args.cantidad_elite = 10;
			args.cantidad_a_cruzar = 80;
			args.cantidad_a_mutar = 10;
		}
	else
		{
			if ((args.cantidad_elite +
					 args.cantidad_a_mutar + args.cantidad_a_cruzar) > 100)
				{
					fprintf (stderr,
									 "La suma de los porcentajes de selección, cruza y mutación no puede superar el 100%%\n");
					exit (EXIT_FAILURE);
				}
			/* Si la suma de los porcentajes no da 100% obliga a completarlos */
			else if ((args.cantidad_elite +
								args.cantidad_a_mutar + args.cantidad_a_cruzar) < 100)
				{
					fprintf (stderr,
									 "Si se especifica un valor para selección, cruza o mutación,\n");
					fprintf (stderr,
									 "también debe especificar un valor para los otros dos y la\n");
					fprintf (stderr, "suma tiene que dar 100\n");
					exit (EXIT_FAILURE);
				}
		}

	char *letras = NULL;
	char **operandos = NULL;
	char *operadores = NULL;
	unsigned int cantidad_operandos = 0;
	char *operacion = NULL;

	int salida = procesar (args.entrada, &letras, &operandos, &operadores,
												 &cantidad_operandos, &operacion);
	/* Todo bien */
	if (salida == 0)
		{
			// calcula la cantidad de permutaciones posibles
			// según la cantidad de letras distintas
			unsigned long int permutaciones = 10;
			for (unsigned int indice = 1;
					 indice < (unsigned int) strlen (letras); indice++)
				permutaciones *= (10 - indice);

			/* si el % representa una población > población máxima,
			 * entonces args.poblacion = args.poblacion_maxima. */
			if (((args.poblacion / 100.) * permutaciones) > args.poblacion_maxima)
				args.poblacion = args.poblacion_maxima;
			else
				args.poblacion = (args.poblacion / 100.) * permutaciones;

			/* Guarda el ancho del individuo para facilitar la visualización */
			unsigned int anchoi = 0;
			for (unsigned int i = args.poblacion; i > 0; i /= 10)
				anchoi++;

			if (args.debug > 0)
				printf ("Permutaciones: %lu\nPoblación: %lu\n",
								permutaciones, args.poblacion);
			struct individuos_s *individuos =
				malloc (args.poblacion * sizeof (struct individuos_s));

			struct individuos_s *elite = NULL;
			if (args.cantidad_elite != 0)
				{
					args.cantidad_elite =
						round (args.poblacion * args.cantidad_elite / 100.);
					if (args.cantidad_elite < 1)
						args.cantidad_elite = 1;
					elite = malloc (args.cantidad_elite * sizeof (struct individuos_s));
					for (unsigned long int i = 0; i < args.cantidad_elite; i++)
						{
							mpz_init (elite[i].aptitud);
							elite[i].letras = malloc (10);
						}
				}

			struct individuos_s *mutados = NULL;
			if (args.cantidad_a_mutar != 0)
				{
					args.cantidad_a_mutar =
						round (args.poblacion * args.cantidad_a_mutar / 100.);
					if (args.cantidad_a_mutar < 1)
						args.cantidad_a_mutar = 1;
					mutados =
						malloc (args.cantidad_a_mutar * sizeof (struct individuos_s));
					for (unsigned long int i = 0; i < args.cantidad_a_mutar; i++)
						{
							mpz_init (mutados[i].aptitud);
							mutados[i].letras = malloc (10);
						}
				}

			struct individuos_s *cruzados = NULL;
			if (args.cantidad_a_cruzar != 0)
				{
					args.cantidad_a_cruzar =
						round (args.poblacion * args.cantidad_a_cruzar / 100.);
					/* La cantidad a cruzar siempre tiene que ser par
					 * porque se toman de a 2 individuos */
					if ((args.cantidad_a_cruzar % 2) != 0)
						{
							/* El restante lo agrego a mutación */
							if (args.cantidad_a_mutar > 0)
								{
									args.cantidad_a_mutar++;
									mutados =
										realloc (mutados,
														 args.cantidad_a_mutar *
														 sizeof (struct individuos_s));
									mpz_init (mutados[args.cantidad_a_mutar - 1].aptitud);
									mutados[args.cantidad_a_mutar - 1].letras = malloc (10);
									args.cantidad_a_cruzar -= 1;
								}
							/* Si no hay mutación se intenta agregar a elite */
							else if (args.cantidad_elite > 0)
								{
									args.cantidad_elite++;
									elite =
										realloc (elite,
														 args.cantidad_elite *
														 sizeof (struct individuos_s));
									mpz_init (elite[args.cantidad_elite - 1].aptitud);
									elite[args.cantidad_elite - 1].letras = malloc (10);
									args.cantidad_a_cruzar -= 1;
								}
							/* Si no hay mutación ni elite queda una cantidad
							 * a cruzar impar y listo */
						}
					cruzados =
						malloc (args.cantidad_a_cruzar * sizeof (struct individuos_s));
					for (unsigned long int i = 0; i < args.cantidad_a_cruzar; i++)
						{
							mpz_init (cruzados[i].aptitud);
							cruzados[i].letras = malloc (10);
						}
				}

			/* Puede que por los rendondeos me quede un individuo sin asignar a
			 * selección, cruza o mutación */
			if ((args.poblacion - args.cantidad_elite - args.cantidad_a_cruzar -
					 args.cantidad_a_mutar) > 0)
				{
					/* Primero intento asignarlo a mutación */
					if (args.cantidad_a_mutar > 0)
						{
							args.cantidad_a_mutar++;
							mutados =
								realloc (mutados,
												 args.cantidad_a_mutar *
												 sizeof (struct individuos_s));
							mpz_init (mutados[args.cantidad_a_mutar - 1].aptitud);
							mutados[args.cantidad_a_mutar - 1].letras = malloc (10);
						}
					/* Si no hay mutación se intenta agregar a elite */
					else if (args.cantidad_elite > 0)
						{
							args.cantidad_elite++;
							elite =
								realloc (elite,
												 args.cantidad_elite * sizeof (struct individuos_s));
							mpz_init (elite[args.cantidad_elite - 1].aptitud);
							elite[args.cantidad_elite - 1].letras = malloc (10);
						}
					/* Si no hay mutación ni elite queda una cantidad
					 * a cruzar impar y listo */
					else
						{
							args.cantidad_a_cruzar++;
							cruzados =
								realloc (cruzados,
												 args.cantidad_a_cruzar *
												 sizeof (struct individuos_s));
							mpz_init (cruzados[args.cantidad_a_cruzar - 1].aptitud);
							cruzados[args.cantidad_a_cruzar - 1].letras = malloc (10);
						}
				}

			if (args.debug > 0)
				{
					printf ("\nElite: %lu\n", args.cantidad_elite);
					printf ("Se cruzan: %lu\n", args.cantidad_a_cruzar);
					printf ("Se mutan: %lu\n", args.cantidad_a_mutar);
				}

			struct h_aptitud *historico_aptitud =
				malloc (args.generaciones * sizeof (struct h_aptitud));

			/* Guarda el estado anterior de la solución */
			struct individuos_s anterior[2];
			anterior[0].letras = malloc (10);
			mpz_init (anterior[0].aptitud);
			anterior[1].letras = malloc (10);
			mpz_init (anterior[1].aptitud);

			struct timespec *comienzo_t, *fin_t, *total_t;
			if (args.debug > 0)
				{
					comienzo_t = malloc (sizeof (struct timespec));
					fin_t = malloc (sizeof (struct timespec));
					total_t = malloc (args.generaciones * sizeof (struct timespec));
				}

			/* ============================================================= */
			/*                    CICLO DE LAS CORRIDAS                      */
			/* ============================================================= */
			for (unsigned long int corrida_n = 0; corrida_n < args.corridas;
					 corrida_n++)
				{
					if (args.debug > 0)
						{
							puts ("\n|||||||||||||||||||||||||||||||||");
							printf ("|       CORRIDA %lu\n", corrida_n + 1);
							puts ("|||||||||||||||||||||||||||||||||");
						}
					/* ============================================================= */
					/*                      POBLACIÓN INICIAL                        */
					/* ============================================================= */
					/* De la segunda corrida en adelante hay que cambiar de semilla
					 * para que los aleatorios no sean iguales */
					if (corrida_n > 0)
						args.semilla = NULL;
					generar_poblacion_inicial (&individuos, letras, &args.poblacion,
																		 args.semilla);
					if (args.debug > 2)
						{
							puts ("\nCálculo de aptitud inicial");
							puts ("--------------------------");
						}
					unsigned long generacion = 0;
					unsigned long solucion = 0;
					unsigned long int *peores = NULL;
					unsigned long int n_peores = 0;
					unsigned long int *peores_m = NULL;
					unsigned long int n_peores_m = 0;
					mpz_t peor_aptitud;
					mpz_init (peor_aptitud);
					/* Calcula aptitud de la población */
					for (unsigned long int i = 0; i < args.poblacion; i++)
						{
							args.faptitud (&individuos[i], operandos,
														 &cantidad_operandos, operadores, operacion,
														 &args.debug, letras);

							/* Se guarda la aptitud del peor individuo */
							if (mpz_cmp (individuos[i].aptitud, peor_aptitud) > 0)
								mpz_set (peor_aptitud, individuos[i].aptitud);

							/* Se guardan las posiciones de los que tienen aptitud -1 */
							if (mpz_cmp_si (individuos[i].aptitud, -1) == 0)
								{
									peores =
										realloc (peores,
														 (n_peores + 1) * sizeof (unsigned long int));
									peores[n_peores] = i;
									n_peores++;
								}

							if (mpz_cmp_ui (individuos[i].aptitud, 0) == 0)
								{
									puts ("\n¡Solución en la población inicial!");
									for (unsigned int j = 0; j < (unsigned int) strlen (letras);
											 j++)
										{
											printf ("%c:", letras[j]);
											for (unsigned int x = 0; x < 10; x++)
												if (letras[j] == individuos[i].letras[x])
													{
														printf ("%u  ", x);
														break;
													}
										}
									mostrar_operacion (&individuos[i], operandos, operadores,
																		 operacion);
									solucion = 1;
									puts ("");
									break;
								}
						}
					/* Se reemplaza la aptitud -1 por 9 (longitud del peor individuo) */
					char *p_a = mpz_get_str (NULL, 10, peor_aptitud);
					for (char *p = p_a; *p; p++)
						*p = '9';
					for (unsigned long int i = 0; i < n_peores; i++)
						mpz_set_str (individuos[peores[i]].aptitud, p_a, 10);

					/* Ordena los individuos por aptitud */
					qsort (individuos, args.poblacion, sizeof (struct individuos_s),
								 individuos_cmp);

					/* Prepara la estructura de la ruleta */
					struct ruleta_s *ruleta =
						malloc (args.poblacion * sizeof (struct ruleta_s));

					/* ============================================================= */
					/*                  CICLO ALGORITMO GENÉTICO                     */
					/* ============================================================= */
					for (; (generacion < args.generaciones) && (solucion == 0);
							 generacion++)
						{
							if (args.debug > 0)
								clock_gettime (CLOCK_PROCESS_CPUTIME_ID, comienzo_t);
							if (args.debug > 1)
								{
									puts ("=================================");
									printf ("       GENERACIÓN %lu\n", generacion + 1);
									puts ("=================================\n");

									/* Muestra ya sea la población inicial o como queda después de aplicar 
									 * selección, cruza y mutación */
									puts ("Población (ordenada por aptitud)");
									puts ("--------------------------------");
									for (unsigned long int i = 0; i < args.poblacion; i++)
										{
											printf ("Individuo[%*lu]:  ", anchoi, i);
											for (unsigned int j = 0;
													 j < (unsigned int) strlen (letras); j++)
												{
													printf ("%c:", letras[j]);
													for (unsigned int x = 0; x < 10; x++)
														if (letras[j] == individuos[i].letras[x])
															{
																printf ("%u  ", x);
																break;
															}
												}
											gmp_printf ("= %Zd\n", individuos[i].aptitud);
										}
								}

							/* ============================================================= */
							/*                          SELECCIÓN                            */
							/* ============================================================= */
							if (args.cantidad_elite > 0)
								{
									seleccion_elitista_con_ranking (&individuos, &elite,
																									&args.poblacion,
																									&args.cantidad_elite, 0,
																									&args.debug);
									/* Muestra los individuos elite */
									if (args.debug > 1)
										{
											puts ("\nIndividuos elite");
											puts ("----------------");
											for (unsigned long int i = 0; i < args.cantidad_elite;
													 i++)
												{
													printf ("Individuo[%*lu]:  ", anchoi, i);
													for (unsigned int j = 0;
															 j < (unsigned int) strlen (letras); j++)
														{
															printf ("%c:", letras[j]);
															for (unsigned int x = 0; x < 10; x++)
																if (letras[j] == elite[i].letras[x])
																	{
																		printf ("%u  ", x);
																		break;
																	}
														}
													gmp_printf ("= %Zd\n", elite[i].aptitud);
												}
										}
								}

							/* Arma la ruleta para cruza y mutación */
							armar_ruleta (individuos, &args.poblacion, ruleta);
							/* ============================================================= */
							/*                             CRUZA                             */
							/* ============================================================= */
							if (args.cantidad_a_cruzar > 0)
								{
									mpz_set_ui (peor_aptitud, 0);
									n_peores = 0;
									if (peores != NULL)
										{
											free (peores);
											peores = NULL;
										}
									if (args.debug > 1)
										{
											puts ("\nIndividuos seleccionados para cruzar");
											printf ("------------------------------------");
										}
									for (unsigned long int i = 0; i < args.cantidad_a_cruzar;
											 i += 2)
										{
											unsigned long int madre =
												seleccion_por_ruleta (&args.poblacion, ruleta);
											unsigned long int padre =
												seleccion_por_ruleta (&args.poblacion, ruleta);

											/* Verifica que no tengan las letras en el mismo orden (sin importar los vacíos) */
											while (iguales (&individuos[padre], &individuos[madre]))
												padre =
													seleccion_por_ruleta (&args.poblacion, ruleta);

											mpz_set (cruzados[i].aptitud,
															 individuos[padre].aptitud);
											memcpy (cruzados[i].letras, individuos[padre].letras,
															10);

											memcpy (cruzados[i + 1].letras,
															individuos[madre].letras, 10);
											mpz_set (cruzados[i + 1].aptitud,
															 individuos[madre].aptitud);

											/* Tiene que mostrar el individuo i y el (i + 1) */
											for (unsigned t = 0; t < 2; t++)
												{
													if (args.debug == 2)
														{
															printf ("\nIndividuo[%*lu]:  ", anchoi, i + t);
															for (unsigned int j = 0;
																	 j < (unsigned int) strlen (letras); j++)
																{
																	printf ("%c:", letras[j]);
																	for (unsigned int x = 0; x < 10; x++)
																		if (letras[j] ==
																				cruzados[i + t].letras[x])
																			{
																				printf ("%u  ", x);
																				break;
																			}
																}
															gmp_printf ("= %Zd", cruzados[i + t].aptitud);
														}
													else if (args.debug == 3)
														gmp_printf ("\nIndividuo[%*lu] aptitud: %Zd ",
																				anchoi, i + t,
																				cruzados[i + t].aptitud);
												}

											/* Copia el estado anterior antes de cruzar */
											memcpy (anterior[0].letras, cruzados[i].letras, 10);
											mpz_set (anterior[0].aptitud, cruzados[i].aptitud);

											/* Copia el estado anterior antes de calcular la aptitud */
											memcpy (anterior[1].letras, cruzados[i].letras, 10);
											mpz_set (anterior[1].aptitud, cruzados[i].aptitud);

											cruza_ciclica (&cruzados[i], &cruzados[i + 1]);

											args.faptitud (&cruzados[i], operandos,
																		 &cantidad_operandos, operadores,
																		 operacion, &args.debug, letras);

											/* Se guarda la aptitud del peor individuo */
											if (mpz_cmp (cruzados[i].aptitud, peor_aptitud) > 0)
												mpz_set (peor_aptitud, cruzados[i].aptitud);

											/* Se guardan las posiciones de los que tienen aptitud -1 */
											if (mpz_cmp_si (cruzados[i].aptitud, -1) == 0)
												{
													peores =
														realloc (peores,
																		 (n_peores +
																			1) * sizeof (unsigned long int));
													peores[n_peores] = i;
													n_peores++;
												}

											args.faptitud (&cruzados[i + 1], operandos,
																		 &cantidad_operandos, operadores,
																		 operacion, &args.debug, letras);

											/* Se guarda la aptitud del peor individuo */
											if (mpz_cmp (cruzados[i + 1].aptitud, peor_aptitud) > 0)
												mpz_set (peor_aptitud, cruzados[i + 1].aptitud);

											/* Se guardan las posiciones de los que tienen aptitud -1 */
											if (mpz_cmp_si (cruzados[i + 1].aptitud, -1) == 0)
												{
													peores =
														realloc (peores,
																		 (n_peores +
																			1) * sizeof (unsigned long int));
													peores[n_peores] = i + 1;
													n_peores++;
												}

											int k = -1;
											if (mpz_cmp_d (cruzados[i].aptitud, 0) == 0)
												k = 0;
											if (mpz_cmp_d (cruzados[i + 1].aptitud, 0) == 0)
												{
													i++;
													k = 1;
												}
											if (k != -1)
												{
													if (args.debug > 0)
														{
															clock_gettime (CLOCK_PROCESS_CPUTIME_ID, fin_t);
															diff_t (comienzo_t, fin_t,
																			&total_t[generacion]);
														}

													if (args.debug > 1)
														puts ("");
													printf
														("\n¡Solución por cruza en la generación %lu",
														 generacion + 1);
													if (args.debug == 0)
														printf (" de la corrida %lu!\n", corrida_n);
													else
														puts ("!");
													for (unsigned int j = 0;
															 j < (unsigned int) strlen (letras); j++)
														{
															printf ("%c:", letras[j]);
															for (unsigned int x = 0; x < 10; x++)
																if (letras[j] == cruzados[i].letras[x])
																	{
																		printf ("%u  ", x);
																		break;
																	}
														}
													mostrar_operacion (&cruzados[i], operandos,
																						 operadores, operacion);

													/* Muestra el estado anterior del individuo solución */
													gmp_printf ("\nEstado anterior (%Zd)\n",
																			anterior[k].aptitud);
													for (unsigned int j = 0;
															 j < (unsigned int) strlen (letras); j++)
														{
															printf ("%c:", letras[j]);
															for (unsigned int x = 0; x < 10; x++)
																if (letras[j] == anterior[k].letras[x])
																	{
																		printf ("%u  ", x);
																		break;
																	}
														}
													mostrar_operacion (&anterior[k], operandos,
																						 operadores, operacion);
													if (args.debug > 0)
														puts ("");
													solucion = 1;
													break;
												}
										}
									if (solucion == 1)
										break;
									if (args.debug > 1)
										{
											if (args.debug == 2)
												puts ("");
											puts ("\nIndividuos después de cruzar");
											puts ("----------------------------");
											for (unsigned long int i = 0;
													 i < args.cantidad_a_cruzar; i++)
												{
													printf ("Individuo[%*lu]:  ", anchoi, i);
													for (unsigned int j = 0;
															 j < (unsigned int) strlen (letras); j++)
														{
															printf ("%c:", letras[j]);
															for (unsigned int x = 0; x < 10; x++)
																if (letras[j] == cruzados[i].letras[x])
																	{
																		printf ("%u  ", x);
																		break;
																	}
														}
													gmp_printf ("= %Zd\n", cruzados[i].aptitud);
												}
										}
								}

							/* ============================================================= */
							/*                          MUTACIÓN                             */
							/* ============================================================= */
							if (args.cantidad_a_mutar > 0)
								{
									peores_m = NULL;
									n_peores_m = 0;
									if (args.debug > 1)
										{
											puts ("\nIndividuos seleccionados para mutar");
											printf ("-----------------------------------");
										}
									/* Muta un % y luego de mutar cada individuo verifica si es solución */
									for (unsigned long int i = 0; i < args.cantidad_a_mutar;
											 i++)
										{
											unsigned long int indice =
												seleccion_por_ruleta (&args.poblacion, ruleta);

											mpz_set (mutados[i].aptitud,
															 individuos[indice].aptitud);
											memcpy (mutados[i].letras, individuos[indice].letras,
															10);

											if (args.debug == 2)
												{
													printf ("\nIndividuo[%*lu]:  ", anchoi, i);
													for (unsigned int j = 0;
															 j < (unsigned int) strlen (letras); j++)
														{
															printf ("%c:", letras[j]);
															for (unsigned int x = 0; x < 10; x++)
																if (letras[j] == mutados[i].letras[x])
																	{
																		printf ("%u  ", x);
																		break;
																	}
														}
													gmp_printf ("= %Zd", mutados[i].aptitud);
												}
											else if (args.debug == 3)
												gmp_printf ("\nIndividuo[%*lu] aptitud: %Zd ", anchoi,
																		i, mutados[i].aptitud);

											/* Copia el estado anterior antes de calcular la aptitud */
											memcpy (anterior[0].letras, mutados[i].letras, 10);
											mpz_set (anterior[0].aptitud, mutados[i].aptitud);

											mutacion (&mutados[i]);

											args.faptitud (&mutados[i], operandos,
																		 &cantidad_operandos, operadores,
																		 operacion, &args.debug, letras);

											/* Se guarda la aptitud del peor individuo */
											if (mpz_cmp (mutados[i].aptitud, peor_aptitud) > 0)
												mpz_set (peor_aptitud, mutados[i].aptitud);

											/* Se guardan las posiciones de los que tienen aptitud -1 */
											if (mpz_cmp_si (mutados[i].aptitud, -1) == 0)
												{
													peores_m =
														realloc (peores_m,
																		 (n_peores_m +
																			1) * sizeof (unsigned long int));
													peores_m[n_peores_m] = i;
													n_peores_m++;
												}

											if (mpz_cmp_d (mutados[i].aptitud, 0) == 0)
												{
													if (args.debug > 0)
														{
															clock_gettime (CLOCK_PROCESS_CPUTIME_ID, fin_t);
															diff_t (comienzo_t, fin_t,
																			&total_t[generacion]);
														}

													if (args.debug > 1)
														puts ("");
													printf
														("\n¡Solución por mutación en la generación %lu",
														 generacion + 1);
													if (args.debug == 0)
														printf (" de la corrida %lu!\n", corrida_n);
													else
														puts ("!");
													for (unsigned int j = 0;
															 j < (unsigned int) strlen (letras); j++)
														{
															printf ("%c:", letras[j]);
															for (unsigned int x = 0; x < 10; x++)
																if (letras[j] == mutados[i].letras[x])
																	{
																		printf ("%u  ", x);
																		break;
																	}
														}
													mostrar_operacion (&mutados[i], operandos,
																						 operadores, operacion);

													/* Muestra el estado anterior del individuo solución */
													gmp_printf ("\nEstado anterior (%Zd)\n",
																			anterior[0].aptitud);
													for (unsigned int j = 0;
															 j < (unsigned int) strlen (letras); j++)
														{
															printf ("%c:", letras[j]);
															for (unsigned int x = 0; x < 10; x++)
																if (letras[j] == anterior[0].letras[x])
																	{
																		printf ("%u  ", x);
																		break;
																	}
														}
													mostrar_operacion (&anterior[0], operandos,
																						 operadores, operacion);
													if (args.debug > 0)
														puts ("");
													solucion = 1;
													break;
												}
										}
									if (solucion == 1)
										break;
									/* Muestra los individuos después de mutarlos */
									if (args.debug > 1)
										{
											if (args.debug == 2)
												puts ("");
											puts ("\nIndividuos después de mutar");
											puts ("---------------------------");
											for (unsigned long int i = 0; i < args.cantidad_a_mutar;
													 i++)
												{
													printf ("Individuo[%*lu]:  ", anchoi, i);
													for (unsigned int j = 0;
															 j < (unsigned int) strlen (letras); j++)
														{
															printf ("%c:", letras[j]);
															for (unsigned int x = 0; x < 10; x++)
																if (letras[j] == mutados[i].letras[x])
																	{
																		printf ("%u  ", x);
																		break;
																	}
														}
													gmp_printf ("= %Zd\n", mutados[i].aptitud);
												}
										}
								}
							/* Se reemplaza la aptitud -1 por 9 (longitud del peor individuo) */
							if (p_a != NULL)
								free (p_a);
							p_a = mpz_get_str (NULL, 10, peor_aptitud);
							for (char *p = p_a; *p; p++)
								*p = '9';
							for (unsigned long int i = 0; i < n_peores_m; i++)
								mpz_set_str (mutados[peores_m[i]].aptitud, p_a, 10);
							for (unsigned long int i = 0; i < n_peores; i++)
								mpz_set_str (cruzados[peores[i]].aptitud, p_a, 10);

							/* Se copian la selección elitista a la nueva generación */
							for (unsigned long int i = 0; i < args.cantidad_elite; i++)
								{
									mpz_set (individuos[i].aptitud, elite[i].aptitud);
									memcpy (individuos[i].letras, elite[i].letras, 10);
								}

							/* Se copian los cruzados a la nueva generación */
							for (unsigned long int i = args.cantidad_elite;
									 i < (args.cantidad_a_cruzar + args.cantidad_elite); i++)
								{
									mpz_set (individuos[i].aptitud,
													 cruzados[i - args.cantidad_elite].aptitud);
									memcpy (individuos[i].letras,
													cruzados[i - args.cantidad_elite].letras, 10);
								}

							/* Se copian los mutados a la nueva generación */
							for (unsigned long int i =
									 args.cantidad_elite + args.cantidad_a_cruzar;
									 i < args.poblacion; i++)
								{
									mpz_set (individuos[i].aptitud,
													 mutados[i - args.cantidad_elite -
																	 args.cantidad_a_cruzar].aptitud);
									memcpy (individuos[i].letras,
													mutados[i - args.cantidad_elite -
																	args.cantidad_a_cruzar].letras, 10);
								}

							/* Ordena los individuos por aptitud */
							qsort (individuos, args.poblacion, sizeof (struct individuos_s),
										 individuos_cmp);

							/* Guada el mejor individuo */
							mpz_init (historico_aptitud[generacion].mejor.aptitud);
							mpz_set (historico_aptitud[generacion].mejor.aptitud,
											 individuos[0].aptitud);

							historico_aptitud[generacion].mejor.letras = malloc (10);
							memcpy (historico_aptitud[generacion].mejor.letras,
											individuos[0].letras, 10);

							/* Guada el peor individuo */
							mpz_init (historico_aptitud[generacion].peor.aptitud);
							mpz_set (historico_aptitud[generacion].peor.aptitud,
											 individuos[args.poblacion - 1].aptitud);

							historico_aptitud[generacion].peor.letras = malloc (10);
							memcpy (historico_aptitud[generacion].peor.letras,
											individuos[args.poblacion - 1].letras, 10);

							/* Guarda la aptitud promedio */
							mpq_t total_aptitud;
							mpq_init (total_aptitud);
							mpq_t media_aptitud;
							mpq_init (media_aptitud);
							mpq_t aptitudes;
							mpq_init (aptitudes);

							mpz_init (historico_aptitud[generacion].media);

							for (unsigned long int i = 0; i < args.poblacion; i++)
								mpz_add (historico_aptitud[generacion].media,
												 historico_aptitud[generacion].media,
												 individuos[i].aptitud);

							mpq_set_z (total_aptitud, historico_aptitud[generacion].media);
							mpq_set_ui (aptitudes, args.poblacion, 1);
							mpq_div (media_aptitud, total_aptitud, aptitudes);

							mpz_set_q (historico_aptitud[generacion].media, media_aptitud);

							mpq_clear (total_aptitud);
							mpq_clear (media_aptitud);
							mpq_clear (aptitudes);
							/* Si encuentra solución no llega hasta acá */
							if (args.debug > 0)
								{
									clock_gettime (CLOCK_PROCESS_CPUTIME_ID, fin_t);
									diff_t (comienzo_t, fin_t, &total_t[generacion]);
								}
						}
					/* Calcula el tiempo total de ejecución de la generación */
					if (args.debug > 0)
						{
							unsigned long int gen =
								(solucion == 1) ? generacion + 1 : generacion;
							unsigned int tiempo = 0;
							for (unsigned long int i = 0; i < gen; i++)
								{
									tiempo +=
										((total_t[i].tv_sec * 1000) +
										 (total_t[i].tv_nsec / 1000000));
									printf
										("Tiempo de ejecución de la generación %lu: %ums\n",
										 i + 1, tiempo);
									tiempo = 0;
								}
						}

					/* Si no encuentra solución imprime como quedó la poblacion final */
					if ((args.debug > 1) && (solucion == 0))
						{
							puts ("\n=================================");
							puts ("        POBLACION FINAL ");
							puts ("=================================\n");

							for (unsigned long int i = 0; i < args.poblacion; i++)
								{
									printf ("Individuo[%*lu]:  ", anchoi, i);
									for (unsigned int j = 0; j < (unsigned int) strlen (letras);
											 j++)
										{
											printf ("%c:", letras[j]);
											for (unsigned int x = 0; x < 10; x++)
												if (letras[j] == individuos[i].letras[x])
													{
														printf ("%u  ", x);
														break;
													}
										}
									gmp_printf ("= %Zd\n", individuos[i].aptitud);
								}
						}
					if (args.debug > 1)
						{
							puts ("\n=================================");
							puts ("     HISTÓRICOS GENERACIONES");
							puts ("=================================\n");

							mpq_t aux;
							mpq_t total_aptitud;

							mpq_init (aux);
							mpq_init (total_aptitud);

							for (unsigned long int i = 0; i < generacion; i++)
								{
									mpq_set_z (aux, historico_aptitud[i].media);
									mpq_add (total_aptitud, total_aptitud, aux);
									gmp_printf
										("Generación %lu, mejor = %Zd, peor = %Zd, promedio = %Zd\n",
										 i + 1, historico_aptitud[i].mejor.aptitud,
										 historico_aptitud[i].peor.aptitud,
										 historico_aptitud[i].media);
								}

							mpq_t aptitudes;
							mpq_init (aptitudes);
							mpq_set_ui (aptitudes, generacion, 1);

							mpq_div (total_aptitud, total_aptitud, aptitudes);

							mpq_clear (aux);
							mpq_clear (aptitudes);

							mpz_t media_global;
							mpz_init (media_global);
							mpz_set_q (media_global, total_aptitud);

							gmp_printf ("Generación promedio total: %Zd\n", media_global);

							mpq_clear (total_aptitud);
							mpz_clear (media_global);
						}
					free (ruleta);

					/* Una vez finalizadas todas las corridas libera la memoria */
					if (corrida_n == args.corridas)
						{
							if (args.semilla != NULL)
								free (args.semilla);

							for (unsigned long int i = 0; i < args.cantidad_elite; i++)
								{
									mpz_clear (elite[i].aptitud);
									free (elite[i].letras);
								}
							if (elite != NULL)
								free (elite);

							for (unsigned long int i = 0; i < args.cantidad_a_cruzar; i++)
								{
									mpz_clear (cruzados[i].aptitud);
									free (cruzados[i].letras);
								}
							if (cruzados != NULL)
								free (cruzados);

							for (unsigned long int i = 0; i < args.cantidad_a_mutar; i++)
								{
									mpz_clear (mutados[i].aptitud);
									free (mutados[i].letras);
								}
							if (mutados != NULL)
								free (mutados);

							for (unsigned long int i = 0; i < args.poblacion; i++)
								{
									free (individuos[i].letras);
									mpz_clear (individuos[i].aptitud);
								}
							free (individuos);

							free (letras);
							while (cantidad_operandos-- > 0)
								free (operandos[cantidad_operandos]);
							free (operandos);
							free (operadores);
							free (operacion);

							for (unsigned long int i = 0; i < generacion; i++)
								{
									free (historico_aptitud[i].mejor.letras);
									free (historico_aptitud[i].peor.letras);

									mpz_clear (historico_aptitud[i].mejor.aptitud);
									mpz_clear (historico_aptitud[i].peor.aptitud);
									mpz_clear (historico_aptitud[i].media);
								}
							free (historico_aptitud);

							/* Se libera todo lo referente a la aptitud -1 */
							if (peores != NULL)
								free (peores);
							if (peores_m != NULL)
								free (peores_m);
							if (p_a != NULL)
								free (p_a);
							mpz_clear (peor_aptitud);
						}
				}

			if (args.debug > 0)
				{
					free (comienzo_t);
					free (fin_t);
					free (total_t);
				}

			/* exit ((solucion == 1) ? EXIT_SUCCESS : EXIT_FAILURE); */
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
