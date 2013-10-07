#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "funciones.h"

// verifica que la entrada esté en el rango [a-zA-Z]
// y convierte a minúsculas
void
a_minusculas (char *restrict operando)
{
	do
		{
			// si es mayúscula convierte a minúscula
			// de http://www.catonmat.net/blog/ascii-case-conversion-trick/
			// 223 = 11011111
			*operando ^= ((223 | *operando) == 223) ? 1 << 5 : 0;
		}
	while (*++operando != '\0');
}

// guarda todos los caracteres distintos de cada entrada
unsigned int
procesar_letras (const char *restrict operando, char **restrict const letras)
{
	do
		{
			if (*letras == NULL)
				{
					*letras = malloc (1);
					*letras[0] = '\0';
				}
			if ((strchr (*letras, *operando) == NULL))
				{
					if (strlen (*letras) == 10)
						return 0;
					size_t len = strlen (*letras);
					*letras = realloc (*letras, len + 2);
					(*letras)[len] = *operando;
					(*letras)[len + 1] = '\0';
				}
		}
	while (*++operando != '\0');

	return 1;
}

void
guardar_operando (const char *restrict const operando,
									char ***restrict const operandos,
									unsigned int *restrict const cantidad_operandos)
{
	(*cantidad_operandos)++;
	*operandos = realloc (*operandos, sizeof (char *) * (*cantidad_operandos));
	(*operandos)[*cantidad_operandos - 1] = malloc (strlen (operando) + 1);
	strcpy ((*operandos)[*cantidad_operandos - 1], operando);
}

// guarda el operador
void
guardar_operador (const char *restrict const operador,
									char **restrict operadores,
									const unsigned int *restrict const cantidad_operandos)
{
	if (*operadores == NULL)
		{
			*operadores = malloc (2);
			*operadores[0] = '\0';
		}
	else
		*operadores = realloc (*operadores, *cantidad_operandos + 1);
	strcat (*operadores, operador);
}

void
generar_poblacion_inicial (struct individuos_s **restrict individuos,
													 const char *restrict const letras,
													 const unsigned long int *restrict const poblacion,
													 const unsigned long int *restrict const semilla)
{
	const size_t len = strlen (letras);

	if (semilla != NULL)
		srand (*semilla);
	else
		srand ((time (0) & 0xFFFF) | (getpid () << 16));

	for (unsigned long int n = 0; n < *poblacion; n++)
		{
			mpz_init ((*individuos)[n].aptitud);
			(*individuos)[n].letras = malloc (10);
			memset ((*individuos)[n].letras, '\0', 10);
			memcpy ((*individuos)[n].letras, letras, len);

			for (unsigned int columna = 0; columna < 10; columna++)
				{
					const unsigned int aleatorio = al_azar (columna, 9);
					const char caracter = (*individuos)[n].letras[aleatorio];
					(*individuos)[n].letras[aleatorio] =
						(*individuos)[n].letras[columna];
					(*individuos)[n].letras[columna] = caracter;
				}
		}
}

