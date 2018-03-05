#include "stdafx.h"

// Konrad Niedüwiedzki
// Deoxys-I-256-128 z Deoxys-BC-384 w úrodku
// Wersja v1.3

#define najbardziejZnaczacyBitSkojarzonychDanych			(0x2<<4)
#define najbardziejZnaczacyBitSkojarzonychDanychOstatni	(0x6<<4)
#define najbardziejZnaczacyBitTekstuJawnego				(0x0<<4)
#define najbardziejZnaczacyBitTekstuJawnegoNiezerowy		(0x4<<4)
#define najbardziejZnaczacyBitSumyKontrolnej				(0x1<<4)
#define najbardziejZnaczacyBitSumyKontrolnejNiepelnej		(0x5<<4)

#define pobierzStalaRCON(runda)				( ((unsigned int)staleRCON[runda] << 24) ^ ((unsigned int)staleRCON[runda] << 16) ^ ((unsigned int)staleRCON[runda] << 8) ^ ((unsigned int)staleRCON[runda] << 0))
#define pobierzStalaU32(tekstJawny)			( ((unsigned int)(tekstJawny)[0]  << 24) ^ ((unsigned int)(tekstJawny)[1]  << 16) ^ ((unsigned int)(tekstJawny)[2]  << 8) ^ ((unsigned int)(tekstJawny)[3])      )
#define ustawStalaU32(szyfrogram, stan)		{ (szyfrogram)[0] = (unsigned char)((stan) >> 24);	(szyfrogram)[1] = (unsigned char)((stan) >> 16);	(szyfrogram)[2] = (unsigned char)((stan) >>  8);	(szyfrogram)[3] = (unsigned char)(stan); }

unsigned char potegowanie(unsigned char x, unsigned char alfa)
{
	if (1 == alfa)
		return x;
	if (2 == alfa)
		return potegiDwojki[x];
	if (4 == alfa)
		return potegiDwojki[potegiDwojki[x]];
	else
	{
		return -1;
		printf("Niepoprawna alfa %d przekazana do funkcji G tweakey!\n", alfa);
	}
}

void funkcjaH(unsigned char tweakey[])
{
	int i;
	unsigned char zmiennaPomocnicza[16];
	for (i = 0; i<16; i++)
		zmiennaPomocnicza[permutacjeH[i]] = tweakey[i];
	memcpy(tweakey, zmiennaPomocnicza, 16);
}

void funkcjaG(unsigned char tweakey[], unsigned char alfa)
{
	int i;
	for (i = 0; i<16; i++)
		tweakey[i] = potegowanie(tweakey[i], alfa);
}

int przygotowanieDeoxysDoSzyfrowania(unsigned int* tweakeyRundy, const unsigned char* kluczTweaka)
{
	const unsigned int rzad1RCON = 0x01020408;
	unsigned char tweakey[3][16], alfa[3];
	int runda, liczbaRund;

	memcpy(tweakey[0], kluczTweaka + 0, 16);
	memcpy(tweakey[1], kluczTweaka + 16, 16);
	memcpy(tweakey[2], kluczTweaka + 32, 16);
	alfa[0] = 4;
	alfa[1] = 2;
	alfa[2] = 1;
	liczbaRund = 16;

	for (runda = 0; runda <= liczbaRund; runda++)
	{
		tweakeyRundy[4 * runda + 0] = pobierzStalaU32(tweakey[0] + 0)  ^ pobierzStalaU32(tweakey[1] + 0)  ^ (pobierzStalaU32(tweakey[2] + 0)) ^ rzad1RCON;
		tweakeyRundy[4 * runda + 1] = pobierzStalaU32(tweakey[0] + 4)  ^ pobierzStalaU32(tweakey[1] + 4)  ^ (pobierzStalaU32(tweakey[2] + 4)) ^ pobierzStalaRCON(runda);
		tweakeyRundy[4 * runda + 2] = pobierzStalaU32(tweakey[0] + 8)  ^ pobierzStalaU32(tweakey[1] + 8)  ^ (pobierzStalaU32(tweakey[2] + 8));
		tweakeyRundy[4 * runda + 3] = pobierzStalaU32(tweakey[0] + 12) ^ pobierzStalaU32(tweakey[1] + 12) ^ (pobierzStalaU32(tweakey[2] + 12));

		funkcjaH(tweakey[0]);
		funkcjaG(tweakey[0], alfa[0]);
		funkcjaH(tweakey[1]);
		funkcjaG(tweakey[1], alfa[1]);
		funkcjaH(tweakey[2]);
		funkcjaG(tweakey[2], alfa[2]);
	}
	return liczbaRund;
}

int przygotowanieDeoxysDoDeszyfrowania(unsigned int* tweakeyRundy, const unsigned char* kluczTweaka, int blokTweakey)
{
	int i, j, liczbaRund;
	unsigned int zmiennaPomocnicza;

	liczbaRund = przygotowanieDeoxysDoSzyfrowania(tweakeyRundy, kluczTweaka);

	for (i = 0, j = 4 * liczbaRund; i < j; i += 4, j -= 4)
	{
		zmiennaPomocnicza = tweakeyRundy[i];
		tweakeyRundy[i] = tweakeyRundy[j];
		tweakeyRundy[j] = zmiennaPomocnicza;

		zmiennaPomocnicza = tweakeyRundy[i + 1];
		tweakeyRundy[i + 1] = tweakeyRundy[j + 1];
		tweakeyRundy[j + 1] = zmiennaPomocnicza;

		zmiennaPomocnicza = tweakeyRundy[i + 2];
		tweakeyRundy[i + 2] = tweakeyRundy[j + 2];
		tweakeyRundy[j + 2] = zmiennaPomocnicza;

		zmiennaPomocnicza = tweakeyRundy[i + 3];
		tweakeyRundy[i + 3] = tweakeyRundy[j + 3];
		tweakeyRundy[j + 3] = zmiennaPomocnicza;
	}

	for (i = 1; i <= liczbaRund; i++)
	{
		tweakeyRundy += 4;
		tweakeyRundy[0] = 	Tdesz0[Tsz4[(tweakeyRundy[0] >> 24)] & 0xff] ^	Tdesz1[Tsz4[(tweakeyRundy[0] >> 16) & 0xff] & 0xff] ^	Tdesz2[Tsz4[(tweakeyRundy[0] >> 8) & 0xff] & 0xff] ^	Tdesz3[Tsz4[(tweakeyRundy[0]) & 0xff] & 0xff];
		tweakeyRundy[1] =	Tdesz0[Tsz4[(tweakeyRundy[1] >> 24)] & 0xff] ^	Tdesz1[Tsz4[(tweakeyRundy[1] >> 16) & 0xff] & 0xff] ^	Tdesz2[Tsz4[(tweakeyRundy[1] >> 8) & 0xff] & 0xff] ^	Tdesz3[Tsz4[(tweakeyRundy[1]) & 0xff] & 0xff];
		tweakeyRundy[2] =	Tdesz0[Tsz4[(tweakeyRundy[2] >> 24)] & 0xff] ^	Tdesz1[Tsz4[(tweakeyRundy[2] >> 16) & 0xff] & 0xff] ^	Tdesz2[Tsz4[(tweakeyRundy[2] >> 8) & 0xff] & 0xff] ^	Tdesz3[Tsz4[(tweakeyRundy[2]) & 0xff] & 0xff];
		tweakeyRundy[3] =	Tdesz0[Tsz4[(tweakeyRundy[3] >> 24)] & 0xff] ^	Tdesz1[Tsz4[(tweakeyRundy[3] >> 16) & 0xff] & 0xff] ^	Tdesz2[Tsz4[(tweakeyRundy[3] >> 8) & 0xff] & 0xff] ^	Tdesz3[Tsz4[(tweakeyRundy[3]) & 0xff] & 0xff];
	}
	return liczbaRund;
}

