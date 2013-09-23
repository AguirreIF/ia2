#pragma once
#ifndef FUNCIONES_H
#define FUNCIONES_H

#include <inttypes.h>

#define UNUSED(x) (void)(x)

// función principal de proceso en entrada.y
int procesar (char *restrict const entrada, char **restrict letras,
							char ***restrict operandos, char **restrict operadores,
							int *restrict cantidad_operandos, char **restrict operacion);

// convierte la entrada a minúsculas
void a_minusculas (char *restrict operando);

// guarda todos los caracteres distintos en un string dinámico
int procesar_letras (const char *restrict operando,
										 char **restrict const letras);

// guarda el operando
void guardar_operando (const char *restrict const operando,
											 char ***restrict const operandos,
											 int *restrict const cantidad_operandos);

// guarda el operador
void guardar_operador (const char *restrict const operador,
											 char **operadores,
											 const int *restrict const cantidad_operandos);

// genera la población inicial aleatoriamente
void generar_poblacion_inicial (char *restrict individuos,
																const char *restrict const letras,
																const uint32_t *restrict const poblacion);

// calcula la aptitud de un individuo
long int funcion_de_parada (const char *restrict const individuos,
														const uint32_t *restrict const poblacion,
														char **restrict const operandos,
														const int *restrict const cantidad_operandos,
														const char *restrict const operaciones,
														char *const operacion);

// calcula la aptitud de un individuo
int calcular_aptitud (const char *restrict const individuo,
											char **restrict const operandos,
											const int *restrict const cantidad_operandos,
											const char *restrict const operadores,
											char *const operacion);

void convertir_operandos_a_numeros (const char *restrict const individuo,
																		char **restrict const operandos,
																		int cantidad_operandos,
																		long long int *restrict const
																		operandos_numericos);

long long int calcular_operacion (const long long int *restrict const
																	operandos_numericos,
																	const char *restrict const operadores,
																	char *const operacion);

#endif /* FUNCIONES_H */
