// Deoxys and OCB Comparison.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ocb3.h"
#include "deoxys.h"
#include "pobieranie_danych.h"

int main()
{
	//freopen("output.txt", "w", stdout);
	const int rozmiar = 5000;
	char P[rozmiar], A[rozmiar], K[16], N[15], A2[rozmiar];
	unsigned char szyfrogram[rozmiar + 16];

	// Pobieranie danych
	unsigned char* tekstJawny = (unsigned char*)pobierzDane(P, "P.txt", rozmiar);
	unsigned long dlugoscTekstuJawnego = pobierzDlugosc("P.txt");
	unsigned char* skojarzoneDane = (unsigned char*)pobierzDane(A, "A.txt", rozmiar);
	unsigned long dlugoscSkojarzonychDanych = pobierzDlugosc("A.txt");
	unsigned char* skojarzoneDane2 = (unsigned char*)pobierzDane(A2, "A2.txt", rozmiar);
	unsigned long dlugoscSkojarzonychDanych2 = pobierzDlugosc("A2.txt");
	unsigned char* klucz = (unsigned char*)pobierzDane(K, "K.txt", rozmiar);
	unsigned long dlugoscKlucza = pobierzDlugosc("K.txt");
	unsigned char* nonce = (unsigned char*)pobierzDane(N, "N.txt", rozmiar);
	unsigned long dlugoscNonce = pobierzDlugosc("N.txt");
	unsigned long dlugoscSzyfrogramu = dlugoscTekstuJawnego + 16;

	// Do sprawdzenia poprawnosci deszyfrowania
	unsigned char tekstJawny2[rozmiar];
	unsigned long dlugoscTekstuJawnego2 = dlugoscTekstuJawnego;

	printf("\n--------------------------------------------------------------------------------------");
	printf("\n------------------------------ DANE WEJSCIOWE ----------------------------------------");
	printf("\n--------------------------------------------------------------------------------------");
	printf("\nDlugosc tekstu jawnego: %d bitow", dlugoscTekstuJawnego * 8);
	printf("\nTekst jawny (hex):\n");
	for (int i = 0; i < (signed int)dlugoscTekstuJawnego; i++)
		printf("%02hhx", tekstJawny[i]);

	printf("\n\nDlugosc skojarzonych danych: %d bitow", dlugoscSkojarzonychDanych * 8);
	printf("\nSkojarzone dane (hex):\n");
	for (int i = 0; i < (signed int)dlugoscSkojarzonychDanych; i++)
		printf("%02hhx", skojarzoneDane[i]);

	printf("\n\nDlugosc klucza: %d bitow", dlugoscKlucza * 8);
	printf("\nKlucz (hex):\n");
	for (int i = 0; i < (signed int)dlugoscKlucza; i++)
		printf("%02hhx", klucz[i]);

	printf("\n\nDlugosc nonce: %d bity", dlugoscNonce * 8);
	printf("\nNonce (hex):\n");
	for (int i = 0; i < (signed int)dlugoscNonce; i++)
		printf("%02hhx", nonce[i]);

	printf("\n\n--------------------------------------------------------------------------------------");
	printf("\n---------------------------------- DEOXYS --------------------------------------------");
	printf("\n--------------------------------------------------------------------------------------");
	clock_t Deoxys1 = clock();
	szyfrowanieDeoxys(skojarzoneDane, dlugoscSkojarzonychDanych, tekstJawny, dlugoscTekstuJawnego, klucz, nonce, szyfrogram, dlugoscSzyfrogramu);
	clock_t Deoxys2 = clock();
	printf("\n\nSzyfrogram (hex):\n");
	for (int i = 0; i < (signed int)dlugoscSzyfrogramu; i++)
		printf("%02hhx", szyfrogram[i]);

	printf("\n\nTekst jawny po deszyfrowaniu (hex):\n");
	clock_t Deoxys1d = clock();
	int flagaUwierzytelnienia = deszyfrowanieDeoxys(skojarzoneDane, dlugoscSkojarzonychDanych, tekstJawny2, dlugoscTekstuJawnego2, klucz, nonce, szyfrogram, dlugoscSzyfrogramu);
	//Blad uwierzytelnienia:
	//int flagaUwierzytelnienia = deszyfrowanieDeoxys(skojarzoneDane2, dlugoscSkojarzonychDanych2, tekstJawny2, dlugoscTekstuJawnego2, klucz, nonce, szyfrogram, dlugoscSzyfrogramu);
	clock_t Deoxys2d = clock();
	if (flagaUwierzytelnienia == 0)
	{
		for (int i = 0; i < (signed int)dlugoscTekstuJawnego2; i++)
			printf("%02hhx", tekstJawny2[i]);
	}
	else
		printf("\nNiepowodzenie deszyfrowania Deoxys - BLAD UWIERZYTELNIENIA!");


	printf("\n\n--------------------------------------------------------------------------------------");
	printf("\n------------------------------------ OCB ---------------------------------------------");
	printf("\n--------------------------------------------------------------------------------------");
	clock_t OCB1 = clock();
	szyfrowanieOCB(skojarzoneDane, dlugoscSkojarzonychDanych, tekstJawny, dlugoscTekstuJawnego, klucz, nonce, szyfrogram, dlugoscSzyfrogramu, nonce);
	clock_t OCB2 = clock();
	printf("\n\nSzyfrogram (hex):\n");
	for (int i = 0; i < (signed int)dlugoscSzyfrogramu; i++)
		printf("%02hhx", szyfrogram[i]);

	printf("\n\nTekst jawny po deszyfrowaniu (hex):\n");
	clock_t OCB1d = clock();
	//flagaUwierzytelnienia = deszyfrowanieOCB(skojarzoneDane, dlugoscSkojarzonychDanych, tekstJawny2, dlugoscTekstuJawnego2, klucz, nonce, szyfrogram, dlugoscSzyfrogramu, nonce);
	//Blad uwierzytelnienia:
	flagaUwierzytelnienia = deszyfrowanieOCB(skojarzoneDane2, dlugoscSkojarzonychDanych2, tekstJawny2, dlugoscTekstuJawnego2, klucz, nonce, szyfrogram, dlugoscSzyfrogramu, nonce);
	clock_t OCB2d = clock();
	if (flagaUwierzytelnienia == 0)
	{
		for (int i = 0; i < (signed int)dlugoscTekstuJawnego2; i++)
			printf("%02hhx", tekstJawny2[i]);
	}
	else
		printf("Niepowodzenie deszyfrowania OCB - BLAD UWIERZYTELNIENIA!");

	printf("\n\n--------------------------------------------------------------------------------------");
	printf("\n-------------------------------- TESTOWANIE ------------------------------------------");
	printf("\n--------------------------------------------------------------------------------------");

	const int rozmiarTablicy = 500;
	double liczbaIteracji = rozmiarTablicy;
	printf("\nLiczba powtorzen szyfrowan i deszyfrowan algorytmow: %d", rozmiarTablicy);
	printf("\nLiczenie...");

	clock_t czas1, czas2, czas3, czas4, czas5;
	double sum1 = 0.0, sum2 = 0.0, sum3 = 0.0, sum4 = 0.0;
	for (int i = 1; i < (signed int)liczbaIteracji; i++)
	{
		czas1 = clock();
		szyfrowanieDeoxys(skojarzoneDane, dlugoscSkojarzonychDanych, tekstJawny, dlugoscTekstuJawnego, klucz, nonce, szyfrogram, dlugoscSzyfrogramu);
		czas2 = clock();
		flagaUwierzytelnienia = deszyfrowanieDeoxys(skojarzoneDane, dlugoscSkojarzonychDanych, tekstJawny, dlugoscTekstuJawnego, klucz, nonce, szyfrogram, dlugoscSzyfrogramu);
		czas3 = clock();
		szyfrowanieOCB(skojarzoneDane, dlugoscSkojarzonychDanych, tekstJawny, dlugoscTekstuJawnego, klucz, nonce, szyfrogram, dlugoscSzyfrogramu, nonce);
		czas4 = clock();
		flagaUwierzytelnienia = deszyfrowanieOCB(skojarzoneDane, dlugoscSkojarzonychDanych, tekstJawny, dlugoscTekstuJawnego, klucz, nonce, szyfrogram, dlugoscSzyfrogramu, nonce);
		czas5 = clock();
		sum1 += ((double)(czas2 - czas1) / CLOCKS_PER_SEC);
		sum2 += ((double)(czas3 - czas2) / CLOCKS_PER_SEC);
		sum3 += ((double)(czas4 - czas3) / CLOCKS_PER_SEC);
		sum4 += ((double)(czas5 - czas4) / CLOCKS_PER_SEC);
	}
	printf("\nObliczenia zakonczone:\n");
	printf("\nSredni czas szyfrowania Deoxys:   %0.45lf s", sum1 / (liczbaIteracji - 1));
	printf("\nSredni czas deszyfrowania Deoxys: %0.45lf s", sum2 / (liczbaIteracji - 1));
	printf("\nSredni czas szyfrowania OCB:      %0.45lf s", sum3 / (liczbaIteracji - 1));
	printf("\nSredni czas deszyfrowania OCB:    %0.45lf s", sum4 / (liczbaIteracji - 1));
	printf("\n");
	
	getchar();
	return 0;
}