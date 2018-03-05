#include "stdafx.h"

// Konrad Niedüwiedzki
// OCB-AES256-TAGLEN128 z AES-256 w úrodku
// Wersja OCB3

typedef unsigned char blok[16];

#define flagaSzyfrowania	1
#define flagaDeszyfrowania	0
#define bajtyKlucza			32
#define bajtyTaga			16
#define bajtyNonce			12

static void podwojenieBlokow(blok blokWyjscia, blok blokWejscia)
{
	unsigned char zmiennaPomocnicza = blokWejscia[0];
	unsigned i;
	for (i = 0; i<15; i++)
		blokWyjscia[i] = (blokWejscia[i] << 1) | (blokWejscia[i + 1] >> 7);
	blokWyjscia[15] = (blokWejscia[15] << 1) ^ ((zmiennaPomocnicza >> 7) * 135);
}

static void obliczenieL_i(blok l, blok l_dolar, unsigned i)
{
	podwojenieBlokow(l, l_dolar);
	for (; (i & 1) == 0; i >>= 1)
		podwojenieBlokow(l, l);
}

static void xorBlokow(blok c, blok a, blok b)
{
	unsigned i;
	for (i = 0; i<16; i++)
		c[i] = a[i] ^ b[i];
}


static void HASH(blok wynik, unsigned char *klucz, unsigned char *skojarzoneDane, unsigned long long bajtySkojarzonychDanych)
{
	blok l_gwiazdka, l_dolar, offset, suma, zmiennaPomocnicza;
	AES_KEY aes_klucz;
	unsigned i;

	memset(zmiennaPomocnicza, 0, 16);
	AES_set_encrypt_key(klucz, bajtyKlucza * 8, &aes_klucz);
	AES_encrypt(zmiennaPomocnicza, l_gwiazdka, &aes_klucz);
	podwojenieBlokow(l_dolar, l_gwiazdka);
	memset(offset, 0, 16);
	memset(suma, 0, 16);

	for (i = 1; i <= (bajtySkojarzonychDanych / 16); i++, skojarzoneDane = skojarzoneDane + 16)
	{
		obliczenieL_i(zmiennaPomocnicza, l_dolar, i);
		xorBlokow(offset, offset, zmiennaPomocnicza);
		xorBlokow(zmiennaPomocnicza, offset, skojarzoneDane);
		AES_encrypt(zmiennaPomocnicza, zmiennaPomocnicza, &aes_klucz);
		xorBlokow(suma, suma, zmiennaPomocnicza);
	}

	bajtySkojarzonychDanych = (bajtySkojarzonychDanych % 16);
	if (bajtySkojarzonychDanych > 0)
	{
		xorBlokow(offset, offset, l_gwiazdka);
		memset(zmiennaPomocnicza, 0, 16);
		memcpy(zmiennaPomocnicza, skojarzoneDane, bajtySkojarzonychDanych);
		zmiennaPomocnicza[bajtySkojarzonychDanych] = 0x80;
		xorBlokow(zmiennaPomocnicza, offset, zmiennaPomocnicza);
		AES_encrypt(zmiennaPomocnicza, zmiennaPomocnicza, &aes_klucz);
		xorBlokow(suma, zmiennaPomocnicza, suma);
	}
	memcpy(wynik, suma, 16);
}