void szyfrowanieAESzTweakiem(unsigned char tekstJawny[16], unsigned char klucz[], unsigned char szyfrogram[16])
{
	unsigned int s0, s1, s2, s3, t0, t1, t2, t3, tweakeyRundy[4 * 17];
	int liczbaRund;

	liczbaRund = przygotowanieDeoxysDoSzyfrowania(tweakeyRundy, klucz);

	s0 = pobierzStalaU32(tekstJawny) ^ tweakeyRundy[0];
	s1 = pobierzStalaU32(tekstJawny + 4) ^ tweakeyRundy[1];
	s2 = pobierzStalaU32(tekstJawny + 8) ^ tweakeyRundy[2];
	s3 = pobierzStalaU32(tekstJawny + 12) ^ tweakeyRundy[3];

	//r1
	t0 = Tsz0[s0 >> 24] ^	Tsz1[(s1 >> 16) & 0xff] ^	Tsz2[(s2 >> 8) & 0xff] ^	Tsz3[s3 & 0xff] ^ tweakeyRundy[4];
	t1 = Tsz0[s1 >> 24] ^	Tsz1[(s2 >> 16) & 0xff] ^	Tsz2[(s3 >> 8) & 0xff] ^	Tsz3[s0 & 0xff] ^ tweakeyRundy[5];
	t2 = Tsz0[s2 >> 24] ^	Tsz1[(s3 >> 16) & 0xff] ^	Tsz2[(s0 >> 8) & 0xff] ^	Tsz3[s1 & 0xff] ^ tweakeyRundy[6];
	t3 = Tsz0[s3 >> 24] ^	Tsz1[(s0 >> 16) & 0xff] ^	Tsz2[(s1 >> 8) & 0xff] ^	Tsz3[s2 & 0xff] ^ tweakeyRundy[7];
	//r2 
	s0 = Tsz0[t0 >> 24] ^	Tsz1[(t1 >> 16) & 0xff] ^	Tsz2[(t2 >> 8) & 0xff] ^	Tsz3[t3 & 0xff] ^ tweakeyRundy[8];
	s1 = Tsz0[t1 >> 24] ^	Tsz1[(t2 >> 16) & 0xff] ^	Tsz2[(t3 >> 8) & 0xff] ^	Tsz3[t0 & 0xff] ^ tweakeyRundy[9];
	s2 = Tsz0[t2 >> 24] ^	Tsz1[(t3 >> 16) & 0xff] ^	Tsz2[(t0 >> 8) & 0xff] ^	Tsz3[t1 & 0xff] ^ tweakeyRundy[10];
	s3 = Tsz0[t3 >> 24] ^	Tsz1[(t0 >> 16) & 0xff] ^	Tsz2[(t1 >> 8) & 0xff] ^	Tsz3[t2 & 0xff] ^ tweakeyRundy[11];
	//r3 
	t0 = Tsz0[s0 >> 24] ^	Tsz1[(s1 >> 16) & 0xff] ^	Tsz2[(s2 >> 8) & 0xff] ^	Tsz3[s3 & 0xff] ^ tweakeyRundy[12];
	t1 = Tsz0[s1 >> 24] ^	Tsz1[(s2 >> 16) & 0xff] ^	Tsz2[(s3 >> 8) & 0xff] ^	Tsz3[s0 & 0xff] ^ tweakeyRundy[13];
	t2 = Tsz0[s2 >> 24] ^	Tsz1[(s3 >> 16) & 0xff] ^	Tsz2[(s0 >> 8) & 0xff] ^	Tsz3[s1 & 0xff] ^ tweakeyRundy[14];
	t3 = Tsz0[s3 >> 24] ^	Tsz1[(s0 >> 16) & 0xff] ^	Tsz2[(s1 >> 8) & 0xff] ^	Tsz3[s2 & 0xff] ^ tweakeyRundy[15];
	//r4 
	s0 = Tsz0[t0 >> 24] ^	Tsz1[(t1 >> 16) & 0xff] ^	Tsz2[(t2 >> 8) & 0xff] ^	Tsz3[t3 & 0xff] ^ tweakeyRundy[16];
	s1 = Tsz0[t1 >> 24] ^	Tsz1[(t2 >> 16) & 0xff] ^	Tsz2[(t3 >> 8) & 0xff] ^	Tsz3[t0 & 0xff] ^ tweakeyRundy[17];
	s2 = Tsz0[t2 >> 24] ^	Tsz1[(t3 >> 16) & 0xff] ^	Tsz2[(t0 >> 8) & 0xff] ^	Tsz3[t1 & 0xff] ^ tweakeyRundy[18];
	s3 = Tsz0[t3 >> 24] ^	Tsz1[(t0 >> 16) & 0xff] ^	Tsz2[(t1 >> 8) & 0xff] ^	Tsz3[t2 & 0xff] ^ tweakeyRundy[19];
	//r5 
	t0 = Tsz0[s0 >> 24] ^	Tsz1[(s1 >> 16) & 0xff] ^	Tsz2[(s2 >> 8) & 0xff] ^	Tsz3[s3 & 0xff] ^ tweakeyRundy[20];
	t1 = Tsz0[s1 >> 24] ^	Tsz1[(s2 >> 16) & 0xff] ^	Tsz2[(s3 >> 8) & 0xff] ^	Tsz3[s0 & 0xff] ^ tweakeyRundy[21];
	t2 = Tsz0[s2 >> 24] ^	Tsz1[(s3 >> 16) & 0xff] ^	Tsz2[(s0 >> 8) & 0xff] ^	Tsz3[s1 & 0xff] ^ tweakeyRundy[22];
	t3 = Tsz0[s3 >> 24] ^	Tsz1[(s0 >> 16) & 0xff] ^	Tsz2[(s1 >> 8) & 0xff] ^	Tsz3[s2 & 0xff] ^ tweakeyRundy[23];
	//r6 
	s0 = Tsz0[t0 >> 24] ^	Tsz1[(t1 >> 16) & 0xff] ^	Tsz2[(t2 >> 8) & 0xff] ^	Tsz3[t3 & 0xff] ^ tweakeyRundy[24];
	s1 = Tsz0[t1 >> 24] ^	Tsz1[(t2 >> 16) & 0xff] ^	Tsz2[(t3 >> 8) & 0xff] ^	Tsz3[t0 & 0xff] ^ tweakeyRundy[25];
	s2 = Tsz0[t2 >> 24] ^	Tsz1[(t3 >> 16) & 0xff] ^	Tsz2[(t0 >> 8) & 0xff] ^	Tsz3[t1 & 0xff] ^ tweakeyRundy[26];
	s3 = Tsz0[t3 >> 24] ^	Tsz1[(t0 >> 16) & 0xff] ^	Tsz2[(t1 >> 8) & 0xff] ^	Tsz3[t2 & 0xff] ^ tweakeyRundy[27];
	//r7 
	t0 = Tsz0[s0 >> 24] ^	Tsz1[(s1 >> 16) & 0xff] ^	Tsz2[(s2 >> 8) & 0xff] ^	Tsz3[s3 & 0xff] ^ tweakeyRundy[28];
	t1 = Tsz0[s1 >> 24] ^	Tsz1[(s2 >> 16) & 0xff] ^	Tsz2[(s3 >> 8) & 0xff] ^	Tsz3[s0 & 0xff] ^ tweakeyRundy[29];
	t2 = Tsz0[s2 >> 24] ^	Tsz1[(s3 >> 16) & 0xff] ^	Tsz2[(s0 >> 8) & 0xff] ^	Tsz3[s1 & 0xff] ^ tweakeyRundy[30];
	t3 = Tsz0[s3 >> 24] ^	Tsz1[(s0 >> 16) & 0xff] ^	Tsz2[(s1 >> 8) & 0xff] ^	Tsz3[s2 & 0xff] ^ tweakeyRundy[31];
	//r8 
	s0 = Tsz0[t0 >> 24] ^	Tsz1[(t1 >> 16) & 0xff] ^	Tsz2[(t2 >> 8) & 0xff] ^	Tsz3[t3 & 0xff] ^ tweakeyRundy[32];
	s1 = Tsz0[t1 >> 24] ^	Tsz1[(t2 >> 16) & 0xff] ^	Tsz2[(t3 >> 8) & 0xff] ^	Tsz3[t0 & 0xff] ^ tweakeyRundy[33];
	s2 = Tsz0[t2 >> 24] ^	Tsz1[(t3 >> 16) & 0xff] ^	Tsz2[(t0 >> 8) & 0xff] ^	Tsz3[t1 & 0xff] ^ tweakeyRundy[34];
	s3 = Tsz0[t3 >> 24] ^	Tsz1[(t0 >> 16) & 0xff] ^	Tsz2[(t1 >> 8) & 0xff] ^	Tsz3[t2 & 0xff] ^ tweakeyRundy[35];
	//r9 
	t0 = Tsz0[s0 >> 24] ^	Tsz1[(s1 >> 16) & 0xff] ^	Tsz2[(s2 >> 8) & 0xff] ^	Tsz3[s3 & 0xff] ^ tweakeyRundy[36];
	t1 = Tsz0[s1 >> 24] ^	Tsz1[(s2 >> 16) & 0xff] ^	Tsz2[(s3 >> 8) & 0xff] ^	Tsz3[s0 & 0xff] ^ tweakeyRundy[37];
	t2 = Tsz0[s2 >> 24] ^	Tsz1[(s3 >> 16) & 0xff] ^	Tsz2[(s0 >> 8) & 0xff] ^	Tsz3[s1 & 0xff] ^ tweakeyRundy[38];
	t3 = Tsz0[s3 >> 24] ^	Tsz1[(s0 >> 16) & 0xff] ^	Tsz2[(s1 >> 8) & 0xff] ^	Tsz3[s2 & 0xff] ^ tweakeyRundy[39];
	//r10 
	s0 = Tsz0[t0 >> 24] ^	Tsz1[(t1 >> 16) & 0xff] ^	Tsz2[(t2 >> 8) & 0xff] ^	Tsz3[t3 & 0xff] ^ tweakeyRundy[40];
	s1 = Tsz0[t1 >> 24] ^	Tsz1[(t2 >> 16) & 0xff] ^	Tsz2[(t3 >> 8) & 0xff] ^	Tsz3[t0 & 0xff] ^ tweakeyRundy[41];
	s2 = Tsz0[t2 >> 24] ^	Tsz1[(t3 >> 16) & 0xff] ^	Tsz2[(t0 >> 8) & 0xff] ^	Tsz3[t1 & 0xff] ^ tweakeyRundy[42];
	s3 = Tsz0[t3 >> 24] ^	Tsz1[(t0 >> 16) & 0xff] ^	Tsz2[(t1 >> 8) & 0xff] ^	Tsz3[t2 & 0xff] ^ tweakeyRundy[43];
	//r11 
	t0 = Tsz0[s0 >> 24] ^	Tsz1[(s1 >> 16) & 0xff] ^	Tsz2[(s2 >> 8) & 0xff] ^	Tsz3[s3 & 0xff] ^ tweakeyRundy[44];
	t1 = Tsz0[s1 >> 24] ^	Tsz1[(s2 >> 16) & 0xff] ^	Tsz2[(s3 >> 8) & 0xff] ^	Tsz3[s0 & 0xff] ^ tweakeyRundy[45];
	t2 = Tsz0[s2 >> 24] ^	Tsz1[(s3 >> 16) & 0xff] ^	Tsz2[(s0 >> 8) & 0xff] ^	Tsz3[s1 & 0xff] ^ tweakeyRundy[46];
	t3 = Tsz0[s3 >> 24] ^	Tsz1[(s0 >> 16) & 0xff] ^	Tsz2[(s1 >> 8) & 0xff] ^	Tsz3[s2 & 0xff] ^ tweakeyRundy[47];
	//r12 
	s0 = Tsz0[t0 >> 24] ^	Tsz1[(t1 >> 16) & 0xff] ^	Tsz2[(t2 >> 8) & 0xff] ^	Tsz3[t3 & 0xff] ^ tweakeyRundy[48];
	s1 = Tsz0[t1 >> 24] ^	Tsz1[(t2 >> 16) & 0xff] ^	Tsz2[(t3 >> 8) & 0xff] ^	Tsz3[t0 & 0xff] ^ tweakeyRundy[49];
	s2 = Tsz0[t2 >> 24] ^	Tsz1[(t3 >> 16) & 0xff] ^	Tsz2[(t0 >> 8) & 0xff] ^	Tsz3[t1 & 0xff] ^ tweakeyRundy[50];
	s3 = Tsz0[t3 >> 24] ^	Tsz1[(t0 >> 16) & 0xff] ^	Tsz2[(t1 >> 8) & 0xff] ^	Tsz3[t2 & 0xff] ^ tweakeyRundy[51];
	//r13 
	t0 = Tsz0[s0 >> 24] ^	Tsz1[(s1 >> 16) & 0xff] ^	Tsz2[(s2 >> 8) & 0xff] ^	Tsz3[s3 & 0xff] ^ tweakeyRundy[52];
	t1 = Tsz0[s1 >> 24] ^	Tsz1[(s2 >> 16) & 0xff] ^	Tsz2[(s3 >> 8) & 0xff] ^	Tsz3[s0 & 0xff] ^ tweakeyRundy[53];
	t2 = Tsz0[s2 >> 24] ^	Tsz1[(s3 >> 16) & 0xff] ^	Tsz2[(s0 >> 8) & 0xff] ^	Tsz3[s1 & 0xff] ^ tweakeyRundy[54];
	t3 = Tsz0[s3 >> 24] ^	Tsz1[(s0 >> 16) & 0xff] ^	Tsz2[(s1 >> 8) & 0xff] ^	Tsz3[s2 & 0xff] ^ tweakeyRundy[55];
	//r14 
	s0 = Tsz0[t0 >> 24] ^	Tsz1[(t1 >> 16) & 0xff] ^	Tsz2[(t2 >> 8) & 0xff] ^	Tsz3[t3 & 0xff] ^ tweakeyRundy[56];
	s1 = Tsz0[t1 >> 24] ^	Tsz1[(t2 >> 16) & 0xff] ^	Tsz2[(t3 >> 8) & 0xff] ^	Tsz3[t0 & 0xff] ^ tweakeyRundy[57];
	s2 = Tsz0[t2 >> 24] ^	Tsz1[(t3 >> 16) & 0xff] ^	Tsz2[(t0 >> 8) & 0xff] ^	Tsz3[t1 & 0xff] ^ tweakeyRundy[58];
	s3 = Tsz0[t3 >> 24] ^	Tsz1[(t0 >> 16) & 0xff] ^	Tsz2[(t1 >> 8) & 0xff] ^	Tsz3[t2 & 0xff] ^ tweakeyRundy[59];
	//r15 
	t0 = Tsz0[s0 >> 24] ^	Tsz1[(s1 >> 16) & 0xff] ^	Tsz2[(s2 >> 8) & 0xff] ^	Tsz3[s3 & 0xff] ^ tweakeyRundy[60];
	t1 = Tsz0[s1 >> 24] ^	Tsz1[(s2 >> 16) & 0xff] ^	Tsz2[(s3 >> 8) & 0xff] ^	Tsz3[s0 & 0xff] ^ tweakeyRundy[61];
	t2 = Tsz0[s2 >> 24] ^	Tsz1[(s3 >> 16) & 0xff] ^	Tsz2[(s0 >> 8) & 0xff] ^	Tsz3[s1 & 0xff] ^ tweakeyRundy[62];
	t3 = Tsz0[s3 >> 24] ^	Tsz1[(s0 >> 16) & 0xff] ^	Tsz2[(s1 >> 8) & 0xff] ^	Tsz3[s2 & 0xff] ^ tweakeyRundy[63];
	//r16 
	s0 = Tsz0[t0 >> 24] ^	Tsz1[(t1 >> 16) & 0xff] ^	Tsz2[(t2 >> 8) & 0xff] ^	Tsz3[t3 & 0xff] ^ tweakeyRundy[64];
	s1 = Tsz0[t1 >> 24] ^	Tsz1[(t2 >> 16) & 0xff] ^	Tsz2[(t3 >> 8) & 0xff] ^	Tsz3[t0 & 0xff] ^ tweakeyRundy[65];
	s2 = Tsz0[t2 >> 24] ^	Tsz1[(t3 >> 16) & 0xff] ^	Tsz2[(t0 >> 8) & 0xff] ^	Tsz3[t1 & 0xff] ^ tweakeyRundy[66];
	s3 = Tsz0[t3 >> 24] ^	Tsz1[(t0 >> 16) & 0xff] ^	Tsz2[(t1 >> 8) & 0xff] ^	Tsz3[t2 & 0xff] ^ tweakeyRundy[67];

	ustawStalaU32(szyfrogram, s0);
	ustawStalaU32(szyfrogram + 4, s1);
	ustawStalaU32(szyfrogram + 8, s2);
	ustawStalaU32(szyfrogram + 12, s3);
}