void
calcular_aptitud1 (struct individuos_s *restrict const individuo,
									 char **restrict const operandos,
									 const unsigned int *restrict const cantidad_operandos,
									 const char *restrict const operadores,
									 char *const operacion,
									 const unsigned int *restrict const debug,
									 const char *restrict const letras)
{
	mpz_t resultado_deseado, *resultado_obtenido = NULL;
	char *operando = NULL;

	/* Calcula el resultado deseado */
	convertir_operando_a_numeros (individuo,
																operandos[*cantidad_operandos - 1],
																&operando);
	mpz_init_set_str (resultado_deseado, operando, 10);
	free (operando);

	if (*debug > 1)
		{
			puts ("");
			for (unsigned int j = 0; j < (unsigned int) strlen (letras); j++)
				{
					char c = letras[j];
					printf ("%c:", c = letras[j]);
					for (unsigned int x = 0; x < 10; x++)
						if (c == individuo->letras[x])
							{
								printf ("%u  ", x);
								break;
							}
				}
		}

	/* Calcula el resultado obtenido */
	calcular_operacion (operandos, individuo, operadores, operacion,
											&resultado_obtenido, debug);
	if (resultado_obtenido == NULL)
		{
			if (*debug > 1)
				puts ("\nIndividuo inválido");
			mpz_set_si (individuo->aptitud, -1);
			mpz_clear (resultado_deseado);
			return;
		}

	/* Se convierte ambos a char y se hacen las comparaciones */
	char *resultado_deseado_str = NULL;
	resultado_deseado_str = mpz_get_str (NULL, 10, resultado_deseado);
	mpz_clear (resultado_deseado);

	char *resultado_obtenido_str = NULL;
	resultado_obtenido_str = mpz_get_str (NULL, 10, *resultado_obtenido);
	mpz_clear (*resultado_obtenido);
	free (resultado_obtenido);

	unsigned int m, t;
	const char *restrict y;
	const char *restrict x;
	if (strlen (resultado_obtenido_str) >= strlen (resultado_deseado_str))
		{
			m = strlen (resultado_deseado_str) - 1;
			x = resultado_deseado_str;
			t = strlen (resultado_obtenido_str) - 1;
			y = resultado_obtenido_str;
		}
	else
		{
			m = strlen (resultado_obtenido_str) - 1;
			x = resultado_obtenido_str;
			t = strlen (resultado_deseado_str) - 1;
			y = resultado_deseado_str;
		}

	if (*debug > 1)
		{
			printf ("Resultado obtenido: %*s\n", t + 1, resultado_obtenido_str);
			printf (" Resultado deseado: %*s\n", t + 1, resultado_deseado_str);
		}

	mpz_t aptitud;
	mpz_init (aptitud);

	mpz_t aptitud1, aptitud2;
	mpz_init (aptitud1);
	mpz_init (aptitud2);

	/* cuántos dígitos de más tiene el resultado más largo en */
	/* comparación con el más corto */
	unsigned const int sobra = t - m;

	/* primera parte de la fórmula, */
	/* va de atrás para adelante porque la unidad está en la última */
	/* posición del array */
	for (int i = m; i > -1; i--)
		mpz_add_ui (aptitud1, aptitud1,
								(abs (((int) x[i] - '0') - ((int) y[i + sobra] - '0')) * 10));
	if (*debug > 1)
		gmp_printf ("Aptitud: %Zd + ", aptitud1);

	/* segunda parte de la fórmula, */
	/* va de adelante para atrás porque la unidad más grande está en */
	/* la primera posición del array */
	for (unsigned int i = 0; i < sobra; i++)
		mpz_add_ui (aptitud2, aptitud2,
								(((int) y[i] - '0') * ((int) pow (10, (sobra - i + 1)))));
	if (*debug > 1)
		gmp_printf ("%Zd\n", aptitud2);

	mpz_add (aptitud, aptitud1, aptitud2);

	mpz_set (individuo->aptitud, aptitud);

	mpz_clear (aptitud);
	mpz_clear (aptitud1);
	mpz_clear (aptitud2);

	free (resultado_deseado_str);
	free (resultado_obtenido_str);
}

