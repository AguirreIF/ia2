
/* Copyright (c) 2013 the authors listed at the following URL, and/or
the authors of referenced articles or incorporated external code:
http://en.literateprograms.org/Shunting_yard_algorithm_(C)?action=history&offset=20130425200036

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Retrieved from: http://en.literateprograms.org/Shunting_yard_algorithm_(C)?oldid=18970
*/

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "funciones.h"

struct op_s **opstack = NULL;
unsigned int nopstack;

mpq_t *numstack = NULL;
unsigned int nnumstack;

void
eval_suma (mpq_t resultado, mpq_t operando1, mpq_t operando2)
{
	mpq_add (resultado, operando1, operando2);
	mpq_clear (operando1);
	mpq_clear (operando2);
}

void
eval_resta (mpq_t resultado, mpq_t operando1, mpq_t operando2)
{
	mpq_sub (resultado, operando1, operando2);
	mpq_clear (operando1);
	mpq_clear (operando2);
}

void
eval_mul (mpq_t resultado, mpq_t operando1, mpq_t operando2)
{
	mpq_mul (resultado, operando1, operando2);
	mpq_clear (operando1);
	mpq_clear (operando2);
}

void
eval_div (mpq_t resultado, mpq_t operando1, mpq_t operando2)
{
	mpq_div (resultado, operando1, operando2);
	mpq_clear (operando1);
	mpq_clear (operando2);
}

enum
{ ASSOC_NONE = 0, ASSOC_LEFT };

static struct op_s
{
	char op;
	unsigned int prec;
	unsigned int assoc;
	void (*eval) (mpq_t resultado, mpq_t operando1, mpq_t operando2);
} ops[] =
{
	{
	'*', 8, ASSOC_LEFT, eval_mul},
	{
	'/', 8, ASSOC_LEFT, eval_div},
	{
	'+', 5, ASSOC_LEFT, eval_suma},
	{
	'-', 5, ASSOC_LEFT, eval_resta},
	{
	'(', 0, ASSOC_NONE, NULL},
	{
	')', 0, ASSOC_NONE, NULL}
};

static struct op_s *
getop (char operador)
{
	for (unsigned int i = 0; i < (int) (sizeof ops / sizeof ops[0]); ++i)
		{
			if (ops[i].op == operador)
				return ops + i;
		}
	return NULL;
}

void
push_opstack (struct op_s *op)
{
	opstack = realloc (opstack, (nopstack + 1) * sizeof (struct op_s *));
	opstack[nopstack++] = op;
}

struct op_s *
pop_opstack ()
{
	struct op_s *op = opstack[--nopstack];
	opstack = realloc (opstack, nopstack * sizeof (struct op_s *));
	return op;
}

void
push_numstack (mpq_t operando)
{
	numstack = realloc (numstack, (nnumstack + 1) * sizeof (mpq_t));
	mpq_init (numstack[nnumstack]);
	mpq_set (numstack[nnumstack++], operando);
	mpq_clear (operando);
}

void
pop_numstack (mpq_t operando)
{
	mpq_set (operando, numstack[--nnumstack]);
	mpq_clear (numstack[nnumstack]);
	numstack = realloc (numstack, nnumstack * sizeof (mpq_t));
}