void deszyfrowanieAESzTweakiem(const unsigned char szyfrogram[16], const unsigned char klucz[], unsigned char tekstJawny[16])
{
	unsigned int s0, s1, s2, s3, t0, t1, t2, t3, tweakeyRundy[4 * 17];
	int liczbaRund;

	liczbaRund = przygotowanieDeoxysDoDeszyfrowania(tweakeyRundy, klucz, 3);

	s0 = pobierzStalaU32(szyfrogram) ^ tweakeyRundy[0];
	s1 = pobierzStalaU32(szyfrogram + 4) ^ tweakeyRundy[1];
	s2 = pobierzStalaU32(szyfrogram + 8) ^ tweakeyRundy[2];
	s3 = pobierzStalaU32(szyfrogram + 12) ^ tweakeyRundy[3];

	s0 = Tdesz0[Tsz4[(s0 >> 24)] & 0xff] ^	Tdesz1[Tsz4[(s0 >> 16) & 0xff] & 0xff] ^	Tdesz2[Tsz4[(s0 >> 8) & 0xff] & 0xff] ^		Tdesz3[Tsz4[(s0)& 0xff] & 0xff];
	s1 = Tdesz0[Tsz4[(s1 >> 24)] & 0xff] ^	Tdesz1[Tsz4[(s1 >> 16) & 0xff] & 0xff] ^	Tdesz2[Tsz4[(s1 >> 8) & 0xff] & 0xff] ^		Tdesz3[Tsz4[(s1)& 0xff] & 0xff];
	s2 = Tdesz0[Tsz4[(s2 >> 24)] & 0xff] ^	Tdesz1[Tsz4[(s2 >> 16) & 0xff] & 0xff] ^	Tdesz2[Tsz4[(s2 >> 8) & 0xff] & 0xff] ^		Tdesz3[Tsz4[(s2)& 0xff] & 0xff];
	s3 = Tdesz0[Tsz4[(s3 >> 24)] & 0xff] ^	Tdesz1[Tsz4[(s3 >> 16) & 0xff] & 0xff] ^	Tdesz2[Tsz4[(s3 >> 8) & 0xff] & 0xff] ^		Tdesz3[Tsz4[(s3)& 0xff] & 0xff];

	//r1 
	t0 = Tdesz0[s0 >> 24] ^		Tdesz1[(s3 >> 16) & 0xff] ^		Tdesz2[(s2 >> 8) & 0xff] ^	Tdesz3[s1 & 0xff] ^		tweakeyRundy[4];
	t1 = Tdesz0[s1 >> 24] ^		Tdesz1[(s0 >> 16) & 0xff] ^		Tdesz2[(s3 >> 8) & 0xff] ^	Tdesz3[s2 & 0xff] ^		tweakeyRundy[5];
	t2 = Tdesz0[s2 >> 24] ^		Tdesz1[(s1 >> 16) & 0xff] ^		Tdesz2[(s0 >> 8) & 0xff] ^	Tdesz3[s3 & 0xff] ^		tweakeyRundy[6];
	t3 = Tdesz0[s3 >> 24] ^		Tdesz1[(s2 >> 16) & 0xff] ^		Tdesz2[(s1 >> 8) & 0xff] ^	Tdesz3[s0 & 0xff] ^		tweakeyRundy[7];
	//r2 
	s0 = Tdesz0[t0 >> 24] ^		Tdesz1[(t3 >> 16) & 0xff] ^		Tdesz2[(t2 >> 8) & 0xff] ^	Tdesz3[t1 & 0xff] ^		tweakeyRundy[8];
	s1 = Tdesz0[t1 >> 24] ^		Tdesz1[(t0 >> 16) & 0xff] ^		Tdesz2[(t3 >> 8) & 0xff] ^	Tdesz3[t2 & 0xff] ^		tweakeyRundy[9];
	s2 = Tdesz0[t2 >> 24] ^		Tdesz1[(t1 >> 16) & 0xff] ^		Tdesz2[(t0 >> 8) & 0xff] ^	Tdesz3[t3 & 0xff] ^		tweakeyRundy[10];
	s3 = Tdesz0[t3 >> 24] ^		Tdesz1[(t2 >> 16) & 0xff] ^		Tdesz2[(t1 >> 8) & 0xff] ^	Tdesz3[t0 & 0xff] ^		tweakeyRundy[11];
	//r3 
	t0 = Tdesz0[s0 >> 24] ^		Tdesz1[(s3 >> 16) & 0xff] ^		Tdesz2[(s2 >> 8) & 0xff] ^	Tdesz3[s1 & 0xff] ^		tweakeyRundy[12];
	t1 = Tdesz0[s1 >> 24] ^		Tdesz1[(s0 >> 16) & 0xff] ^		Tdesz2[(s3 >> 8) & 0xff] ^	Tdesz3[s2 & 0xff] ^		tweakeyRundy[13];
	t2 = Tdesz0[s2 >> 24] ^		Tdesz1[(s1 >> 16) & 0xff] ^		Tdesz2[(s0 >> 8) & 0xff] ^	Tdesz3[s3 & 0xff] ^		tweakeyRundy[14];
	t3 = Tdesz0[s3 >> 24] ^		Tdesz1[(s2 >> 16) & 0xff] ^		Tdesz2[(s1 >> 8) & 0xff] ^	Tdesz3[s0 & 0xff] ^		tweakeyRundy[15];
	//r4 
	s0 = Tdesz0[t0 >> 24] ^		Tdesz1[(t3 >> 16) & 0xff] ^		Tdesz2[(t2 >> 8) & 0xff] ^	Tdesz3[t1 & 0xff] ^		tweakeyRundy[16];
	s1 = Tdesz0[t1 >> 24] ^		Tdesz1[(t0 >> 16) & 0xff] ^		Tdesz2[(t3 >> 8) & 0xff] ^	Tdesz3[t2 & 0xff] ^		tweakeyRundy[17];
	s2 = Tdesz0[t2 >> 24] ^		Tdesz1[(t1 >> 16) & 0xff] ^		Tdesz2[(t0 >> 8) & 0xff] ^	Tdesz3[t3 & 0xff] ^		tweakeyRundy[18];
	s3 = Tdesz0[t3 >> 24] ^		Tdesz1[(t2 >> 16) & 0xff] ^		Tdesz2[(t1 >> 8) & 0xff] ^	Tdesz3[t0 & 0xff] ^		tweakeyRundy[19];
	//r5 
	t0 = Tdesz0[s0 >> 24] ^		Tdesz1[(s3 >> 16) & 0xff] ^		Tdesz2[(s2 >> 8) & 0xff] ^	Tdesz3[s1 & 0xff] ^		tweakeyRundy[20];
	t1 = Tdesz0[s1 >> 24] ^		Tdesz1[(s0 >> 16) & 0xff] ^		Tdesz2[(s3 >> 8) & 0xff] ^	Tdesz3[s2 & 0xff] ^		tweakeyRundy[21];
	t2 = Tdesz0[s2 >> 24] ^		Tdesz1[(s1 >> 16) & 0xff] ^		Tdesz2[(s0 >> 8) & 0xff] ^	Tdesz3[s3 & 0xff] ^		tweakeyRundy[22];
	t3 = Tdesz0[s3 >> 24] ^		Tdesz1[(s2 >> 16) & 0xff] ^		Tdesz2[(s1 >> 8) & 0xff] ^	Tdesz3[s0 & 0xff] ^		tweakeyRundy[23];
	//r6 
	s0 = Tdesz0[t0 >> 24] ^		Tdesz1[(t3 >> 16) & 0xff] ^		Tdesz2[(t2 >> 8) & 0xff] ^	Tdesz3[t1 & 0xff] ^		tweakeyRundy[24];
	s1 = Tdesz0[t1 >> 24] ^		Tdesz1[(t0 >> 16) & 0xff] ^		Tdesz2[(t3 >> 8) & 0xff] ^	Tdesz3[t2 & 0xff] ^		tweakeyRundy[25];
	s2 = Tdesz0[t2 >> 24] ^		Tdesz1[(t1 >> 16) & 0xff] ^		Tdesz2[(t0 >> 8) & 0xff] ^	Tdesz3[t3 & 0xff] ^		tweakeyRundy[26];
	s3 = Tdesz0[t3 >> 24] ^		Tdesz1[(t2 >> 16) & 0xff] ^		Tdesz2[(t1 >> 8) & 0xff] ^	Tdesz3[t0 & 0xff] ^		tweakeyRundy[27];
	//r7 
	t0 = Tdesz0[s0 >> 24] ^		Tdesz1[(s3 >> 16) & 0xff] ^		Tdesz2[(s2 >> 8) & 0xff] ^	Tdesz3[s1 & 0xff] ^		tweakeyRundy[28];
	t1 = Tdesz0[s1 >> 24] ^		Tdesz1[(s0 >> 16) & 0xff] ^		Tdesz2[(s3 >> 8) & 0xff] ^	Tdesz3[s2 & 0xff] ^		tweakeyRundy[29];
	t2 = Tdesz0[s2 >> 24] ^		Tdesz1[(s1 >> 16) & 0xff] ^		Tdesz2[(s0 >> 8) & 0xff] ^	Tdesz3[s3 & 0xff] ^		tweakeyRundy[30];
	t3 = Tdesz0[s3 >> 24] ^		Tdesz1[(s2 >> 16) & 0xff] ^		Tdesz2[(s1 >> 8) & 0xff] ^	Tdesz3[s0 & 0xff] ^		tweakeyRundy[31];
	//r8 
	s0 = Tdesz0[t0 >> 24] ^		Tdesz1[(t3 >> 16) & 0xff] ^		Tdesz2[(t2 >> 8) & 0xff] ^	Tdesz3[t1 & 0xff] ^		tweakeyRundy[32];
	s1 = Tdesz0[t1 >> 24] ^		Tdesz1[(t0 >> 16) & 0xff] ^		Tdesz2[(t3 >> 8) & 0xff] ^	Tdesz3[t2 & 0xff] ^		tweakeyRundy[33];
	s2 = Tdesz0[t2 >> 24] ^		Tdesz1[(t1 >> 16) & 0xff] ^		Tdesz2[(t0 >> 8) & 0xff] ^	Tdesz3[t3 & 0xff] ^		tweakeyRundy[34];
	s3 = Tdesz0[t3 >> 24] ^		Tdesz1[(t2 >> 16) & 0xff] ^		Tdesz2[(t1 >> 8) & 0xff] ^	Tdesz3[t0 & 0xff] ^		tweakeyRundy[35];
	//r9 
	t0 = Tdesz0[s0 >> 24] ^		Tdesz1[(s3 >> 16) & 0xff] ^		Tdesz2[(s2 >> 8) & 0xff] ^	Tdesz3[s1 & 0xff] ^		tweakeyRundy[36];
	t1 = Tdesz0[s1 >> 24] ^		Tdesz1[(s0 >> 16) & 0xff] ^		Tdesz2[(s3 >> 8) & 0xff] ^	Tdesz3[s2 & 0xff] ^		tweakeyRundy[37];
	t2 = Tdesz0[s2 >> 24] ^		Tdesz1[(s1 >> 16) & 0xff] ^		Tdesz2[(s0 >> 8) & 0xff] ^	Tdesz3[s3 & 0xff] ^		tweakeyRundy[38];
	t3 = Tdesz0[s3 >> 24] ^		Tdesz1[(s2 >> 16) & 0xff] ^		Tdesz2[(s1 >> 8) & 0xff] ^	Tdesz3[s0 & 0xff] ^		tweakeyRundy[39];
	//r10 
	s0 = Tdesz0[t0 >> 24] ^		Tdesz1[(t3 >> 16) & 0xff] ^		Tdesz2[(t2 >> 8) & 0xff] ^	Tdesz3[t1 & 0xff] ^		tweakeyRundy[40];
	s1 = Tdesz0[t1 >> 24] ^		Tdesz1[(t0 >> 16) & 0xff] ^		Tdesz2[(t3 >> 8) & 0xff] ^	Tdesz3[t2 & 0xff] ^		tweakeyRundy[41];
	s2 = Tdesz0[t2 >> 24] ^		Tdesz1[(t1 >> 16) & 0xff] ^		Tdesz2[(t0 >> 8) & 0xff] ^	Tdesz3[t3 & 0xff] ^		tweakeyRundy[42];
	s3 = Tdesz0[t3 >> 24] ^		Tdesz1[(t2 >> 16) & 0xff] ^		Tdesz2[(t1 >> 8) & 0xff] ^	Tdesz3[t0 & 0xff] ^		tweakeyRundy[43];
	//r11 
	t0 = Tdesz0[s0 >> 24] ^		Tdesz1[(s3 >> 16) & 0xff] ^		Tdesz2[(s2 >> 8) & 0xff] ^	Tdesz3[s1 & 0xff] ^		tweakeyRundy[44];
	t1 = Tdesz0[s1 >> 24] ^		Tdesz1[(s0 >> 16) & 0xff] ^		Tdesz2[(s3 >> 8) & 0xff] ^	Tdesz3[s2 & 0xff] ^		tweakeyRundy[45];
	t2 = Tdesz0[s2 >> 24] ^		Tdesz1[(s1 >> 16) & 0xff] ^		Tdesz2[(s0 >> 8) & 0xff] ^	Tdesz3[s3 & 0xff] ^		tweakeyRundy[46];
	t3 = Tdesz0[s3 >> 24] ^		Tdesz1[(s2 >> 16) & 0xff] ^		Tdesz2[(s1 >> 8) & 0xff] ^	Tdesz3[s0 & 0xff] ^		tweakeyRundy[47];
	//r12 
	s0 = Tdesz0[t0 >> 24] ^		Tdesz1[(t3 >> 16) & 0xff] ^		Tdesz2[(t2 >> 8) & 0xff] ^	Tdesz3[t1 & 0xff] ^		tweakeyRundy[48];
	s1 = Tdesz0[t1 >> 24] ^		Tdesz1[(t0 >> 16) & 0xff] ^		Tdesz2[(t3 >> 8) & 0xff] ^	Tdesz3[t2 & 0xff] ^		tweakeyRundy[49];
	s2 = Tdesz0[t2 >> 24] ^		Tdesz1[(t1 >> 16) & 0xff] ^		Tdesz2[(t0 >> 8) & 0xff] ^	Tdesz3[t3 & 0xff] ^		tweakeyRundy[50];
	s3 = Tdesz0[t3 >> 24] ^		Tdesz1[(t2 >> 16) & 0xff] ^		Tdesz2[(t1 >> 8) & 0xff] ^	Tdesz3[t0 & 0xff] ^		tweakeyRundy[51];
	//r13 
	t0 = Tdesz0[s0 >> 24] ^		Tdesz1[(s3 >> 16) & 0xff] ^		Tdesz2[(s2 >> 8) & 0xff] ^	Tdesz3[s1 & 0xff] ^		tweakeyRundy[52];
	t1 = Tdesz0[s1 >> 24] ^		Tdesz1[(s0 >> 16) & 0xff] ^		Tdesz2[(s3 >> 8) & 0xff] ^	Tdesz3[s2 & 0xff] ^		tweakeyRundy[53];
	t2 = Tdesz0[s2 >> 24] ^		Tdesz1[(s1 >> 16) & 0xff] ^		Tdesz2[(s0 >> 8) & 0xff] ^	Tdesz3[s3 & 0xff] ^		tweakeyRundy[54];
	t3 = Tdesz0[s3 >> 24] ^		Tdesz1[(s2 >> 16) & 0xff] ^		Tdesz2[(s1 >> 8) & 0xff] ^	Tdesz3[s0 & 0xff] ^		tweakeyRundy[55];
	//r14 
	s0 = Tdesz0[t0 >> 24] ^		Tdesz1[(t3 >> 16) & 0xff] ^		Tdesz2[(t2 >> 8) & 0xff] ^	Tdesz3[t1 & 0xff] ^		tweakeyRundy[56];
	s1 = Tdesz0[t1 >> 24] ^		Tdesz1[(t0 >> 16) & 0xff] ^		Tdesz2[(t3 >> 8) & 0xff] ^	Tdesz3[t2 & 0xff] ^		tweakeyRundy[57];
	s2 = Tdesz0[t2 >> 24] ^		Tdesz1[(t1 >> 16) & 0xff] ^		Tdesz2[(t0 >> 8) & 0xff] ^	Tdesz3[t3 & 0xff] ^		tweakeyRundy[58];
	s3 = Tdesz0[t3 >> 24] ^		Tdesz1[(t2 >> 16) & 0xff] ^		Tdesz2[(t1 >> 8) & 0xff] ^	Tdesz3[t0 & 0xff] ^		tweakeyRundy[59];
	//r15 
	t0 = Tdesz0[s0 >> 24] ^		Tdesz1[(s3 >> 16) & 0xff] ^		Tdesz2[(s2 >> 8) & 0xff] ^	Tdesz3[s1 & 0xff] ^		tweakeyRundy[60];
	t1 = Tdesz0[s1 >> 24] ^		Tdesz1[(s0 >> 16) & 0xff] ^		Tdesz2[(s3 >> 8) & 0xff] ^	Tdesz3[s2 & 0xff] ^		tweakeyRundy[61];
	t2 = Tdesz0[s2 >> 24] ^		Tdesz1[(s1 >> 16) & 0xff] ^		Tdesz2[(s0 >> 8) & 0xff] ^	Tdesz3[s3 & 0xff] ^		tweakeyRundy[62];
	t3 = Tdesz0[s3 >> 24] ^		Tdesz1[(s2 >> 16) & 0xff] ^		Tdesz2[(s1 >> 8) & 0xff] ^	Tdesz3[s0 & 0xff] ^		tweakeyRundy[63];
	//r16 
	s0 = Tdesz0[t0 >> 24] ^		Tdesz1[(t3 >> 16) & 0xff] ^		Tdesz2[(t2 >> 8) & 0xff] ^	Tdesz3[t1 & 0xff] ^		tweakeyRundy[64];
	s1 = Tdesz0[t1 >> 24] ^		Tdesz1[(t0 >> 16) & 0xff] ^		Tdesz2[(t3 >> 8) & 0xff] ^	Tdesz3[t2 & 0xff] ^		tweakeyRundy[65];
	s2 = Tdesz0[t2 >> 24] ^		Tdesz1[(t1 >> 16) & 0xff] ^		Tdesz2[(t0 >> 8) & 0xff] ^	Tdesz3[t3 & 0xff] ^		tweakeyRundy[66];
	s3 = Tdesz0[t3 >> 24] ^		Tdesz1[(t2 >> 16) & 0xff] ^		Tdesz2[(t1 >> 8) & 0xff] ^	Tdesz3[t0 & 0xff] ^		tweakeyRundy[67];

	s0 = Tsz0[Tdesz4[(s0 >> 24)] & 0xff] ^	Tsz1[Tdesz4[(s0 >> 16) & 0xff] & 0xff] ^	Tsz2[Tdesz4[(s0 >> 8) & 0xff] & 0xff] ^		Tsz3[Tdesz4[(s0)& 0xff] & 0xff];
	s1 = Tsz0[Tdesz4[(s1 >> 24)] & 0xff] ^	Tsz1[Tdesz4[(s1 >> 16) & 0xff] & 0xff] ^	Tsz2[Tdesz4[(s1 >> 8) & 0xff] & 0xff] ^		Tsz3[Tdesz4[(s1)& 0xff] & 0xff];
	s2 = Tsz0[Tdesz4[(s2 >> 24)] & 0xff] ^	Tsz1[Tdesz4[(s2 >> 16) & 0xff] & 0xff] ^	Tsz2[Tdesz4[(s2 >> 8) & 0xff] & 0xff] ^		Tsz3[Tdesz4[(s2)& 0xff] & 0xff];
	s3 = Tsz0[Tdesz4[(s3 >> 24)] & 0xff] ^	Tsz1[Tdesz4[(s3 >> 16) & 0xff] & 0xff] ^	Tsz2[Tdesz4[(s3 >> 8) & 0xff] & 0xff] ^		Tsz3[Tdesz4[(s3)& 0xff] & 0xff];

	ustawStalaU32(tekstJawny, s0);
	ustawStalaU32(tekstJawny + 4, s1);
	ustawStalaU32(tekstJawny + 8, s2);
	ustawStalaU32(tekstJawny + 12, s3);
}

