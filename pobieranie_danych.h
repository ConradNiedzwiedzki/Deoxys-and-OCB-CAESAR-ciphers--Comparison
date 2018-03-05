#include "stdafx.h"

unsigned long pobierzDlugosc(const char* nazwapliku)
{
	FILE *plikTestowy;
	char c;
	unsigned long dlugosc = 0;
	plikTestowy = fopen(nazwapliku, "r");
	while ((c = fgetc(plikTestowy)))
	{
		if (c == EOF)
			break;
		dlugosc += 1;
	}
	fclose(plikTestowy);
	return dlugosc;
}

char * pobierzDane(char * bufor, const char* nazwapliku, int rozmiar)
{
	FILE *plikTestowy;
	plikTestowy = fopen(nazwapliku, "r");
	fgets(bufor, rozmiar, plikTestowy);
	fclose(plikTestowy);
	return bufor;
}
