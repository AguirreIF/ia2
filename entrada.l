%option read noyywrap bison-bridge noinput nounput never-interactive warn
%option outfile="entrada.c" nodefault pointer 8bit nodefault perf-report perf-report

%{
	#include "entrada.tab.h"
%}

%%

[ \t\n]+		/* ignora espacios en blanco */

[[:alpha:]]+				{ yylval->cadena = strdup(yytext); return OPERANDO; }
"+"|"-"|"*"|"/"|"="|"("|")"	{ return yytext[0]; }

.							{ printf("Token no reconocido %s\n", yytext); yyterminate(); return -1; }

<<EOF>>						{ yy_delete_buffer(YY_CURRENT_BUFFER); return 0; }
