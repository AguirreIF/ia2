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
 				  unsigned int *restrict cantidad_operandos, char **restrict operacion, const char *restrict const mensaje);
	char *aux = NULL;
%}

%define api.pure
%defines /* crea el .h, es lo mismo que -d */
%error-verbose
%parse-param { char **letras }
%parse-param { char ***operandos }
%parse-param { char **operadores }
%parse-param { unsigned int *cantidad_operandos }
%parse-param { char **operacion }

%union {
	char *cadena;
}
%left '-' '+' '*' '/'
%token<cadena> OPERANDO

%%
operacion:
	expresion '=' operando {
		/* Elimina el último dígito que representa al resultado porque no interesa */
		*operacion = malloc (strlen(aux));
		strncpy(*operacion, aux, strlen(aux) - 1);
		(*operacion)[strlen(aux) - 1] = '\0';
		free (aux);
	};

expresion:
	operando
	|
	expresion '+' {
		guardar_operador ("+", operadores, cantidad_operandos);
		aux = realloc (aux, strlen (aux) + 2);
		strcat(aux, "0");
	} expresion
	|
	expresion '-' {
		guardar_operador ("-", operadores, cantidad_operandos);
		aux = realloc (aux, strlen (aux) + 2);
		strcat(aux, "0");
	} expresion
	|
	expresion '*' {
		guardar_operador ("*", operadores, cantidad_operandos);
		aux = realloc (aux, strlen (aux) + 2);
		strcat(aux, "0");
	} expresion
	|
	expresion '/' {
		guardar_operador ("/", operadores, cantidad_operandos);
		aux = realloc (aux, strlen (aux) + 2);
		strcat(aux, "0");
	} expresion
	|
	'(' {
		guardar_operador ("(", operadores, cantidad_operandos);
		aux = realloc (aux, strlen (aux) + 1);
		strcat(aux, "0");
	}
	expresion
	')' {
		guardar_operador (")", operadores, cantidad_operandos);
		aux = realloc (aux, strlen (aux) + 1);
		strcat(aux, "0");
	};

operando:
	OPERANDO {
		a_minusculas($OPERANDO);
		if (procesar_letras ($OPERANDO, letras)) {
			guardar_operando ($OPERANDO, operandos, cantidad_operandos);
			free (yylval.cadena);
			aux = realloc (aux, strlen (aux) + 2);
			strcat(aux, "1");
		}
		else
			return 2;
	};
%%

void yyerror (char **restrict letras, char ***restrict operandos, char **restrict operadores,
 			  unsigned int *restrict cantidad_operandos, char **restrict operacion, const char *restrict const mensaje) {
	UNUSED(letras);
	UNUSED(operandos);
	UNUSED(operadores);
	UNUSED(cantidad_operandos);
	UNUSED(operacion);
	printf ("Error: %s\n", mensaje);
}

int procesar (char *restrict const entrada, char **restrict letras,
			  char ***restrict operandos, char **restrict operadores,
			  unsigned int *restrict cantidad_operandos, char **restrict operacion) {
	if (entrada == NULL)
		yyin = (FILE *) 0;
	else {
		yy_scan_string(entrada);
		free(entrada);
	}

	aux = malloc (1);
	aux[0] = '\0';

	return yyparse(letras, operandos, operadores, cantidad_operandos, operacion);
}
