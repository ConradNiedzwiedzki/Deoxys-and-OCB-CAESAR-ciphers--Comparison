#include "stdafx.h"


double liczbaIteracji = rozmiarTablicy;
void test(double* DeoxysSz, double* DeoxysDesz, double* OCBSz, double* OCBDesz, clock_t Deoxys1, clock_t Deoxys2, Clock_z)
{
	double sum1 = 0.0, sum2 = 0.0, sum3 = 0.0, sum4 = 0.0;
	for (int i = 1; i < (int)liczbaIteracji; i++)
	{
		DeoxysSz[i] = ((double)(Deoxys2 - Deoxys1) / CLOCKS_PER_SEC);
		DeoxysDesz[i] = ((double)(Deoxys2d - Deoxys1d) / CLOCKS_PER_SEC);
		OCBSz[i] = ((double)(OCB2 - OCB1) / CLOCKS_PER_SEC);
		OCBDesz[i] = ((double)(OCB2d - OCB1d) / CLOCKS_PER_SEC);
		//printf("\n\nDeoxys szyfrowanie (%d):   %0.50lf sekund(y)\n", i, DeoxysSz[i]);
		//printf("\nDeoxys deszyfrowanie (%d): %0.50lf sekund(y)\n", i, DeoxysDesz[i]);
		//printf("\nOCB szyfrowanie (%d):      %0.50lf sekund(y)\n", i, OCBSz[i]);
		//printf("\nOCB deszyfrowanie (%d):    %0.50lf sekund(y)\n", i, OCBDesz[i]);

		Deoxys1 = clock();
		szyfrowanieDeoxys(skojarzoneDane, dlugoscSkojarzonychDanych, tekstJawny, dlugoscTekstuJawnego, klucz, nonce, szyfrogram, dlugoscSzyfrogramu);
		Deoxys2 = clock();
		Deoxys1d = clock();
		authentication = deszyfrowanieDeoxys(skojarzoneDane, dlugoscSkojarzonychDanych, tekstJawny, dlugoscTekstuJawnego, klucz, nonce, szyfrogram, dlugoscSzyfrogramu);
		Deoxys2d = clock();
		OCB1 = clock();
		szyfrowanieOCB(skojarzoneDane, dlugoscSkojarzonychDanych, tekstJawny, dlugoscTekstuJawnego, klucz, nonce, szyfrogram, dlugoscSzyfrogramu, nonce);
		OCB2 = clock();
		OCB1d = clock();
		authentication = deszyfrowanieOCB(skojarzoneDane, dlugoscSkojarzonychDanych, tekstJawny, dlugoscTekstuJawnego, klucz, nonce, szyfrogram, dlugoscSzyfrogramu, nonce);
		OCB2d = clock();
		sum1 += DeoxysSz[i];
		sum2 += DeoxysDesz[i];
		sum3 += OCBSz[i];
		sum4 += OCBDesz[i];
	}
	printf("\n\nSredni czas szyfrowania Deoxys:   %0.65lf sekund(y)\n", sum1 / liczbaIteracji);
	printf("\n\nSredni czas deszyfrowania Deoxys: %0.65lf sekund(y)\n", sum2 / liczbaIteracji);
	printf("\n\nSredni czas szyfrowania OCB:      %0.65lf sekund(y)\n", sum3 / liczbaIteracji);
	printf("\n\nSredni czas deszyfrowania OCB:    %0.65f sekund(y)\n", sum4 / liczbaIteracji);
	printf("\n");
}