static int OCB(unsigned char *szyfrogram, unsigned char *klucz, unsigned char *wejscieNonce, unsigned char *skojarzoneDane, unsigned long long bajtySkojarzonychDanych, unsigned char *tekstJawny, unsigned long long bajtyTekstuJawnego, int flagaAkcji)
{
	blok l_gwiazdka, l_dolar, suma, offset, kGorne, pad, nonce, tag, zmiennaPomocnicza, skojarzoneDane_hash;
	AES_KEY kluczSzyfrowaniaAES, kluczDeszyfrowaniaAES;
	unsigned podloga, przesuniecieBajtowe, przesuniecieBitowe, i;
	unsigned char rozciagniecie[24];

	if (!flagaAkcji)
	{
		if (bajtyTekstuJawnego < bajtyTaga)
			return -1;
		bajtyTekstuJawnego -= bajtyTaga;
		AES_set_decrypt_key(klucz, bajtyKlucza * 8, &kluczDeszyfrowaniaAES);
	}
	AES_set_encrypt_key(klucz, bajtyKlucza * 8, &kluczSzyfrowaniaAES);

	memset(zmiennaPomocnicza, 0, 16);
	AES_encrypt(zmiennaPomocnicza, l_gwiazdka, &kluczSzyfrowaniaAES);
	podwojenieBlokow(l_dolar, l_gwiazdka);
	memset(nonce, 0, 16);
	memcpy(&nonce[16 - bajtyNonce], wejscieNonce, bajtyNonce);

	nonce[0] = (unsigned char)(((bajtyTaga * 8) % 128) << 1);
	nonce[16 - bajtyNonce - 1] |= 0x01;
	podloga = nonce[15] & 0x3F;
	nonce[15] &= 0xC0;
	AES_encrypt(nonce, kGorne, &kluczSzyfrowaniaAES);
	memcpy(rozciagniecie, kGorne, 16);
	memcpy(zmiennaPomocnicza, &kGorne[1], 8);
	xorBlokow(zmiennaPomocnicza, zmiennaPomocnicza, kGorne);
	memcpy(&rozciagniecie[16], zmiennaPomocnicza, 8);
	przesuniecieBajtowe = podloga / 8;
	przesuniecieBitowe = podloga % 8;
	if (przesuniecieBitowe != 0)
		for (i = 0; i<16; i++)
			offset[i] = (rozciagniecie[i + przesuniecieBajtowe] << przesuniecieBitowe) | (rozciagniecie[i + przesuniecieBajtowe + 1] >> (8 - przesuniecieBitowe));
	else
		for (i = 0; i<16; i++)
			offset[i] = rozciagniecie[i + przesuniecieBajtowe];
	memset(suma, 0, 16);
	HASH(skojarzoneDane_hash, klucz, skojarzoneDane, bajtySkojarzonychDanych);

	for (i = 1; i <= bajtyTekstuJawnego / 16; i++, tekstJawny = tekstJawny + 16, szyfrogram = szyfrogram + 16)
	{
		obliczenieL_i(zmiennaPomocnicza, l_dolar, i);
		xorBlokow(offset, offset, zmiennaPomocnicza);
		xorBlokow(zmiennaPomocnicza, offset, tekstJawny);
		if (flagaAkcji)
		{
			xorBlokow(suma, tekstJawny, suma);
			AES_encrypt(zmiennaPomocnicza, zmiennaPomocnicza, &kluczSzyfrowaniaAES);
			xorBlokow(szyfrogram, offset, zmiennaPomocnicza);
		}
		else
		{
			AES_decrypt(zmiennaPomocnicza, zmiennaPomocnicza, &kluczDeszyfrowaniaAES);
			xorBlokow(szyfrogram, offset, zmiennaPomocnicza);
			xorBlokow(suma, szyfrogram, suma);
		}
	}


	bajtyTekstuJawnego = bajtyTekstuJawnego % 16;
	if (bajtyTekstuJawnego > 0)
	{
		xorBlokow(offset, offset, l_gwiazdka);
		AES_encrypt(offset, pad, &kluczSzyfrowaniaAES);

		if (flagaAkcji)
		{
			memset(zmiennaPomocnicza, 0, 16);
			memcpy(zmiennaPomocnicza, tekstJawny, bajtyTekstuJawnego);
			zmiennaPomocnicza[bajtyTekstuJawnego] = 0x80;
			xorBlokow(suma, zmiennaPomocnicza, suma);
			xorBlokow(pad, zmiennaPomocnicza, pad);
			memcpy(szyfrogram, pad, bajtyTekstuJawnego);
			szyfrogram = szyfrogram + bajtyTekstuJawnego;
		}
		else
		{
			memcpy(zmiennaPomocnicza, pad, 16);
			memcpy(zmiennaPomocnicza, tekstJawny, bajtyTekstuJawnego);
			xorBlokow(zmiennaPomocnicza, pad, zmiennaPomocnicza);
			zmiennaPomocnicza[bajtyTekstuJawnego] = 0x80;
			memcpy(szyfrogram, zmiennaPomocnicza, bajtyTekstuJawnego);
			xorBlokow(suma, zmiennaPomocnicza, suma);
			tekstJawny = tekstJawny + bajtyTekstuJawnego;
		}
	}
	xorBlokow(zmiennaPomocnicza, suma, offset);
	xorBlokow(zmiennaPomocnicza, zmiennaPomocnicza, l_dolar);
	AES_encrypt(zmiennaPomocnicza, tag, &kluczSzyfrowaniaAES);
	xorBlokow(tag, skojarzoneDane_hash, tag);

	if (flagaAkcji)
	{
		memcpy(szyfrogram, tag, bajtyTaga);
		return 0;
	}
	else
		return (memcmp(tekstJawny, tag, bajtyTaga) ? -1 : 0);
}

void szyfrowanieOCB(unsigned char *skojarzoneDane, unsigned long long dlugoscSkojarzonychDanych, unsigned char *tekstJawny, unsigned long long dlugoscTekstuJawnego, unsigned char *klucz, unsigned char *publicznyNonce, unsigned char *szyfrogram, unsigned long long dlugoscSzyfrogramu, unsigned char *sekretnyNonce)
{
	dlugoscSzyfrogramu = dlugoscTekstuJawnego + bajtyTaga;
	OCB(szyfrogram, (unsigned char *)klucz, (unsigned char *)publicznyNonce, (unsigned char *)skojarzoneDane, (unsigned) dlugoscSkojarzonychDanych, (unsigned char *)tekstJawny, dlugoscTekstuJawnego, flagaSzyfrowania);
}

int deszyfrowanieOCB(unsigned char *skojarzoneDane, unsigned long long dlugoscSkojarzonychDanych, unsigned char *tekstJawny, unsigned long long dlugoscTekstuJawnego, unsigned char *klucz, unsigned char *publicznyNonce, unsigned char *szyfrogram, unsigned long long dlugoscSzyfrogramu, unsigned char *sekretnyNonce)
{
	dlugoscTekstuJawnego = dlugoscSzyfrogramu - bajtyTaga;
	return OCB(tekstJawny, (unsigned char *)klucz, (unsigned char *)publicznyNonce, (unsigned char *)skojarzoneDane, (unsigned) dlugoscSkojarzonychDanych, (unsigned char *)szyfrogram, dlugoscSzyfrogramu, flagaDeszyfrowania);
}
