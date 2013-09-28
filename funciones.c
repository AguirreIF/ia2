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
int
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
									int *restrict const cantidad_operandos)
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
									const int *restrict const cantidad_operandos)
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
													 const uint32_t * restrict const poblacion,
													 const long *restrict const semilla)
{
	const size_t len = strlen (letras);

	if (*semilla != -1)
		srand (*semilla);
	else
		srand ((time (0) & 0xFFFF) | (getpid () << 16));

	for (uint32_t n = 0; n < *poblacion; n++)
		{
			(*individuos)[n].aptitud = 0;
			(*individuos)[n].letras = malloc (10);
			memset ((*individuos)[n].letras, '\0', 10);
			memcpy ((*individuos)[n].letras, letras, len);

			for (int columna = 0; columna < 10; columna++)
				{
					const short aleatorio =
						(short) (columna + rand () / (RAND_MAX / (10 - columna) + 1));
					const char caracter = (*individuos)[n].letras[aleatorio];
					(*individuos)[n].letras[aleatorio] =
						(*individuos)[n].letras[columna];
					(*individuos)[n].letras[columna] = caracter;
				}
		}
}

long int
funcion_de_parada (const struct individuos_s *restrict const individuos,
									 const uint32_t * restrict const poblacion)
{
	for (long int n = 0; n < *poblacion; n++)
		if (individuos[n].aptitud == 0)
			return n;
	return -1;
}

void
calcular_aptitud (struct individuos_s *restrict const individuo,
									char **restrict const operandos,
									const int *restrict const cantidad_operandos,
									const char *restrict const operadores,
									char *const operacion)
{
	long long int operandos_numericos[*cantidad_operandos];
	memset (operandos_numericos, 0, sizeof (operandos_numericos));

	convertir_operandos_a_numeros (individuo, operandos,
																 *cantidad_operandos, operandos_numericos);

	long long int resultado =
		calcular_operacion (operandos_numericos, operadores, operacion);

	/* resultado deseado = operandos_numericos[*cantidad_operandos - 1] (en formato numérico) */
	/* resultado obtenido = pila_operandos[0] (en formato numérico) */
	/* Se convierte ambos a char y se hacen las comparaciones */
	char resultado_deseado[30];
	sprintf (resultado_deseado, "%lld",
					 operandos_numericos[*cantidad_operandos - 1]);

	char resultado_obtenido[30];
	sprintf (resultado_obtenido, "%lld", resultado);

	int m, t;
	const char *restrict y;
	const char *restrict x;
	if (strlen (resultado_obtenido) >= strlen (resultado_deseado))
		{
			m = strlen (resultado_deseado) - 1;
			x = resultado_deseado;
			t = strlen (resultado_obtenido) - 1;
			y = resultado_obtenido;
		}
	else
		{
			m = strlen (resultado_obtenido) - 1;
			x = resultado_obtenido;
			t = strlen (resultado_deseado) - 1;
			y = resultado_deseado;
		}

	int aptitud = 0, i;

	int sobra = t - m;
	/* la variable sobra tiene cuántos dígitos de más tiene el resultado más largo en */
	/* comparación con el más corto */

	/* primera parte de la fórmula, */
	/* va de atrás para adelante porque la unidad está en la última */
	/* posición del array */
	for (i = m; i > -1; i--)
		aptitud += (abs (((int) x[i] - '0') - ((int) y[i + sobra] - '0')) * 10);

	/* segunda parte de la fórmula, */
	/* va de adelante para atrás porque la unidad más grande está en */
	/* la primera posición del array */
	for (i = 0; i < sobra; i++)
		aptitud += (((int) y[i] - '0') * ((int) pow (10, (sobra - i + 1))));

	individuo->aptitud = aptitud;
}

void
convertir_operandos_a_numeros (const struct individuos_s *restrict const
															 individuo, char **restrict const operandos,
															 int cantidad_operandos,
															 long long int *restrict const
															 operandos_numericos)
{
	while (cantidad_operandos-- > 0)
		{
			int columna = 0;
			const int longitud_operando =
				(int) strlen (operandos[cantidad_operandos]);
			// recorre todos los caracteres del operando
			do
				{
					const char caracter_buscado =
						operandos[cantidad_operandos][columna];
					// recorre todos los caracteres del individuo hasta encontrar el valor que corresponda
					// con el caracter seleccionado del operando
					for (int indice = 0; indice < 10; indice++)
						{
							const char caracter_encontrado = individuo->letras[indice];
							if (caracter_buscado == caracter_encontrado)
								{
									int aux = 0;	// aux incrementa cuántes veces hay que multiplicar por 10
									long long int numero = indice;
									if (indice > 0)
										while (++aux < longitud_operando - columna)
											numero = (numero << 3) + (numero << 1);	// x*10 => x*[(2^3+2)]
									operandos_numericos[cantidad_operandos] += numero;
									break;
								}
						}
				}
			while (++columna < longitud_operando);
		}
}

