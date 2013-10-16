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
unsigned int
procesar_letras (const char *restrict operando, char **restrict const letras)
{
	do
		{
			if (*letras == NULL)
				{
					*letras = malloc (1);
					*letras[0] = '\0';
				}
			if ((strchr (*letras, *operando) == NULL))
				{
					if (strlen (*letras) == 10)
						return 0;
					size_t len = strlen (*letras);
					*letras = realloc (*letras, len + 2);
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
									unsigned int *restrict const cantidad_operandos)
{
	(*cantidad_operandos)++;
	*operandos = realloc (*operandos, sizeof (char *) * (*cantidad_operandos));
	(*operandos)[*cantidad_operandos - 1] = malloc (strlen (operando) + 1);
	strcpy ((*operandos)[*cantidad_operandos - 1], operando);
}

// guarda el operador
void
guardar_operador (const char *restrict const operador,
									char **restrict operadores)
{
	if (*operadores == NULL)
		{
			*operadores = malloc (2);
			*operadores[0] = '\0';
		}
	else
		*operadores = realloc (*operadores, strlen (*operadores) + 2);
	strcat (*operadores, operador);
}

void
generar_poblacion_inicial (struct individuos_s **restrict individuos,
													 const char *restrict const letras,
													 const unsigned long int *restrict const poblacion,
													 const unsigned long int *restrict const semilla)
{
	const size_t len = strlen (letras);

	if (semilla != NULL)
		mt_seed32new (*semilla);
	else
		mt_seed ();

	for (unsigned long int n = 0; n < *poblacion; n++)
		{
			mpz_init ((*individuos)[n].aptitud);
			(*individuos)[n].letras = malloc (10);
			memset ((*individuos)[n].letras, '\0', 10);
			memcpy ((*individuos)[n].letras, letras, len);

			for (unsigned int columna = 0; columna < 10; columna++)
				{
					unsigned int aleatorio = al_azar (0, 9);
					const char caracter = (*individuos)[n].letras[aleatorio];
					(*individuos)[n].letras[aleatorio] =
						(*individuos)[n].letras[columna];
					(*individuos)[n].letras[columna] = caracter;
				}
		}
}

void
calcular_aptitud1 (struct individuos_s *restrict const individuo,
									 char **restrict const operandos,
									 const unsigned int *restrict const cantidad_operandos,
									 const char *restrict const operadores,
									 char *const operacion,
									 const unsigned int *restrict const debug,
									 const char *restrict const letras)
{
	mpz_t resultado_deseado, *resultado_obtenido = NULL;
	char *operando = NULL;

	/* Calcula el resultado deseado */
	convertir_operando_a_numeros (individuo,
																operandos[*cantidad_operandos - 1],
																&operando);
	mpz_init_set_str (resultado_deseado, operando, 10);
	free (operando);

	if (*debug > 2)
		{
			puts ("");
			for (unsigned int j = 0; j < (unsigned int) strlen (letras); j++)
				{
					char c = letras[j];
					printf ("%c:", c);
					for (unsigned int x = 0; x < 10; x++)
						if (c == individuo->letras[x])
							{
								printf ("%u  ", x);
								break;
							}
				}
		}

	/* Calcula el resultado obtenido */
	calcular_operacion (operandos, individuo, operadores, operacion,
											&resultado_obtenido, debug);
	if (resultado_obtenido == NULL)
		{
			if (*debug > 2)
				puts ("\nIndividuo inválido");
			mpz_set_si (individuo->aptitud, -1);
			mpz_clear (resultado_deseado);
			return;
		}

	/* Se convierte ambos a char y se hacen las comparaciones */
	char *resultado_deseado_str = NULL;
	resultado_deseado_str = mpz_get_str (NULL, 10, resultado_deseado);
	mpz_clear (resultado_deseado);

	char *resultado_obtenido_str = NULL;
	resultado_obtenido_str = mpz_get_str (NULL, 10, *resultado_obtenido);
	mpz_clear (*resultado_obtenido);
	free (resultado_obtenido);

	unsigned int m, t;
	const char *restrict y;
	const char *restrict x;
	if (strlen (resultado_obtenido_str) >= strlen (resultado_deseado_str))
		{
			m = strlen (resultado_deseado_str) - 1;
			x = resultado_deseado_str;
			t = strlen (resultado_obtenido_str) - 1;
			y = resultado_obtenido_str;
		}
	else
		{
			m = strlen (resultado_obtenido_str) - 1;
			x = resultado_obtenido_str;
			t = strlen (resultado_deseado_str) - 1;
			y = resultado_deseado_str;
		}

	mpz_t aptitud;
	mpz_init (aptitud);

	mpz_t aptitud1, aptitud2;
	if (*debug > 2)
		{
			printf ("Resultado obtenido: %*s\n", t + 1, resultado_obtenido_str);
			printf (" Resultado deseado: %*s\n", t + 1, resultado_deseado_str);
			mpz_init (aptitud1);
			mpz_init (aptitud2);
		}

	/* cuántos dígitos de más tiene el resultado más largo en */
	/* comparación con el más corto */
	unsigned const int sobra = t - m;

	/* primera parte de la fórmula, */
	/* va de atrás para adelante porque la unidad está en la última */
	/* posición del array */
	/* multiplica por (i+1) en vez de por 10, empezando con i = 0 */
	unsigned int j = 0;
	if (*debug > 2)
		{
			for (int i = m; i > -1; i--)
				mpz_add_ui (aptitud1, aptitud1,
										(abs (((int) x[i] - '0') - ((int) y[i + sobra] - '0')) *
										 ((j++) + 1)));
			gmp_printf ("Aptitud: %Zd + ", aptitud1);
		}
	else
		for (int i = m; i > -1; i--)
			mpz_add_ui (aptitud, aptitud,
									(abs (((int) x[i] - '0') - ((int) y[i + sobra] - '0')) *
									 ((j++) + 1)));

	/* segunda parte de la fórmula, */
	/* va de adelante para atrás porque la unidad más grande está en */
	/* la primera posición del array */
	if (*debug > 2)
		{
			for (unsigned int i = 0; i < sobra; i++)
				mpz_add_ui (aptitud2, aptitud2,
										(((int) y[i] - '0') * ((int) pow (10, (sobra - i + 1)))));
			gmp_printf ("%Zd\n", aptitud2);
			mpz_add (aptitud, aptitud1, aptitud2);
			mpz_clear (aptitud1);
			mpz_clear (aptitud2);
		}
	else
		for (unsigned int i = 0; i < sobra; i++)
			mpz_add_ui (aptitud, aptitud,
									(((int) y[i] - '0') * ((int) pow (10, (sobra - i + 1)))));

	mpz_set (individuo->aptitud, aptitud);

	mpz_clear (aptitud);

	free (resultado_obtenido_str);
	free (resultado_deseado_str);
}

void
calcular_aptitud2 (struct individuos_s *restrict const individuo,
									 char **restrict const operandos,
									 const unsigned int *restrict const cantidad_operandos,
									 const char *restrict const operadores,
									 char *const operacion,
									 const unsigned int *restrict const debug,
									 const char *restrict const letras)
{
	mpz_t resultado_deseado, *resultado_obtenido = NULL;
	char *operando = NULL;

	/* Calcula el resultado deseado */
	convertir_operando_a_numeros (individuo,
																operandos[*cantidad_operandos - 1],
																&operando);

	mpz_init_set_str (resultado_deseado, operando, 10);

	if (*debug > 2)
		{
			puts ("");
			for (unsigned int j = 0; j < (unsigned int) strlen (letras); j++)
				{
					char c = letras[j];
					printf ("%c:", c);
					for (unsigned int x = 0; x < 10; x++)
						if (c == individuo->letras[x])
							{
								printf ("%u  ", x);
								break;
							}
				}
		}

	/* Calcula el resultado obtenido */
	calcular_operacion (operandos, individuo, operadores, operacion,
											&resultado_obtenido, debug);
	if (resultado_obtenido == NULL)
		{
			if (*debug > 2)
				puts ("\nIndividuo inválido");
			mpz_set_si (individuo->aptitud, -1);
			return;
		}

	char *resultado_obtenido_str = mpz_get_str (NULL, 10, *resultado_obtenido);
	unsigned int ancho;
	ancho =
		(strlen (resultado_obtenido_str) >
		 strlen (operando) ? strlen (resultado_obtenido_str) : strlen (operando));
	if (*debug > 2)
		{
			printf ("Resultado obtenido: %*s\n", ancho, resultado_obtenido_str);
			printf (" Resultado deseado: %s\n", operando);
		}
	free (resultado_obtenido_str);
	free (operando);

	/* Calcula la aptitud */
	mpz_t aptitud;
	mpz_init (aptitud);
	mpz_sub (aptitud, resultado_deseado, *resultado_obtenido);
	mpz_abs (aptitud, aptitud);

	if (*debug > 2)
		{
			char *aptitud_str = mpz_get_str (NULL, 10, aptitud);
			printf ("           Aptitud: %*s\n", ancho, aptitud_str);
			free (aptitud_str);
		}

	mpz_set (individuo->aptitud, aptitud);

	/* Libera memoria */
	mpz_clear (resultado_deseado);
	mpz_clear (*resultado_obtenido);
	free (resultado_obtenido);
	mpz_clear (aptitud);
}

void
convertir_operando_a_numeros (const struct individuos_s *restrict const
															individuo,
															const char *restrict const operando_str,
															char **restrict operando)
{
	unsigned int columna = 0;
	const unsigned int longitud_operando = (int) strlen (operando_str);
	*operando = malloc (longitud_operando + 1);
	// recorre todos los caracteres del operando
	do
		// recorre todos los caracteres del individuo hasta encontrar el valor que corresponda
		// con el caracter seleccionado del operando
		for (unsigned int indice = 0; indice < 10; indice++)
			if (operando_str[columna] == individuo->letras[indice])
				{
					(*operando)[columna] = '0' + indice;
					break;
				}
	while (++columna < longitud_operando);
	(*operando)[columna] = '\0';
}

/* Función de comparación de estructuras de individuos */
int
individuos_cmp (const void *const ptr1, const void *const ptr2)
{
	const struct individuos_s *restrict const individuo1 = ptr1;
	const struct individuos_s *restrict const individuo2 = ptr2;
	if (mpz_sgn (individuo1->aptitud) == -1)
		return 1;
	if (mpz_sgn (individuo2->aptitud) == -1)
		return -1;
	else
		return mpz_cmp (individuo1->aptitud, individuo2->aptitud);
}

void
seleccion_elitista_con_ranking (struct individuos_s **individuos,
																struct individuos_s **restrict elite,
																const unsigned long int *restrict const
																poblacion,
																const unsigned long int *restrict const
																cantidad, const float rmin,
																const unsigned int *restrict const debug)
{
	struct individuos_s **reserva = NULL;
	unsigned long int copias_totales = 0, indice_nuevos = 0, copias_extras = 0;

	for (unsigned long int indice = 0;
			 (indice < *cantidad) && (copias_totales < *cantidad); indice++)
		{
			unsigned int copias = 0;

			if (*cantidad == 1)
				copias = 1;
			else
				/* Total de copias redondeada */
				copias = round (rmin +
												2. * (((*poblacion - indice) * (1. - rmin)) /
															(*poblacion - 1)));

			/* Verifica que copias no supere a cantidad.
			 * Puede pasar si cantidad es muy chica */
			if ((copias + copias_totales) >= *cantidad)
				copias = *cantidad - copias_totales;

			/* Primero verifica que no se haya copiado el individuo */
			unsigned int copiar = 1;
			for (unsigned long int i = 0; i < copias_totales; i++)
				{
					copiar = 0;
					for (unsigned int j = 0; (j < 10) && (!copiar); j++)
						if ((*elite)[i].letras[j] != (*individuos)[indice].letras[j])
							copiar = 1;
					if (copiar == 0)
						break;
				}

			if (copiar)
				{
					/* Hace la cantidad de copias correspondientes */
					for (unsigned int n = 0; n < copias; n++, indice_nuevos++)
						{
							mpz_set ((*elite)[indice_nuevos].aptitud,
											 (*individuos)[indice].aptitud);

							memcpy ((*elite)[indice_nuevos].letras,
											(*individuos)[indice].letras, 10);
						}
					copias_totales += copias;
				}
			/* Si no se hace copia por las dudas mantener en reserva
			 * por si al final no se llega a la cantidad */
			else if (copias > 0)
				{
					unsigned int a_copiar = copias_extras + copias;
					reserva =
						realloc (reserva, a_copiar * sizeof (struct individuos_s *));

					for (unsigned int i = copias_extras; i < a_copiar; i++)
						{
							reserva[i] = malloc (sizeof (struct individuos_s));

							mpz_init (reserva[i]->aptitud);
							mpz_set (reserva[i]->aptitud, (*individuos)[indice].aptitud);

							reserva[i]->letras = malloc (10);
							memcpy (reserva[i]->letras, (*individuos)[indice].letras, 10);

							copias_extras++;
						}
				}

			if (*debug > 2)
				{
					if (copiar)
						printf ("Individuo %lu: %u copias\n", indice + 1, copias);
					else
						printf ("Individuo %lu: 0 copias\n", indice + 1);
				}
		}

	/* Si no alcanzaron las copias de la parte entera 
	 * copia de la parte decimal hasta completar la cantidad */
	for (unsigned int n = 0; copias_totales < *cantidad; n++, indice_nuevos++)
		{
			mpz_set ((*elite)[indice_nuevos].aptitud, reserva[n]->aptitud);
			memcpy ((*elite)[indice_nuevos].letras, reserva[n]->letras, 10);
			copias_totales++;
		}

	if (*debug > 2)
		printf ("\nCopias totales: %lu\n", copias_totales);

	/* Libera todo la memoria reservada para las copias extras */
	if (reserva != NULL)
		{
			for (unsigned int n = 0; n < copias_extras; n++)
				{
					mpz_clear (reserva[n]->aptitud);
					free (reserva[n]->letras);
					free (reserva[n]);
				}
			free (reserva);
		}
}

void
armar_ruleta (const struct individuos_s *restrict const individuos,
							const unsigned long int *restrict const poblacion,
							struct ruleta_s *restrict const ruleta)
{
	double total_aptitud = 0;

	/* Suma las aptitudes */
	for (unsigned long int i = 0; i < *poblacion; i++)
		/* Si la aptitud no entra en un unsigned long int no
		 * lo suma, su aporte sería marginal, muy cercano a 0 */
		if (mpz_fits_ulong_p (individuos[i].aptitud))
			total_aptitud += (1. / mpz_get_ui (individuos[i].aptitud));

	unsigned int ancho = 0;
	unsigned int x = mpz_get_ui (individuos[*poblacion - 1].aptitud);
	/* Define la cantidad de ranuras como 10^(longitud de la aptitud más grande) */
	while ((x /= 10) > 0)
		ancho++;

	double anterior = 0, rango;
	for (unsigned long int i = 0; i < *poblacion; i++)
		{
			rango = (1. / mpz_get_ui (individuos[i].aptitud)) / total_aptitud;
			rango *= pow (10, ancho);

			ruleta[i].desde = anterior;
			ruleta[i].hasta = anterior + rango;
			anterior += rango;
		}
}

unsigned long int
seleccion_por_ruleta (const unsigned long int *restrict const poblacion,
											const struct ruleta_s *restrict const ruleta)
{
	double aleatorio =
		al_azar_d (ruleta[0].desde, ruleta[*poblacion - 1].hasta);
	for (unsigned long int i = 0; i < *poblacion; i++)
		if ((aleatorio >= ruleta[i].desde) && (aleatorio < ruleta[i].hasta))
			aleatorio = i;
	return aleatorio;
}

void
cruza_ciclica (struct individuos_s *restrict madre,
							 struct individuos_s *restrict padre)
{
	char *letras_m = NULL;

	unsigned int numl = 0;

	for (unsigned int j = 0; j < 10; j++)
		if (madre->letras[j] != '\0')
			{
				letras_m = realloc (letras_m, numl + 1);
				letras_m[numl++] = madre->letras[j];
			}

	char letras_p[numl], hijo1[numl], hijo2[numl];

	numl = 0;

	for (unsigned int j = 0; j < 10; j++)
		if (padre->letras[j] != '\0')
			letras_p[numl++] = padre->letras[j];

	memset (hijo1, '\0', numl);
	memset (hijo2, '\0', numl);

	unsigned int i = al_azar (0, numl - 1);
	while (letras_m[i] == letras_p[i])
		i = al_azar (0, numl - 1);

	for (unsigned int x = 0; x < numl; x++)
		{
			hijo1[i] = letras_m[i];
			hijo2[i] = letras_p[i];
			for (unsigned int j = 0; j < numl; j++)
				/* Busca la letra del padre en la madre */
				if (letras_p[i] == letras_m[j])
					{
						/* Verifica si cumplió el ciclo */
						for (unsigned int h = 0; h < numl; h++)
							{
								/* La letra del padre ya está en el hijo de la madre */
								/* Ciclo cumplido, copia el resto */
								if (letras_p[j] == hijo1[h])
									{
										hijo1[j] = letras_m[j];
										hijo2[j] = letras_p[j];
										/* Recorre todas las letras */
										for (unsigned int u = 0; u < numl; u++)
											{
												if (hijo1[u] == '\0')
													{
														/* Se guardan intercambiando padre y madre */
														hijo1[u] = letras_p[u];
														hijo2[u] = letras_m[u];
													}
											}
										x = numl;
										break;
									}
							}
						i = j;
						break;
					}
		}

	numl = 0;
	for (unsigned int j = 0; j < 10; j++)
		if (madre->letras[j] != '\0')
			madre->letras[j] = hijo1[numl++];

	numl = 0;
	for (unsigned int j = 0; j < 10; j++)
		if (padre->letras[j] != '\0')
			padre->letras[j] = hijo2[numl++];

	free (letras_m);
}

void
mutacion (struct individuos_s *restrict individuo)
{
	if (al_azar (0, 1))
		{
			unsigned int gen1, gen2;

			gen1 = al_azar (0, 9);
			gen2 = al_azar (0, 9);

			while ((gen1 == gen2) || ((individuo->letras[gen1] == '\0') &&
																(individuo->letras[gen2] == '\0')))
				gen2 = al_azar (0, 9);

			char aux = individuo->letras[gen1];
			individuo->letras[gen1] = individuo->letras[gen2];
			individuo->letras[gen2] = aux;
		}
	else
		{
			/* Calcula la cantidad de cambios según la
			 * longitud de la aptitud */
			unsigned int aptitud, cambios = 0;
			if (mpz_fits_ulong_p (individuo->aptitud))
				{
					aptitud = mpz_get_ui (individuo->aptitud);
					unsigned int x = aptitud;
					do
						cambios++;
					while ((x /= 10) > 0);
					cambios += round (aptitud / pow (10, cambios));
					if (cambios == 1)
						cambios++;
				}
			else
				cambios = mpz_sizeinbase (individuo->aptitud, 10);

			/* Obtiene un número al azar por cada cambio */
			unsigned int genes[cambios];
			for (unsigned int i = 0; i < cambios; i++)
				genes[i] = al_azar (0, 9);

			/* Verifica que los números extremos no sean iguales */
			while (genes[0] == genes[cambios - 1])
				genes[0] = al_azar (0, 9);

			/* Verifica en los números de adentro que uno no sea
			 * igual al siguiente */
			for (unsigned int i = 0; i < (cambios - 1); i++)
				while (genes[i] == genes[i + 1])
					if ((cambios > 2) && (i == (cambios - 2)))
						while ((genes[i] == genes[i - 1]) || (genes[i] == genes[i + 1]))
							genes[i] = al_azar (0, 9);
					else
						genes[i + 1] = al_azar (0, 9);

			/* Hace los intercambios */
			char c;
			for (unsigned int i = 0; i < (cambios - 1); i++)
				{
					c = individuo->letras[genes[i]];
					individuo->letras[genes[i]] = individuo->letras[genes[i + 1]];
					individuo->letras[genes[i + 1]] = c;
				}
		}
}

unsigned long int
al_azar (const unsigned int min, const unsigned int max)
{
	return (min + mt_llrand () / (UINT64_MAX / (max - min + 1) + 1));
}

double
al_azar_d (const double min, const double max)
{
	return ((mt_ldrand () * (max - min)) + min);
}

void
mostrar_operacion (const struct individuos_s *restrict const
									 individuo, char **restrict const operandos,
									 const char *restrict const operadores,
									 const char *restrict operacion)
{
	unsigned int n_operando = 0, n_operador = 0;

	char *operacion_completa = NULL;
	operacion_completa = malloc (1);
	operacion_completa[0] = '\0';

	do
		{
			/* operador */
			if (*operacion == '0')
				{
					operacion_completa =
						realloc (operacion_completa, strlen (operacion_completa) + 2);

					operacion_completa[strlen (operacion_completa) + 1] = '\0';

					operacion_completa[strlen (operacion_completa)] =
						operadores[n_operador];

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

					if (*operacion == '\0')
						{
							operacion_completa =
								realloc (operacion_completa,
												 strlen (operacion_completa) + longitud_operando + 4);
							strcat (operacion_completa, " = ");
						}
					else
						operacion_completa =
							realloc (operacion_completa,
											 strlen (operacion_completa) + longitud_operando + 1);

					strcat (operacion_completa, operando_str);
					free (operando_str);
					n_operando++;
				}
		}
	while (*operacion++);

	printf ("\nOperación: %s\n", operacion_completa);
	free (operacion_completa);
}

unsigned int
iguales (const struct individuos_s *restrict const i1,
				 const struct individuos_s *restrict const i2)
{
	unsigned int j = 0;
	for (unsigned int i = 0; i < 10; i++)
		{
			/* Avanza hasta la primer letra del 1er individuo */
			if (i1->letras[i] == '\0')
				continue;
			/* Avanza hasta la primer letra del 2do individuo */
			while (i2->letras[j] == '\0')
				j++;
			if (i1->letras[i] == i2->letras[j])
				{
					j++;
					continue;
				}
			else
				return 0;
		}
	return 1;
}

void
diff_t (const struct timespec *restrict const comienzo_t,
				const struct timespec *restrict const fin_t,
				struct timespec *restrict const total_t)
{
	if ((fin_t->tv_nsec - comienzo_t->tv_nsec) < 0)
		{
			total_t->tv_sec = fin_t->tv_sec - comienzo_t->tv_sec - 1;
			total_t->tv_nsec = 1000000000 + fin_t->tv_nsec - comienzo_t->tv_nsec;
		}
	else
		{
			total_t->tv_sec = fin_t->tv_sec - comienzo_t->tv_sec;
			total_t->tv_nsec = fin_t->tv_nsec - comienzo_t->tv_nsec;
		}
}

void
cantidades (struct args *restrict const args,
						struct individuos_s **restrict individuos,
						struct individuos_s **restrict elite,
						struct individuos_s **restrict cruzados,
						struct individuos_s **restrict mutados)
{
	unsigned int inicializar, c_elite = 0, c_cruzar = 0, c_mutar = 0;
	inicializar = (*individuos == NULL) ? 1 : 0;

	*individuos =
		realloc (*individuos, args->poblacion * sizeof (struct individuos_s));

	if (args->p_elite != 0)
		{
			c_elite = round (args->poblacion * args->p_elite / 100.);
			if (c_elite < 1)
				c_elite = 1;
		}

	if (args->p_mutar != 0)
		{
			c_mutar = round (args->poblacion * args->p_mutar / 100.);
			if (c_mutar < 1)
				c_mutar = 1;
		}

	if (args->p_cruzar != 0)
		{
			c_cruzar = round (args->poblacion * args->p_cruzar / 100.);
			if ((c_cruzar % 2) != 0)
				{
					/* El restante lo agrego a mutación */
					if (args->p_mutar > 0)
						{
							c_mutar++;
							c_cruzar -= 1;
						}
					/* Si no hay mutación se intenta agregar a elite */
					else if (args->p_elite > 0)
						{
							c_elite++;
							c_cruzar -= 1;
						}
					/* Si no hay mutación ni elite queda una cantidad
					 * a cruzar impar y listo */
				}
		}

	/* Puede que por los rendondeos me quede un individuo sin asignar a
	 * selección, cruza o mutación */
	if ((args->poblacion - c_elite - c_cruzar - c_mutar) > 0)
		{
			/* Primero intento asignarlo a mutación */
			if (args->p_mutar > 0)
				c_mutar++;
			/* Si no hay mutación se intenta agregar a elite */
			else if (args->p_elite > 0)
				c_elite++;
			/* Si no hay mutación ni elite queda una cantidad
			 * a cruzar impar y listo */
			else
				c_cruzar++;
		}

	*elite = realloc (*elite, c_elite * sizeof (struct individuos_s));
	*cruzados = realloc (*cruzados, c_cruzar * sizeof (struct individuos_s));
	*mutados = realloc (*mutados, c_mutar * sizeof (struct individuos_s));

	if (inicializar)
		{
			for (unsigned long int i = 0; i < c_elite; i++)
				{
					mpz_init ((*elite)[i].aptitud);
					(*elite)[i].letras = malloc (10);
				}
			for (unsigned long int i = 0; i < c_cruzar; i++)
				{
					mpz_init ((*cruzados)[i].aptitud);
					(*cruzados)[i].letras = malloc (10);
				}
			for (unsigned long int i = 0; i < c_mutar; i++)
				{
					mpz_init ((*mutados)[i].aptitud);
					(*mutados)[i].letras = malloc (10);
				}
		}
	else
		{
			for (unsigned int i = args->cantidad_elite; i < c_elite; i++)
				{
					mpz_init ((*elite)[i].aptitud);
					(*elite)[i].letras = malloc (10);
				}
			for (unsigned int i = args->cantidad_a_cruzar; i < c_cruzar; i++)
				{
					mpz_init ((*cruzados)[i].aptitud);
					(*cruzados)[i].letras = malloc (10);
				}
			for (unsigned int i = args->cantidad_a_mutar; i < c_mutar; i++)
				{
					mpz_init ((*mutados)[i].aptitud);
					(*mutados)[i].letras = malloc (10);
				}
		}

	args->cantidad_elite = c_elite;
	args->cantidad_a_cruzar = c_cruzar;
	args->cantidad_a_mutar = c_mutar;
}