static void ustawNonceWTweaku(unsigned char *tweak, const unsigned char *nonce)
{
	tweak[0] = (tweak[0] & 0xf0)		^	(nonce[0] >> 4);
	tweak[1] = (nonce[0] & 0xf) << 4	^	(nonce[1] >> 4);
	tweak[2] = (nonce[1] & 0xf) << 4	^	(nonce[2] >> 4);
	tweak[3] = (nonce[2] & 0xf) << 4	^	(nonce[3] >> 4);
	tweak[4] = (nonce[3] & 0xf) << 4	^	(nonce[4] >> 4);
	tweak[5] = (nonce[4] & 0xf) << 4	^	(nonce[5] >> 4);
	tweak[6] = (nonce[5] & 0xf) << 4	^	(nonce[6] >> 4);
	tweak[7] = (nonce[6] & 0xf) << 4	^	(nonce[7] >> 4);
	tweak[8] = (nonce[7] & 0xf) << 4;
}

static void ustawNumerBlokuWTweaku(unsigned char *tweak, const unsigned long long numerBloku)
{
	tweak[ 8] =	(tweak[8] & 0xf0) ^ ((numerBloku >> 56ULL) & 0xf);
	tweak[ 9] =	((numerBloku >> 48ULL)	& 0xff);
	tweak[10] = ((numerBloku >> 40ULL)	& 0xff);
	tweak[11] = ((numerBloku >> 32ULL)	& 0xff);
	tweak[12] = ((numerBloku >> 24ULL)	& 0xff);
	tweak[13] = ((numerBloku >> 16ULL)	& 0xff);
	tweak[14] = ((numerBloku >> 8ULL)	& 0xff);
	tweak[15] = ((numerBloku >> 0ULL)	& 0xff);
}