void
calcular_aptitud2 (struct individuos_s *restrict const individuo,
									 char **restrict const operandos,
									 const unsigned int *restrict const cantidad_operandos,
									 const char *restrict const operadores,
									 char *const operacion,
									 const unsigned int *restrict const debug,
									 const char *restrict const letras)
{
	mpz_t resultado_deseado, *resultado_obtenido = NULL;
	char *operando = NULL;

	/* Calcula el resultado deseado */
	convertir_operando_a_numeros (individuo,
																operandos[*cantidad_operandos - 1],
																&operando);
	mpz_init_set_str (resultado_deseado, operando, 10);
	free (operando);

	if (*debug > 1)
		{
			puts ("");
			for (unsigned int j = 0; j < (unsigned int) strlen (letras); j++)
				{
					char c = letras[j];
					printf ("%c:", c);
					for (unsigned int x = 0; x < 10; x++)
						if (c == individuo->letras[x])
							{
								printf ("%u  ", x);
								break;
							}
				}
		}

	/* Calcula el resultado obtenido */
	calcular_operacion (operandos, individuo, operadores, operacion,
											&resultado_obtenido, debug);
	if (resultado_obtenido == NULL)
		{
			if (*debug > 1)
				puts ("\nIndividuo inválido");
			mpz_set_si (individuo->aptitud, -1);
			mpz_clear (resultado_deseado);
			return;
		}

	/* Se convierte ambos a char y se hacen las comparaciones */
	char *resultado_deseado_str = NULL;
	resultado_deseado_str = mpz_get_str (NULL, 10, resultado_deseado);
	mpz_clear (resultado_deseado);

	char *resultado_obtenido_str = NULL;
	resultado_obtenido_str = mpz_get_str (NULL, 10, *resultado_obtenido);
	mpz_clear (*resultado_obtenido);
	free (resultado_obtenido);

	unsigned int m, t;
	const char *restrict y;
	const char *restrict x;
	if (strlen (resultado_obtenido_str) >= strlen (resultado_deseado_str))
		{
			m = strlen (resultado_deseado_str) - 1;
			x = resultado_deseado_str;
			t = strlen (resultado_obtenido_str) - 1;
			y = resultado_obtenido_str;
		}
	else
		{
			m = strlen (resultado_obtenido_str) - 1;
			x = resultado_obtenido_str;
			t = strlen (resultado_deseado_str) - 1;
			y = resultado_deseado_str;
		}

	if (*debug > 1)
		{
			printf ("Resultado obtenido: %*s\n", t + 1, resultado_obtenido_str);
			printf (" Resultado deseado: %*s\n", t + 1, resultado_deseado_str);
		}

	mpz_t aptitud;
	mpz_init (aptitud);

	mpz_t aptitud1, aptitud2;
	mpz_init (aptitud1);
	mpz_init (aptitud2);

	/* cuántos dígitos de más tiene el resultado más largo en */
	/* comparación con el más corto */
	unsigned const int sobra = t - m;

	/* primera parte de la fórmula, */
	/* va de atrás para adelante porque la unidad está en la última */
	/* posición del array */
	unsigned int j = 0;
	for (int i = m; i > -1; i--)
		/* multiplicar por (i+1) en vez de por 10, empezando con i = 0 */
		mpz_add_ui (aptitud1, aptitud1,
								(abs (((int) x[i] - '0') - ((int) y[i + sobra] - '0')) *
								 ((j++) + 1)));
	if (*debug > 1)
		gmp_printf ("Aptitud: %Zd + ", aptitud1);

	/* segunda parte de la fórmula, */
	/* va de adelante para atrás porque la unidad más grande está en */
	/* la primera posición del array */
	for (unsigned int i = 0; i < sobra; i++)
		mpz_add_ui (aptitud2, aptitud2,
								(((int) y[i] - '0') * ((int) pow (10, (sobra - i + 1)))));
	if (*debug > 1)
		gmp_printf ("%Zd\n", aptitud2);

	mpz_add (aptitud, aptitud1, aptitud2);

	mpz_set (individuo->aptitud, aptitud);

	mpz_clear (aptitud);
	mpz_clear (aptitud1);
	mpz_clear (aptitud2);

	free (resultado_obtenido_str);
	free (resultado_deseado_str);
}

