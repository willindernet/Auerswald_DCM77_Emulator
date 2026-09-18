#ifndef DCF77_H
#define DCF77_H

#include <Arduino.h>
#include <time.h>


// --------------------------------------------------
// Array for time information
// --------------------------------------------------

extern bool dcfBits[59];


// --------------------------------------------------
// DCF Initialization
// --------------------------------------------------

void initDCF();


// --------------------------------------------------
// Wait for the next minute to start
// --------------------------------------------------

void waitForNextMinute();


// --------------------------------------------------
// Transmit DCF77 Bit
// --------------------------------------------------

void sendBit1();
void sendBit0();


// --------------------------------------------------
// DCF output
// --------------------------------------------------

void dcfActive();
void dcfInactive();


// --------------------------------------------------
// Generate DCF77 telegram
// --------------------------------------------------

void createDCF77Telegram(struct tm now);


// --------------------------------------------------
// Parity calculation
// --------------------------------------------------

int parity(int start, int end);


// --------------------------------------------------
// Calculation of the summer time announcement
// --------------------------------------------------

bool getDCF77A1(const struct tm &t);


#endif