static void ustawStanWTweaku(unsigned char *tweak, const unsigned char wartosc)
{
	tweak[0] = (tweak[0] & 0xf) ^ wartosc;
}

static void ustawTweakaWTweakey(unsigned char *tweakey, unsigned char *tweak)
{
	memcpy(tweakey + 32, tweak, 16); //dla D-BC-256: memcpy(tweakey + 16, tweak, 16);
}

static int memcmpStale(const void * a, const void *b, const unsigned long rozmiar)
{
	unsigned long i;
	unsigned char wynik = 0;
	const unsigned char *_a = (const unsigned char *)a;
	const unsigned char *_b = (const unsigned char *)b;

	for (i = 0; i < rozmiar; i++)
		wynik |= _a[i] ^ _b[i];

	return wynik;
}

static void wartosciXOR(unsigned char *wartosc1, const unsigned char *wartosc2)
{
	int i;
	for (i = 0; i<16; i++)
		wartosc1[i] ^= wartosc2[i];
}


void szyfrowanieDeoxys(unsigned char *skojarzoneDane, unsigned long dlugoscSkojarzonychDanych, unsigned char *tekstJawny, unsigned long dlugoscTekstuJawnego, unsigned char *klucz, unsigned char *nonce, unsigned char *szyfrogram, unsigned long dlugoscSzyfrogramu)
{
	unsigned long long i, j;
	unsigned char tweak[16], tweakey[384 / 8], auth[16];
	unsigned char ostatniBlok[16], sumaKontrolna[16], final[16], zerowyBlok[16], pad[16], zmiennaPomocnicza[16];

	memset(tweak, 0, sizeof(tweak)); 

	memcpy(tweakey, klucz, 32);

	memset(auth, 0, 16);

	if (dlugoscSkojarzonychDanych)
	{
		ustawStanWTweaku(tweak, najbardziejZnaczacyBitSkojarzonychDanych);

		i = 0;
		while (16 * (i + 1) <= dlugoscSkojarzonychDanych)
		{
			ustawNumerBlokuWTweaku(tweak, i);
			ustawTweakaWTweakey(tweakey, tweak);
			szyfrowanieAESzTweakiem(skojarzoneDane + 16 * i, tweakey, zmiennaPomocnicza);

			wartosciXOR(auth, zmiennaPomocnicza);

			i++;
		}

		if (dlugoscSkojarzonychDanych > 16 * i)
		{
			memset(ostatniBlok, 0, 16);
			memcpy(ostatniBlok, skojarzoneDane + 16 * i, dlugoscSkojarzonychDanych - 16 * i);
			ostatniBlok[dlugoscSkojarzonychDanych - 16 * i] = 0x80;

			ustawStanWTweaku(tweak, najbardziejZnaczacyBitSkojarzonychDanychOstatni);
			ustawNumerBlokuWTweaku(tweak, i);
			ustawTweakaWTweakey(tweakey, tweak);
			szyfrowanieAESzTweakiem(ostatniBlok, tweakey, zmiennaPomocnicza);

			wartosciXOR(auth, zmiennaPomocnicza);
		}
	}

	memset(tweak, 0, sizeof(tweak));
	ustawNonceWTweaku(tweak, nonce);

	memset(sumaKontrolna, 0, 16);
	ustawStanWTweaku(tweak, najbardziejZnaczacyBitTekstuJawnego);
	i = 0;
	while (16 * (i + 1) <= dlugoscTekstuJawnego)
	{
		wartosciXOR(sumaKontrolna, tekstJawny + 16 * i);
		ustawNumerBlokuWTweaku(tweak, i);
		ustawTweakaWTweakey(tweakey, tweak);
		szyfrowanieAESzTweakiem(tekstJawny + 16 * i, tweakey, szyfrogram + 16 * i);
		i++;
	}

	if (dlugoscTekstuJawnego > 16 * i)
	{
		memset(ostatniBlok, 0, 16);
		memcpy(ostatniBlok, tekstJawny + 16 * i, dlugoscTekstuJawnego - 16 * i);
		ostatniBlok[dlugoscTekstuJawnego - 16 * i] = 0x80;
		wartosciXOR(sumaKontrolna, ostatniBlok);

		memset(zerowyBlok, 0, 16);

		ustawStanWTweaku(tweak, najbardziejZnaczacyBitTekstuJawnegoNiezerowy);
		ustawNumerBlokuWTweaku(tweak, i);
		ustawTweakaWTweakey(tweakey, tweak);
		szyfrowanieAESzTweakiem(zerowyBlok, tweakey, pad);

		for (j = 0; j<dlugoscTekstuJawnego - 16 * i; j++)
			szyfrogram[16 * i + j] = ostatniBlok[j] ^ pad[j];
		ustawStanWTweaku(tweak, najbardziejZnaczacyBitSumyKontrolnejNiepelnej);
		ustawTweakaWTweakey(tweakey, tweak);
		szyfrowanieAESzTweakiem(sumaKontrolna, tweakey, final);
	}
	else
	{
		ustawNumerBlokuWTweaku(tweak, i);
		ustawStanWTweaku(tweak, najbardziejZnaczacyBitSumyKontrolnej);
		ustawTweakaWTweakey(tweakey, tweak);
		szyfrowanieAESzTweakiem(sumaKontrolna, tweakey, final);
	}

	for (i = 0; i<16; i++)
		szyfrogram[dlugoscTekstuJawnego + i] = final[i] ^ auth[i];

	dlugoscSzyfrogramu = dlugoscTekstuJawnego + 16;
}


