
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

#include <stdlib.h>
#include <limits.h>
#include <string.h>

static int
eval_suma (int operando1, int operando2)
{
	return operando1 + operando2;
}

static int
eval_resta (int operando1, int operando2)
{
	return operando1 - operando2;
}

static int
eval_mul (int operando1, int operando2)
{
	return operando1 * operando2;
}

static int
eval_div (int operando1, int operando2)
{
	return operando1 / operando2;
}

enum
{ ASSOC_NONE = 0, ASSOC_LEFT };

static struct op_s
{
	char op;
	int prec;
	int assoc;
	int (*eval) (int operando1, int operando2);
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
	for (int i = 0; i < (int) (sizeof ops / sizeof ops[0]); ++i)
		{
			if (ops[i].op == operador)
				return ops + i;
		}
	return NULL;
}

struct op_s *opstack[64];
int numstack[64];
int nopstack = 0;
int nnumstack = 0;

void
push_opstack (struct op_s *op)
{
	opstack[nopstack++] = op;
}

struct op_s *
pop_opstack ()
{
	return opstack[--nopstack];
}

void
push_numstack (int num)
{
	numstack[nnumstack++] = num;
}

int
pop_numstack ()
{
	return numstack[--nnumstack];
}


int
shunt_op (struct op_s *op)
{
	struct op_s *pop;
	int operando1, operando2;
	if (op->op == '(')
		{
			push_opstack (op);
			return 1;
		}
	else if (op->op == ')')
		{
			while (nopstack > 0 && opstack[nopstack - 1]->op != '(')
				{
					/* Si está por dividir por 0 salir */
					if ((opstack[nopstack - 1]->op == '/')
							&& (numstack[nnumstack - 1] == 0))
						return -1;
					else
						{
							pop = pop_opstack ();
							operando1 = pop_numstack ();
							operando2 = pop_numstack ();
							push_numstack (pop->eval (operando2, operando1));
						}
				}
			/* Saca el '(' */
			pop_opstack ();
			return 1;
		}

	while (nopstack && op->prec <= opstack[nopstack - 1]->prec)
		{
			if ((opstack[nopstack - 1]->op == '/')
					&& (numstack[nnumstack - 1] == 0))
				return -1;
			else
				{
					pop = pop_opstack ();
					operando1 = pop_numstack ();
					operando2 = pop_numstack ();
					push_numstack (pop->eval (operando2, operando1));
				}
		}
	push_opstack (op);
	return 1;
}

long long int
calcular_operacion (const long long int *restrict const operandos_numericos,
										const char *restrict const operadores,
										char *const operacion)
{
	nopstack = 0;
	nnumstack = 0;

	struct op_s *op = NULL;

	int n_operando = 0, n_operador = 0;
	for (char *tipo = operacion; *tipo; tipo++)
		{
			/* operador */
			if (*tipo == '0')
				{
					op = getop (operadores[n_operador]);
					if (shunt_op (op) == -1)
						return LONG_MAX;
					n_operador++;
				}
			/* operando */
			else
				{
					push_numstack (operandos_numericos[n_operando]);
					n_operando++;
				}
		}
	while (nopstack)
		{
			/* Si está por dividir por 0 salir */
			if ((opstack[nopstack - 1]->op == '/')
					&& (numstack[nnumstack - 1] == 0))
				return LONG_MAX;
			else
				{
					op = pop_opstack ();
					int operando1 = pop_numstack ();
					int operando2 = pop_numstack ();
					push_numstack (op->eval (operando2, operando1));
				}
		}

	return numstack[0];
}
