#include "stdafx.h"
/*
These functions will encode and decode a Flagger mobile serial number and manufacturing date.
written: Glenn Stephens, 11 Feb 2021
copyright: East Coast Racing Technologies, Inc. d/b/a Flagger
*/

#define ENCODEBASE	36		// encode using base 36 to use all uppercase alphanumeric tokens
 
// returns alphanumeric symbol '0'-'9', 'A'-'Z' for a given value
char GetBaseSymbol(uint8_t val) {
	if (val < 10)
		return (char) ('0' + val);
	else if (val < ENCODEBASE)
		return (char) ('A' + val - 10);
	else
		return 0;
}

// converts base 36 symbol to numeric value
uint8_t GetBaseNumber(char c) {
	if (c >= '0' && c <= '9')
		return(c - '0');
	else if (c >= 'A' && c <= 'Z')
		return(c - 'A' + 10);
	else
		return 0;
}

// utility function to reverse a string 
void strev(char *str) { 
    int len = strlen(str); 
    int i; 
    for (i = 0; i < len/2; i++) { 
        char temp = str[i]; 
        str[i] = str[len-i-1]; 
        str[len-i-1] = temp; 
    } 
}

// converts number to zero filled width digit string in base 36
void EncodeNumber(char *str, uint8_t width, uint16_t inputNum) {
	int index = 0;

	if (0 == inputNum)
		str[index++] = '0';
	else {
    // Convert input number to given base by repeatedly 
		// dividing it by base and taking remainder 
		while (inputNum > 0) { 
			str[index++] = GetBaseSymbol(inputNum % ENCODEBASE); 
			inputNum = inputNum / ENCODEBASE; 
		}
	}
	while (index < width)
		str[index++] = '0';

	str[index] = '\0'; 
  
	// Reverse the result 
	strev(str); 
}

// converts string in base 36 to number
uint16_t DecodeNumber(char *str) {
	int len = strlen(str);
	uint16_t n;

	n = 0;
	if (0 != len)
		// go char by char in reverse order to build up the number
		for (int i = 0; i < len; i++)
			n = n*ENCODEBASE + GetBaseNumber(str[i]);

	return (n);
}

// inverts base 36 digits in a string
// 0 = Z, 1 = Y, ...
void InvertDigits(char *s) {
	while ('\0' != *s) {
		*s = GetBaseSymbol(ENCODEBASE - 1 - GetBaseNumber(*s));
		++s;
	}
}

void SwapChar(char *a, char *b) {
	char temp = *a;
	*a = *b;
	*b = temp;
}

// this swaps characters and inverts the base 36 digits for the update request (9 characters)
// calling again will undo the mix
void MixSN(char *s) {

	// swap some digits for obfuscation
	SwapChar(&(s[0]), &(s[4]));
	SwapChar(&(s[1]), &(s[6]));
	SwapChar(&(s[2]), &(s[8]));
	SwapChar(&(s[5]), &(s[7]));

	InvertDigits(s);
}

// this swaps characters and inverts the base 36 digits for the service authorization (8 characters)
// calling again will undo the mix
void MixAuth(char *s) {

	// swap some digits for obfuscation
	SwapChar(&(s[0]), &(s[6]));
	SwapChar(&(s[1]), &(s[4]));
	SwapChar(&(s[2]), &(s[5]));
	SwapChar(&(s[3]), &(s[7]));

}