int deszyfrowanieDeoxys(unsigned char *skojarzoneDane, unsigned long dlugoscSkojarzonychDanych, unsigned char *tekstJawny, unsigned long dlugoscTekstuJawnego, unsigned char *klucz, unsigned char *nonce, unsigned char *szyfrogram, unsigned long dlugoscSzyfrogramu)
{
	unsigned long long i, j;
	unsigned char tweak[16], tweakey[384 / 8], auth[16], ostatniBlok[16], sumaKontrolna[16], final[16], zerowyBlok[16], pad[16], tag[16], zmiennaPomocnicza[16];

	memcpy(tag, szyfrogram + dlugoscSzyfrogramu - 16, 16);
	dlugoscSzyfrogramu -= 16;

	memset(tweak, 0, sizeof(tweak));
	memcpy(tweakey, klucz, 32);
	memset(auth, 0, 16);

	if (dlugoscSkojarzonychDanych)
	{
		ustawStanWTweaku(tweak, najbardziejZnaczacyBitSkojarzonychDanych);
		i = 0;
		while (16 * (i + 1) <= dlugoscSkojarzonychDanych)
		{
			ustawNumerBlokuWTweaku(tweak, i);
			ustawTweakaWTweakey(tweakey, tweak);
			szyfrowanieAESzTweakiem(skojarzoneDane + 16 * i, tweakey, zmiennaPomocnicza);
			wartosciXOR(auth, zmiennaPomocnicza);
			i++;
		}

		if (dlugoscSkojarzonychDanych > 16 * i)
		{
			memset(ostatniBlok, 0, 16);
			memcpy(ostatniBlok, skojarzoneDane + 16 * i, dlugoscSkojarzonychDanych - 16 * i);
			ostatniBlok[dlugoscSkojarzonychDanych - 16 * i] = 0x80;
			ustawStanWTweaku(tweak, najbardziejZnaczacyBitSkojarzonychDanychOstatni);
			ustawNumerBlokuWTweaku(tweak, i);
			ustawTweakaWTweakey(tweakey, tweak);
			szyfrowanieAESzTweakiem(ostatniBlok, tweakey, zmiennaPomocnicza);
			wartosciXOR(auth, zmiennaPomocnicza);
		}
	}

	memset(tweak, 0, sizeof(tweak));
	ustawNonceWTweaku(tweak, nonce);
	memset(sumaKontrolna, 0, 16);
	ustawStanWTweaku(tweak, najbardziejZnaczacyBitTekstuJawnego);
	i = 0;
	while (16 * (i + 1) <= dlugoscSzyfrogramu)
	{
		ustawTweakaWTweakey(tweakey, tweak);
		ustawNumerBlokuWTweaku(tweak, i);
		ustawTweakaWTweakey(tweakey, tweak);
		deszyfrowanieAESzTweakiem(szyfrogram + 16 * i, tweakey, tekstJawny + 16 * i);
		wartosciXOR(sumaKontrolna, tekstJawny + 16 * i);
		i++;
	}

	if (dlugoscSzyfrogramu == 16 * i)
	{
		ustawNumerBlokuWTweaku(tweak, i);
		ustawStanWTweaku(tweak, najbardziejZnaczacyBitSumyKontrolnej);
		ustawTweakaWTweakey(tweakey, tweak);
		szyfrowanieAESzTweakiem(sumaKontrolna, tweakey, final);
		wartosciXOR(final, auth);

		if (0 != memcmpStale(final, tag, sizeof(tag)))
		{
			memset(tekstJawny, 0, dlugoscSzyfrogramu);
			return -1;
		}
	}
	else
	{
		memset(zerowyBlok, 0, 16);

		ustawStanWTweaku(tweak, najbardziejZnaczacyBitTekstuJawnegoNiezerowy);
		ustawNumerBlokuWTweaku(tweak, i);
		ustawTweakaWTweakey(tweakey, tweak);

		szyfrowanieAESzTweakiem(zerowyBlok, tweakey, pad);

		memset(ostatniBlok, 0, 16);
		memcpy(ostatniBlok, szyfrogram + 16 * i, dlugoscSzyfrogramu - 16 * i);
		memset(pad + dlugoscSzyfrogramu - 16 * i, 0, 16 - (dlugoscSzyfrogramu - 16 * i));
		wartosciXOR(ostatniBlok, pad);
		ostatniBlok[dlugoscSzyfrogramu - 16 * i] = 0x80;

		for (j = 0; j<dlugoscSzyfrogramu - 16 * i; j++)
			tekstJawny[16 * i + j] = ostatniBlok[j];

		wartosciXOR(sumaKontrolna, ostatniBlok);

		ustawStanWTweaku(tweak, najbardziejZnaczacyBitSumyKontrolnejNiepelnej);
		ustawTweakaWTweakey(tweakey, tweak);
		szyfrowanieAESzTweakiem(sumaKontrolna, tweakey, final);
		wartosciXOR(final, auth);

		if (0 != memcmpStale(final, tag, sizeof(tag)))
		{
			memset(tekstJawny, 0, dlugoscSzyfrogramu);
			return -1;
		}
	}
	dlugoscTekstuJawnego = dlugoscSzyfrogramu;
	return 0;
}