void
calcular_aptitud3 (struct individuos_s *restrict const individuo,
									 char **restrict const operandos,
									 const unsigned int *restrict const cantidad_operandos,
									 const char *restrict const operadores,
									 char *const operacion,
									 const unsigned int *restrict const debug,
									 const char *restrict const letras)
{
	mpz_t resultado_deseado, *resultado_obtenido = NULL;
	char *operando = NULL;

	/* Calcula el resultado deseado */
	convertir_operando_a_numeros (individuo,
																operandos[*cantidad_operandos - 1],
																&operando);

	mpz_init_set_str (resultado_deseado, operando, 10);

	if (*debug > 1)
		{
			puts ("");
			for (unsigned int j = 0; j < (unsigned int) strlen (letras); j++)
				{
					char c = letras[j];
					printf ("%c:", c);
					for (unsigned int x = 0; x < 10; x++)
						if (c == individuo->letras[x])
							{
								printf ("%u  ", x);
								break;
							}
				}
		}

	/* Calcula el resultado obtenido */
	calcular_operacion (operandos, individuo, operadores, operacion,
											&resultado_obtenido, debug);
	if (resultado_obtenido == NULL)
		{
			if (*debug > 1)
				puts ("\nIndividuo inválido");
			mpz_set_si (individuo->aptitud, -1);
			return;
		}

	char *resultado_obtenido_str = mpz_get_str (NULL, 10, *resultado_obtenido);
	unsigned int ancho;
	ancho =
		(strlen (resultado_obtenido_str) >
		 strlen (operando) ? strlen (resultado_obtenido_str) : strlen (operando));
	if (*debug > 1)
		{
			printf ("Resultado obtenido: %*s\n", ancho, resultado_obtenido_str);
			printf (" Resultado deseado: %s\n", operando);
		}
	free (resultado_obtenido_str);
	free (operando);

	/* Calcula la aptitud */
	mpz_t aptitud;
	mpz_init (aptitud);
	mpz_sub (aptitud, resultado_deseado, *resultado_obtenido);
	mpz_abs (aptitud, aptitud);

	if (*debug > 1)
		{
			char *aptitud_str = mpz_get_str (NULL, 10, aptitud);
			printf ("           Aptitud: %*s\n", ancho, aptitud_str);
			free (aptitud_str);
		}

	mpz_set (individuo->aptitud, aptitud);

	/* Libera memoria */
	mpz_clear (resultado_deseado);
	mpz_clear (*resultado_obtenido);
	free (resultado_obtenido);
	mpz_clear (aptitud);
}

void
convertir_operando_a_numeros (const struct individuos_s *restrict const
															individuo,
															const char *restrict const operando_str,
															char **restrict operando)
{
	unsigned int columna = 0;
	const unsigned int longitud_operando = (int) strlen (operando_str);
	*operando = malloc (longitud_operando + 1);
	// recorre todos los caracteres del operando
	do
		// recorre todos los caracteres del individuo hasta encontrar el valor que corresponda
		// con el caracter seleccionado del operando
		for (unsigned int indice = 0; indice < 10; indice++)
			if (operando_str[columna] == individuo->letras[indice])
				(*operando)[columna] = '0' + indice;
	while (++columna < longitud_operando);
	(*operando)[columna] = '\0';
}

/* Función de comparación de estructuras de individuos */
int
individuos_cmp (const void *const ptr1, const void *const ptr2)
{
	const struct individuos_s *restrict const individuo1 = ptr1;
	const struct individuos_s *restrict const individuo2 = ptr2;
	if (mpz_sgn (individuo1->aptitud) == -1)
		return 1;
	if (mpz_sgn (individuo2->aptitud) == -1)
		return -1;
	else
		return mpz_cmp (individuo1->aptitud, individuo2->aptitud);
}

