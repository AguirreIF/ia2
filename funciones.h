#pragma once
#ifndef FUNCIONES_H
#define FUNCIONES_H

#include <gmp.h>
#include <inttypes.h>

#define UNUSED(x) (void)(x)

struct individuos_s
{
	char *letras;
	mpz_t aptitud;
};

// función principal de proceso en entrada.y
int procesar (char *restrict const entrada, char **restrict letras,
							char ***restrict operandos, char **restrict operadores,
							unsigned int *restrict cantidad_operandos,
							char **restrict operacion);

// convierte la entrada a minúsculas
void a_minusculas (char *restrict operando);

// guarda todos los caracteres distintos en un string dinámico
unsigned int procesar_letras (const char *restrict operando,
															char **restrict const letras);

// guarda el operando
void guardar_operando (const char *restrict const operando,
											 char ***restrict const operandos,
											 unsigned int *restrict const cantidad_operandos);

// guarda el operador
void guardar_operador (const char *restrict const operador,
											 char **operadores,
											 const unsigned int *restrict const cantidad_operandos);

// genera la población inicial aleatoriamente
void generar_poblacion_inicial (struct individuos_s **restrict individuos,
																const char *restrict const letras,
																const unsigned long int *restrict const
																poblacion,
																const unsigned long int *restrict const
																semilla);

// calcula la aptitud de un individuo
void calcular_aptitud1 (struct individuos_s *restrict const individuo,
												char **restrict const operandos,
												const unsigned int *restrict const cantidad_operandos,
												const char *restrict const operadores,
												char *const operacion);

// otra función de aptitud
void calcular_aptitud2 (struct individuos_s *restrict const individuo,
												char **restrict const operandos,
												const unsigned int *restrict const cantidad_operandos,
												const char *restrict const operadores,
												char *const operacion);

// otra función de aptitud
void calcular_aptitud3 (struct individuos_s *restrict const individuo,
												char **restrict const operandos,
												const unsigned int *restrict const cantidad_operandos,
												const char *restrict const operadores,
												char *const operacion);

void convertir_operando_a_numeros (const struct individuos_s *restrict const
																	 individuo,
																	 const char *restrict const operando_str,
																	 char **restrict operando);

void calcular_operacion (char **restrict const operandos,
												 const struct individuos_s *restrict const
												 individuo, const char *restrict const operadores,
												 char *const operacion, mpz_t ** resultado);

int individuos_cmp (const void *ptr1, const void *ptr2);

void seleccion_por_ranking (struct individuos_s **restrict individuos,
														const unsigned long int *restrict const
														inicio,
														const unsigned long int *restrict const
														cantidad, const float rmin);

void cruza (struct individuos_s *restrict individuos,
						const unsigned long int *restrict const cantidad);

#endif /* FUNCIONES_H */