/* Función de comparación de estructuras de individuos */
int
individuos_cmp (const void *const ptr1, const void *const ptr2)
{
	const struct individuos_s *restrict const individuo1 = ptr1;
	const struct individuos_s *restrict const individuo2 = ptr2;
	if (individuo1->aptitud < individuo2->aptitud)
		return -1;
	else if (individuo1->aptitud > individuo2->aptitud)
		return 1;
	else
		return 0;
}

void
seleccion_por_ranking_con_ce (struct individuos_s **individuos,
															const uint32_t * restrict const inicio,
															const uint32_t * restrict const cantidad,
															const float rmin)
{
	struct individuos_s *seleccionados =
		malloc (*cantidad * sizeof (struct individuos_s));

	uint32_t copias_totales = 0, indice_nuevos = 0;

	for (uint32_t indice = 0;
			 (indice < *cantidad) && (copias_totales < *cantidad); indice++)
		{
			short copias_por_individuo;
			if (*cantidad == 1)
				copias_por_individuo = 1;
			else
				copias_por_individuo =
					(round) (rmin +
									 2. * (((*cantidad - indice) * (1. - rmin)) /
												 (*cantidad - 1)));

			/* Verifica que copias_por_individuo no supere a cantidad.
			 * Puede pasar si cantidad es muy chica */
			if ((copias_por_individuo + copias_totales) >= *cantidad)
				copias_por_individuo = *cantidad - copias_totales;

			/* Hace la cantidad de copias correspondientes */
			for (short n = 0; n < copias_por_individuo; n++, indice_nuevos++)
				{
					seleccionados[indice_nuevos].aptitud =
						(*individuos)[indice].aptitud;

					seleccionados[indice_nuevos].letras = malloc (10);
					memcpy (seleccionados[indice_nuevos].letras,
									(*individuos)[indice].letras, 10);
				}
			copias_totales += copias_por_individuo;
		}

	/* Se copian los individuos seleccionados a la estructura original */
	for (uint32_t indice = 0; indice < *cantidad; indice++)
		{
			(*individuos)[indice + *inicio].aptitud = seleccionados[indice].aptitud;
			memcpy ((*individuos)[indice + *inicio].letras,
							seleccionados[indice].letras, 10);
			free (seleccionados[indice].letras);
		}

	free (seleccionados);
}

void
cruza (struct individuos_s *restrict individuos,
			 const uint32_t * restrict const cantidad, const unsigned short puntos)
{
	unsigned short d;
	switch (puntos)
		{
		case 1:
			d = 5;
			break;
		case 2:
			d = 4;
			break;
		case 3:
			d = 3;
			break;
		case 4:
			d = 2;
			break;
		}

	for (uint32_t i = 0; i < *cantidad; i++)
		{
			/* Si el siguiente es un copia no se cruza */
			if ((i + 1) < *cantidad)
				if (individuos[i].letras[0] == individuos[i + 1].letras[0] &&
						individuos[i].letras[1] == individuos[i + 1].letras[1] &&
						individuos[i].letras[2] == individuos[i + 1].letras[2] &&
						individuos[i].letras[3] == individuos[i + 1].letras[3] &&
						individuos[i].letras[4] == individuos[i + 1].letras[4] &&
						individuos[i].letras[5] == individuos[i + 1].letras[5] &&
						individuos[i].letras[6] == individuos[i + 1].letras[6] &&
						individuos[i].letras[7] == individuos[i + 1].letras[7] &&
						individuos[i].letras[8] == individuos[i + 1].letras[8] &&
						individuos[i].letras[9] == individuos[i + 1].letras[9])
					continue;

			unsigned short j = 0;
			for (short x = 0; x < puntos; x++)
				if ((10 - j) >= (d * 2))
					{
						for (short a = 0; a < d; a++, j++)
							{
								char letra = individuos[i].letras[j];
								individuos[i].letras[j] = individuos[i].letras[j + d];
								individuos[i].letras[j + d] = letra;
							}
						j += d;
					}
		}
}