void
seleccion_elitista (struct individuos_s **restrict individuos,
										const unsigned long int *restrict const cantidad_elite,
										struct individuos_s **restrict elite)
{
	for (unsigned long int indice = 0; indice < *cantidad_elite; indice++)
		{
			mpz_init ((*elite)[indice].aptitud);
			mpz_set ((*elite)[indice].aptitud, (*individuos)[indice].aptitud);

			(*elite)[indice].letras = malloc (10);
			memcpy ((*elite)[indice].letras, (*individuos)[indice].letras, 10);
		}
}

void
seleccion_por_ranking (struct individuos_s **individuos,
											 const unsigned long int *restrict const inicio,
											 const unsigned long int *restrict const
											 cantidad, const float rmin,
											 const unsigned int *restrict const debug)
{
	struct individuos_s *seleccionados =
		malloc (*cantidad * sizeof (struct individuos_s)), **extras = NULL;

	unsigned long int copias_totales = 0, indice_nuevos = 0, copias_extras = 0;

	for (unsigned long int indice = 0;
			 (indice < *cantidad) && (copias_totales < *cantidad); indice++)
		{
			float copias;
			unsigned int copias_e, copias_d = 0;

			if (*cantidad == 1)
				copias_e = 1;
			else
				{
					/* Total de copias con decimales */
					copias =
						rmin +
						2. * (((*cantidad - indice) * (1. - rmin)) / (*cantidad - 1));
					/* Copias por la parte entera */
					copias_e = copias;
					/* Copias por la parte decimal */
					copias_d = round (copias) - copias_e;
				}

			/* Se hace la copia por la parte decimal si corresponde */
			if (copias_d > 0)
				{
					extras =
						realloc (extras,
										 (copias_extras + 1) * sizeof (struct individuos_s *));

					extras[copias_extras] = malloc (sizeof (struct individuos_s));

					mpz_init (extras[copias_extras]->aptitud);
					mpz_set (extras[copias_extras]->aptitud,
									 (*individuos)[indice].aptitud);

					extras[copias_extras]->letras = malloc (10);
					memcpy (extras[copias_extras]->letras,
									(*individuos)[indice].letras, 10);

					copias_extras++;
				}

			if (*debug > 1)
				printf ("individuo %lu: %u + %u copias\n", indice + 1, copias_e,
								copias_d);

			/* Verifica que copias_e no supere a cantidad.
			 * Puede pasar si cantidad es muy chica */
			if ((copias_e + copias_totales) >= *cantidad)
				copias_e = *cantidad - copias_totales;

			/* Hace la cantidad de copias correspondientes */
			for (unsigned int n = 0; n < copias_e; n++, indice_nuevos++)
				{
					mpz_init (seleccionados[indice_nuevos].aptitud);
					mpz_set (seleccionados[indice_nuevos].aptitud,
									 (*individuos)[indice].aptitud);

					seleccionados[indice_nuevos].letras = malloc (10);
					memcpy (seleccionados[indice_nuevos].letras,
									(*individuos)[indice].letras, 10);
				}
			copias_totales += copias_e;
		}

	/* Si no alcanzaron las copias de la parte entera 
	 * copia de la parte decimal hasta completar la cantidad */
	for (unsigned int n = 0; copias_totales < *cantidad; n++, indice_nuevos++)
		{
			mpz_init (seleccionados[indice_nuevos].aptitud);
			mpz_set (seleccionados[indice_nuevos].aptitud, extras[n]->aptitud);

			seleccionados[indice_nuevos].letras = malloc (10);
			memcpy (seleccionados[indice_nuevos].letras, extras[n]->letras, 10);

			copias_totales++;
		}

	/* Libera todo la memoria reservada para las copias extras */
	if (extras != NULL)
		{
			for (unsigned int n = 0; n < copias_extras; n++)
				{
					mpz_clear (extras[n]->aptitud);
					free (extras[n]->letras);
					free (extras[n]);
				}
			free (extras);
		}

	if (*debug > 1)
		printf ("\nCopias totales: %lu\n", copias_totales);

	/* Se copian los individuos seleccionados a la estructura original */
	for (unsigned long int indice = 0; indice < *cantidad; indice++)
		{
			mpz_set ((*individuos)[indice + *inicio].aptitud,
							 seleccionados[indice].aptitud);

			memcpy ((*individuos)[indice + *inicio].letras,
							seleccionados[indice].letras, 10);

			mpz_clear (seleccionados[indice].aptitud);
			free (seleccionados[indice].letras);
		}

	if (*debug > 1)
		puts ("");

	free (seleccionados);
}