// Converts day mon year to days since jan 1, 2020
uint16_t CalcDays(uint8_t day, uint8_t mon, uint16_t year) {
	const uint8_t calendar[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	uint32_t daycount = 0;

	if ((year%4) && (mon > 2)) daycount++;				// If leap year and already past feb add a day
	daycount += day;															// Add on the days into this month

	// add on the days this year up to last month
	mon--;
	for (; mon > 0; mon--) daycount += calendar[mon-1];	// Calendar is zero indexed, mon is 1 indexed

	--year;												// Start with last year

	// Add on the days in the previous years
	while (year >= 2020) {
		if (year%4)						  // Fix leap year
			daycount += 365;
		else										// Evenly divisible by 4 so leap year
			daycount += 366;
		--year;
	}

	return (daycount);
}

// Day 1 = Jan 1, 2020
// returns day of month (1-31), month (jan = 1), and year
void CalcDate(uint16_t *day, uint16_t *mon, uint16_t *year, uint16_t days) {
	uint8_t calendar[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

	*year = 2020;

	while (days > 365 ) {
		// Fix leap year
		if (!(*year%4) && days == 366) break;
		days -= ((*year % 4)? 365 : 366);
		++*year;
	}

	if (!(*year%4)) calendar[1] = 29;	// Fix leap year

	*mon = 0;

	while (days > calendar[*mon])	{
		days -= calendar[*mon];
		++*mon;
	}

	++*mon;
	*day = days;
}

// calculates a base 36 additive checksum for the input string
char GetCheckChar(char *s) {
	uint8_t x = 0;
	
	while ('\0' != *s) {
		x = x+GetBaseNumber(*s);
		x = x % 36;
		++s;
	}

	x = GetBaseSymbol(x);
	return x;
}

// encodes serial number, mfr date, and expiration date into snstr
// this is done on the Flagger mobile device
// month is 1-12, year is actual year > 2020 and < 2055
// sn is unsigned 16 bit 1-42356
// expiration date is day (1-31), month (1-12), year (2020 - 2055)
// manufacture week is 1-52, year is 2020-2055
// returns false on error
// return string is encoded as 9 character string (before scrambling):
// WWSSSXXXC where WW is week of manufacture since 2020, SSS is serial number, XXX is expiration day since 2020, and C is additive checksum
bool EncodeSN(char *snstr, uint16_t mweek, uint16_t myear, uint16_t eday, uint16_t emon, uint16_t eyear, uint16_t sn) {
	uint16_t expday;
	char checkchar;

	if (mweek < 1 || mweek > 52)
		return false;
	if (myear < 2020 || myear > 2055)
		return false;
	if (eday < 1 || eday > 31)
		return false;
	if (emon < 1 || emon > 12)
		return false;
	if (eyear < 2020 || eyear > 2055)
		return false;
	if (sn <= 1000 || sn >= 42357)
		return false;

	// put mfr week into string
	EncodeNumber(snstr, 2, (mweek + 52*(myear-2020)));

	// put sn into string
	EncodeNumber(snstr+2, 3, sn);

	// calculate expiration day and put into string
	expday = CalcDays(eday, emon, eyear);
	EncodeNumber(snstr+5, 3, expday);
	snstr[8] = '\0';

	// calculate and insert check character
	checkchar = GetCheckChar(snstr);
	snstr[8] = checkchar;
	snstr[9] = '\0';

	// scramble
	MixSN(snstr);
	return true;
}

// decodes serial number, mfr date, and current expiration date from encoded snstr
// this is done on the web server
// returns false if check character does not match or if values are out of range
bool DecodeSN(char *snstr, uint16_t *mweek, uint16_t *myear, uint16_t *eday, uint16_t *emon, uint16_t *eyear, uint16_t *sn) {
	uint8_t checkchar;
	uint32_t expday;

	// descramble
	MixSN(snstr);

	checkchar = snstr[8];
	snstr[8] = '\0';
	if (checkchar != GetCheckChar(snstr))
		return false;
	
	expday = DecodeNumber(snstr+5);
	CalcDate(eday, emon, eyear, expday);

	snstr[5] = '\0';
	*sn = DecodeNumber(snstr+2);

	snstr[2] = '\0';
	*mweek = DecodeNumber(snstr);
	*myear = 2020 + *mweek / 52;
	*mweek = *mweek % 52;

	// range check
	if (*mweek < 1 || *mweek > 52)
		return false;
	if (*myear < 2020 || *myear > 2055)
		return false;
	if (*eday < 1 || *eday > 31)
		return false;
	if (*emon < 1 || *emon > 12)
		return false;
	if (*eyear < 2020 || *eyear > 2055)
		return false;
	if (*sn <= 1000 || *sn >= 42357)
		return false;

	return true;
}

// calculates a 16 bit crc from an array of bytes
uint16_t crc16(uint8_t *data_p, uint8_t length){
    uint8_t x;
    uint16_t crc = 0xFFFF;

    while (length--){
        x = crc >> 8 ^ *data_p++;
        x ^= x>>4;
        crc = (crc << 8) ^ ((unsigned short)(x << 12)) ^ ((unsigned short)(x <<5)) ^ ((unsigned short)x);
    }
    return crc;
}

// encodes serial number and new service plan expiration date into snstr
// this is done on the web server
// expiration date is day (1-31), month (1-12), year (2020 - 2055)
// returns false on error
// return string is encoded as 9 character string (before scrambling):
// WWSSSXXXC where WW is week of manufacture since 2020, SSS is serial number, XXX is expiration day since 2020, and C is additive checksum
bool EncodeAuthKey(char *snstr, uint16_t eday, uint16_t emon, uint16_t eyear, uint16_t sn) {
	uint16_t expday;
	uint16_t crc;

	if (eday < 1 || eday > 31)
		return false;
	if (emon < 1 || emon > 12)
		return false;
	if (eyear < 2020 || eyear > 2055)
		return false;
	if (sn <= 1000 || sn >= 42357)
		return false;

	// calculate expiration day and put into string
	expday = CalcDays(eday, emon, eyear);
	EncodeNumber(snstr, 3, expday);

	// put sn into string
	EncodeNumber(snstr+3, 3, sn);

	// calculate and insert check digits
	crc = crc16((uint8_t *) snstr, 6);
	snstr[6] = crc & 0xFF;
	snstr[7] = (crc & 0xFF00) >> 8;
	snstr[8] = '\0';

	// obfuscate
	MixAuth(snstr);
	return true;
}

// decodes serial number and new service plan expiration date
// this is done on the Flagger mobile
// returns false if check character does not match or if values are out of range
bool DecodeAuth(char *snstr, uint16_t *eday, uint16_t *emon, uint16_t *eyear, uint16_t *sn) {
	uint16_t crc;
	uint32_t expday;

	// descramble
	MixAuth(snstr);

	// check the crc
	crc = snstr[6] + (snstr[7] << 8);
	if (crc != crc16((uint8_t *) snstr, 6))
		return false;

	snstr[6] = '\0';
	expday = DecodeNumber(snstr+3);
	CalcDate(eday, emon, eyear, expday);

	snstr[3] = '\0';
	*sn = DecodeNumber(snstr);

	// range check
	if (*eday < 1 || *eday > 31)
		return false;
	if (*emon < 1 || *emon > 12)
		return false;
	if (*eyear < 2020 || *eyear > 2055)
		return false;
	if (*sn <= 1000 || *sn >= 42357)
		return false;

	return true;
}