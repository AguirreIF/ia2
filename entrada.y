%{
	#include <errno.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <unistd.h>
	#include "funciones.h"

	// la declaración hace falta sino da
	// warning: implicit declaration of function ‘yylex’
	extern int yylex ();
	extern FILE *yyin;
	extern int yy_scan_string(const char *entrada);

	void yyerror (char **restrict letras, char ***restrict operandos, char **restrict operadores,
 				  int *restrict cantidad_operandos, const char *restrict const mensaje);
%}

%define api.pure
%defines /* crea el .h, es lo mismo que -d */
%error-verbose
%parse-param { char **letras }
%parse-param { char ***operandos }
%parse-param { char **operadores }
%parse-param { int *cantidad_operandos }

%union {
	char *cadena;
}
%left '-' '+' '*' '/'
%token<cadena> OPERANDO

%%
operacion:
	expresion '=' operando;

expresion:
	operando
	|
	expresion '+' { guardar_operador ("+", operadores, cantidad_operandos); } expresion
	|
	expresion '-' { guardar_operador ("-", operadores, cantidad_operandos); } expresion
	|
	expresion '*' { guardar_operador ("*", operadores, cantidad_operandos); } expresion
	|
	expresion '/' { guardar_operador ("/", operadores, cantidad_operandos); } expresion
	|
	'(' { guardar_operando ("(", operandos, cantidad_operandos); }
	expresion
	')' { guardar_operando (")", operandos, cantidad_operandos); };

operando:
	OPERANDO {
		a_minusculas($OPERANDO);
		if (procesar_letras ($OPERANDO, letras)) {
			guardar_operando ($OPERANDO, operandos, cantidad_operandos);
		}
		else
			return 2;
	};
%%

void yyerror (char **restrict letras, char ***restrict operandos, char **restrict operadores,
			  int *restrict cantidad_operandos, const char *restrict const mensaje) {
	UNUSED(letras);
	UNUSED(operandos);
	UNUSED(operadores);
	UNUSED(cantidad_operandos);
	printf ("Error: %s\n", mensaje);
}

int procesar (const char *restrict const entrada, char **restrict letras,
			  char ***restrict operandos, char **restrict operadores,
			  int *restrict cantidad_operandos) {
	if (entrada == NULL)
		yyin = (FILE *) 0;
	else
		yy_scan_string(entrada);

	return yyparse(letras, operandos, operadores, cantidad_operandos);
}
