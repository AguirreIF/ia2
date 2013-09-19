#pragma once
#ifndef FUNCIONES_H
#define FUNCIONES_H

#define UNUSED(x) (void)(x)

// función principal de proceso en entrada.y
int procesar (const char *restrict const entrada, char **restrict letras,
							char ***restrict operandos, char **restrict operadores,
							int *restrict cantidad_operandos);

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
																const long int *restrict const poblacion);

// calcula la aptitud de un individuo
long int funcion_de_parada (const char *restrict const individuos,
														const long int *restrict const poblacion,
														char **restrict const operandos,
														const int *restrict const cantidad_operandos,
														const char *restrict const operaciones);

// calcula la aptitud de un individuo
int calcular_aptitud (const char *restrict const individuo,
											char **restrict const operandos,
											const int *restrict const cantidad_operandos,
											const char *restrict const operaciones);

void convertir_operandos_a_numeros (const char *restrict const individuo,
																		char **restrict const operandos,
																		int cantidad_operandos,
																		long long int *restrict const
																		operandos_numericos);

int precedencia (const char *restrict const operacion);

#endif /* FUNCIONES_H */
