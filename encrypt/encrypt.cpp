// encrypt.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

int _tmain(int argc, _TCHAR* argv[])
{
	char encodedstr[30];
	uint16_t inmfrweek, inmfryear, inexpyear, inexpmonth, inexpday, insn;
	uint16_t outmfrweek, outmfryear, outexpyear, outexpmonth, outexpday, outsn;
	time_t t;
	struct tm *timeStruct;

	t = time(0);
	timeStruct = localtime(&t);

	puts("Encryption test");
	printf("Today's Date: %d-%02d-%02d\n", 1900+timeStruct->tm_year, 1+timeStruct->tm_mon, timeStruct->tm_mday);

	while (1) {
		puts("Enter mfr week, mfr year, exp day, exp mon, exp year, sn ");
		scanf("%d,%d,%d,%d,%d,%d", &inmfrweek, &inmfryear, &inexpday, &inexpmonth, &inexpyear, &insn);
		printf("Got %d,%d,%d,%d,%d,%d\n", inmfrweek, inmfryear, inexpday, inexpmonth, inexpyear, insn);

		if (!EncodeSN(encodedstr, inmfrweek, inmfryear, inexpday, inexpmonth, inexpyear, insn))
			puts("Error Encoding\n");
		else {
			printf("Encoded string = |%s|\n", encodedstr);
			if (!DecodeSN(encodedstr, &outmfrweek, &outmfryear, &outexpday, &outexpmonth, &outexpyear, &outsn))
				puts("Error Decoding\n");
			else
				printf("Decoded values %d,%d,%d,%d,%d,%d\n", outmfrweek, outmfryear, outexpday, outexpmonth, outexpyear, outsn);
		}
	}

	return 0;
}

