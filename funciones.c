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
					*letras = (char *) malloc (1);
					*letras[0] = '\0';
				}
			if ((strchr (*letras, *operando) == NULL))
				{
					if (strlen (*letras) == 10)
						return 0;
					size_t len = strlen (*letras);
					*letras = (char *) realloc (*letras, len + 1);
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
	*operandos =
		(char **) realloc (*operandos, sizeof (char *) * (*cantidad_operandos));
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
			*operadores = (char *) malloc (1);
			*operadores[0] = '\0';
		}
	else
		*operadores = (char *) realloc (*operadores, *cantidad_operandos + 1);
	strcat (*operadores, operador);
}

void
generar_poblacion_inicial (char *restrict individuos,
													 const char *restrict const letras,
													 const long int *restrict const poblacion)
{
	memset (individuos, '\0', (*poblacion * 11));

	const size_t len = strlen (letras);
	long int n;

	// avanza el puntero 11 posiciones porque ese es el largo de cada fila
	for (n = 0; n++ < *poblacion; individuos += 11)
		memcpy (individuos, letras, len);
	individuos -= (*poblacion * 11);

	n = 0;
	// de http://benpfaff.org/writings/clc/shuffle.html
	srand ((time (0) & 0xFFFF) | (getpid () << 16));
	do
		{
			int columna;
			/* todos los individuos tiene 10 posiciones,
			 * independientemente de la cantidad de letras distintas */
			/* accede a cada posición del individuos como si fuera de 1 dimensión */
			for (columna = 0; columna < 10; columna++)
				{
					const short aleatorio =
						(short) (columna + rand () / (RAND_MAX / (10 - columna) + 1));
					const char caracter = individuos[n * 11 + aleatorio];
					individuos[n * 11 + aleatorio] = individuos[n * 11 + columna];
					individuos[n * 11 + columna] = caracter;
				}
		}
	while (n++ < *poblacion);
}

long int
funcion_de_parada (const char *restrict const individuos,
									 const long int *restrict const poblacion,
									 char **restrict const operandos,
									 const int *restrict const cantidad_operandos,
									 const char *restrict const operaciones)
{
	/* parada = |resultado - (operaciones con operadores)| */
	long int n = 0;
	do
		{
			long long int operandos_numericos[*cantidad_operandos];
			memset (operandos_numericos, 0, sizeof (operandos_numericos));

			convertir_operandos_a_numeros (&individuos[n * 11], operandos,
																		 *cantidad_operandos,
																		 operandos_numericos);

			long long pila_operandos[*cantidad_operandos - 1];
			int tope_pila_operandos = 0;

			char pila_operaciones[*cantidad_operandos - 2];
			int tope_pila_operaciones = 0;

			int k;
			for (k = 0; k < (*cantidad_operandos - 1); k++)
				{
					if (strcmp ("(", operandos[k]) == 0
							|| strcmp (")", operandos[k]) == 0)
						continue;

					pila_operandos[tope_pila_operandos] = operandos_numericos[k];
					tope_pila_operandos++;

					// primero verificar si hay operaciones por cargar
					if (k < (*cantidad_operandos - 2))
						{
							// Antes de guardar la operación hay que verificar si ya hay otra operación en la pila
							// si hay y es de mayor o igual precedencia que la que estoy por guardar hay que hacer
							// la operación antes de guardar la operación actual.
							// Ejemplo: hay una operación multiplicación y voy a guardar una suma. Hay que hacer la
							// multiplicación y luego guardar la suma
							if ((tope_pila_operaciones > 0) &&
									(precedencia (&pila_operaciones[tope_pila_operaciones - 1])
									 >= precedencia (&operaciones[k])))
								{
									switch (pila_operaciones[tope_pila_operaciones - 1])
										{
										case '+':
											pila_operandos[tope_pila_operandos - 2] +=
												pila_operandos[tope_pila_operandos - 1];
											break;
										case '-':
											pila_operandos[tope_pila_operandos - 2] -=
												pila_operandos[tope_pila_operandos - 1];
											break;
										case '*':
											pila_operandos[tope_pila_operandos - 2] *=
												pila_operandos[tope_pila_operandos - 1];
											break;
										case '/':
											if (pila_operandos[tope_pila_operandos - 2] != 0)
												pila_operandos[tope_pila_operandos -
																			 2] /=
													pila_operandos[tope_pila_operandos - 1];
											else
												puts ("División por 0");
											break;
										}
									tope_pila_operandos--;
									tope_pila_operaciones--;
								}
							pila_operaciones[tope_pila_operaciones] = operaciones[k];
							tope_pila_operaciones++;
						}
					// no hay más operaciones, procesar las pilas
					else
						{
							while (tope_pila_operandos > 1)
								{
									switch (pila_operaciones[tope_pila_operaciones - 1])
										{
										case '+':
											pila_operandos[tope_pila_operandos - 2] +=
												pila_operandos[tope_pila_operandos - 1];
											break;
										case '-':
											pila_operandos[tope_pila_operandos - 2] -=
												pila_operandos[tope_pila_operandos - 1];
											break;
										case '*':
											pila_operandos[tope_pila_operandos - 2] *=
												pila_operandos[tope_pila_operandos - 1];
											break;
										case '/':
											if (pila_operandos[tope_pila_operandos - 1] != 0)
												pila_operandos[tope_pila_operandos -
																			 2] /=
													pila_operandos[tope_pila_operandos - 1];
											else
												puts ("División por 0");
											break;
										}
									tope_pila_operandos--;
									tope_pila_operaciones--;
								}
						}
				}


			int diferencia = abs (operandos_numericos[*cantidad_operandos - 1] -
														pila_operandos[0]);
			if (diferencia == 0)
				return n;
		}
	while (++n < *poblacion);
	return -1;
}

void
convertir_operandos_a_numeros (const char *restrict const individuo,
															 char **restrict const operandos,
															 int cantidad_operandos,
															 long long int *restrict const
															 operandos_numericos)
{
	while (cantidad_operandos-- > 0)
		{
			if (strcmp ("(", operandos[cantidad_operandos]) == 0
					|| strcmp (")", operandos[cantidad_operandos]) == 0)
				continue;

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
					int indice;
					for (indice = 0; indice < 10; indice++)
						{
							const char caracter_encontrado = individuo[indice];
							if (caracter_buscado == caracter_encontrado)
								{
									int aux = 0;	// aux incrementa cuántes veces hay que multiplicar por 10
									int numero = indice;
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

int
precedencia (const char *restrict const operacion)
{
	if (*operacion == '*' || *operacion == '/')
		return 2;
	if (*operacion == '+' || *operacion == '-')
		return 1;
	return 0;
}
