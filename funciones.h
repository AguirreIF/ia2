#pragma once
#ifndef FUNCIONES_H
#define FUNCIONES_H

#define UNUSED(x) (void)(x)

// función principal de proceso en entrada.y
int procesar (const char *restrict entrada, char **restrict letras,
							char ***restrict operandos, char **restrict operadores,
							int *restrict cantidad_operandos);

// convierte la entrada a minúsculas
void a_minusculas (char *restrict operando);

// guarda todos los caracteres distintos en un string dinámico
int procesar_letras (char *restrict operando, char **restrict letras);

// guarda el operando
void guardar_operando (const char *restrict operando,
											 char ***restrict operandos,
											 int *restrict cantidad_operandos);

// guarda el operador
void guardar_operador (const char *restrict operador,
											 char **operadores,
											 const int *restrict cantidad_operandos);

// genera la población inicial aleatoriamente
void generar_poblacion_inicial (char *individuos,
																char *restrict letras,
																long int *restrict poblacion);

// calcula la aptitud de un individuo
long int funcion_de_parada (const char *restrict individuos,
														const long int *restrict poblacion,
														char **restrict operandos,
														const int *restrict cantidad_operandos,
														const char *restrict operaciones);

void convertir_operandos_a_numeros (const char *restrict individuo,
																		char **restrict operandos,
																		int cantidad_operandos,
																		int *restrict operandos_numericos);

int precedencia (const char *restrict operacion);

#endif /* FUNCIONES_H */