int
shunt_op (struct op_s *op)
{
	struct op_s *pop;
	if (op->op == '(')
		{
			push_opstack (op);
			return 1;
		}
	else if (op->op == ')')
		{
			while (nopstack > 0 && opstack[nopstack - 1]->op != '(')
				{
					/* Prepara los operandos */
					mpq_t resultado, operando1, operando2;
					mpq_init (operando1);
					mpq_init (operando2);
					mpq_init (resultado);
					/* Los saca de la pila */
					pop_numstack (operando1);
					pop_numstack (operando2);
					/* Obtiene el operador */
					pop = pop_opstack ();
					/* Verifica que no vaya a dividir por 0
					 * libera la memoria y retorna */
					if (((pop->op == '/') && (mpq_cmp_ui (operando1, 0, 1) == 0)) ||
							((pop->op == '/') && (mpq_cmp (operando2, operando1) < 0)) ||
							((pop->op == '-') && (mpq_cmp (operando2, operando1) < 0)))
						{
							mpq_clear (operando1);
							mpq_clear (operando2);
							mpq_clear (resultado);
							return -1;
						}
					/* Hace el cálculo y lo guarda en resultado */
					pop->eval (resultado, operando2, operando1);
					/* Guarda el resultado en la pila */
					push_numstack (resultado);
				}
			/* Saca el '(' */
			pop_opstack ();
			return 1;
		}

	while (nopstack && op->prec <= opstack[nopstack - 1]->prec)
		{
			/* Prepara los operandos */
			mpq_t resultado, operando1, operando2;
			mpq_init (operando1);
			mpq_init (operando2);
			mpq_init (resultado);
			/* Los saca de la pila */
			pop_numstack (operando1);
			pop_numstack (operando2);
			/* Obtiene el operador */
			pop = pop_opstack ();
			/* Verifica que no vaya a dividir por 0
			 * libera la memoria y retorna */
			if (((pop->op == '/') && (mpq_cmp_ui (operando1, 0, 1) == 0)) ||
					((pop->op == '/') && (mpq_cmp (operando2, operando1) < 0)) ||
					((pop->op == '-') && (mpq_cmp (operando2, operando1) < 0)))
				{
					mpq_clear (operando1);
					mpq_clear (operando2);
					mpq_clear (resultado);
					return -1;
				}
			/* Hace el cálculo y lo guarda en resultado */
			pop->eval (resultado, operando2, operando1);
			/* Guarda el resultado en la pila */
			push_numstack (resultado);
		}
	push_opstack (op);
	return 1;
}

void
calcular_operacion (char **restrict const operandos,
										const struct individuos_s *restrict const individuo,
										const char *restrict const operadores,
										char *const operacion, mpz_t ** resultado_op,
										const unsigned int *restrict const debug)
{
	nopstack = 0;
	nnumstack = 0;

	char *operacion_completa = NULL;
	if (*debug > 1)
		{
			operacion_completa = malloc (1);
			operacion_completa[0] = '\0';
		}

	struct op_s *op = NULL;

	unsigned int n_operando = 0, n_operador = 0;

	for (char *tipo = operacion; *tipo; tipo++)
		{
			/* operador */
			if (*tipo == '0')
				{
					if (*debug > 1)
						{
							operacion_completa =
								realloc (operacion_completa, strlen (operacion_completa) + 2);

							operacion_completa[strlen (operacion_completa) + 1] = '\0';

							operacion_completa[strlen (operacion_completa)] =
								operadores[n_operador];
						}

					op = getop (operadores[n_operador]);
					if (shunt_op (op) == -1)
						return;
					n_operador++;
				}
			/* operando */
			else
				{
					char *operando_str = NULL;

					convertir_operando_a_numeros (individuo, operandos[n_operando],
																				&operando_str);

					const unsigned int longitud_operando =
						(unsigned int) strlen (operandos[n_operando]);

					if (*debug > 1)
						{
							operacion_completa =
								realloc (operacion_completa,
												 strlen (operacion_completa) + longitud_operando + 1);

							strcat (operacion_completa, operando_str);
						}

					mpq_t operando;
					mpq_init (operando);
					mpq_set_str (operando, operando_str, 10);
					free (operando_str);

					push_numstack (operando);
					n_operando++;
				}
		}
	while (nopstack)
		{
			/* Prepara los operandos */
			mpq_t resultado, operando1, operando2;
			mpq_init (operando1);
			mpq_init (operando2);
			mpq_init (resultado);
			/* Los saca de la pila */
			pop_numstack (operando1);
			pop_numstack (operando2);
			/* Obtiene el operador */
			op = pop_opstack ();
			/* Verifica que no vaya a dividir por 0
			 * libera la memoria y retorna */
			if (((op->op == '/') && (mpq_cmp_ui (operando1, 0, 1) == 0)) ||
					((op->op == '/') && (mpq_cmp (operando2, operando1) < 0)) ||
					((op->op == '-') && (mpq_cmp (operando2, operando1) < 0)))
				{
					mpq_clear (operando1);
					mpq_clear (operando2);
					mpq_clear (resultado);
					return;
				}
			/* Hace el cálculo y lo guarda en resultado */
			op->eval (resultado, operando2, operando1);
			/* Guarda el resultado en la pila */
			push_numstack (resultado);
		}
	if (*debug > 1)
		gmp_printf ("\nOperación: %s\n", operacion_completa);
	*resultado_op = malloc (sizeof (mpz_t));
	mpz_init (**resultado_op);
	mpz_set_q (**resultado_op, numstack[0]);
	mpq_clear (numstack[--nnumstack]);
	free (numstack[nnumstack]);
	numstack = NULL;
}