void
cruza_ciclica (struct individuos_s *restrict madre,
							 struct individuos_s *restrict padre)
{
	UNUSED(padre);
	unsigned int longitud_aptitud = mpz_sizeinbase (madre->aptitud, 10);

	while (longitud_aptitud-- > 0)
		{
			unsigned int gen1, gen2;

			gen1 = al_azar (0, 9);
			gen2 = al_azar (0, 9);

			while ((gen1 == gen2) || ((madre->letras[gen1] == '\0') &&
																(madre->letras[gen2] == '\0')))
				gen2 = al_azar (0, 9);

			char aux = madre->letras[gen1];
			madre->letras[gen1] = madre->letras[gen2];
			madre->letras[gen2] = aux;
		}
}

void
mutacion (struct individuos_s *restrict individuo)
{
	unsigned int gen1, gen2;

	gen1 = al_azar (0, 9);
	gen2 = al_azar (0, 9);

	while ((gen1 == gen2) || ((individuo->letras[gen1] == '\0') &&
														(individuo->letras[gen2] == '\0')))
		gen2 = al_azar (0, 9);

	char aux = individuo->letras[gen1];
	individuo->letras[gen1] = individuo->letras[gen2];
	individuo->letras[gen2] = aux;
}

unsigned int
al_azar (const unsigned int min, const unsigned int max)
{
	return (min + rand () / (RAND_MAX / (max - min + 1) + 1));
}

void
mostrar_operacion (const struct individuos_s *restrict const
									 individuo, char **restrict const operandos,
									 const char *restrict const operadores,
									 char *restrict operacion)
{
	unsigned int n_operando = 0, n_operador = 0;

	char *operacion_completa = NULL;
	operacion_completa = malloc (1);
	operacion_completa[0] = '\0';

	do
		{
			/* operador */
			if (*operacion == '0')
				{
					operacion_completa =
						realloc (operacion_completa, strlen (operacion_completa) + 2);

					operacion_completa[strlen (operacion_completa) + 1] = '\0';

					operacion_completa[strlen (operacion_completa)] =
						operadores[n_operador];

					n_operador++;
				}
			/* operando */
			else
				{
					/* Convierte el operando en su representación numérica */
					unsigned int columna = 0;
					const unsigned int longitud_operando =
						(int) strlen (operandos[n_operando]);
					char *operando_str = malloc (longitud_operando + 1);
					// recorre todos los caracteres del operando
					do
						{
							const char caracter_buscado = operandos[n_operando][columna];
							// recorre todos los caracteres del individuo hasta encontrar el valor que corresponda
							// con el caracter seleccionado del operando
							for (unsigned int indice = 0; indice < 10; indice++)
								if (caracter_buscado == individuo->letras[indice])
									operando_str[columna] = '0' + indice;
						}
					while (++columna < longitud_operando);
					operando_str[columna] = '\0';

					if (*operacion == '\0')
						{
							operacion_completa =
								realloc (operacion_completa,
												 strlen (operacion_completa) + longitud_operando + 4);
							strcat (operacion_completa, " = ");
						}
					else
						operacion_completa =
							realloc (operacion_completa,
											 strlen (operacion_completa) + longitud_operando + 1);

					strcat (operacion_completa, operando_str);
					free (operando_str);
					n_operando++;
				}
		}
	while (*operacion++);

	printf ("\nOperación: %s\n", operacion_completa);
	free (operacion_completa);
}
