#ifndef DCF77_H
#define DCF77_H

#include <Arduino.h>
#include <time.h>


// --------------------------------------------------
// Array for time information
// --------------------------------------------------

extern bool dcfBits[59];


//---------------------------------------------------------------------------
// void initDCF()
//---------------------------------------------------------------------------
// Description     | DCF Initialization
// Parameter       | None
// Return value    | void
//---------------------------------------------------------------------------
void initDCF();


//---------------------------------------------------------------------------
// void waitForNextMinute()
//---------------------------------------------------------------------------
// Description     | Wait for the next minute to start
// Parameter       | None
// Return value    | void
//---------------------------------------------------------------------------
void waitForNextMinute();


//---------------------------------------------------------------------------
// void sendBit1()
//---------------------------------------------------------------------------
// Description     | Transmit DCF77 Bit 1
// Parameter       | None
// Return value    | void
//---------------------------------------------------------------------------
void sendBit1();


//---------------------------------------------------------------------------
// void sendBit0()
//---------------------------------------------------------------------------
// Description     | Transmit DCF77 Bit 0
// Parameter       | None
// Return value    | The void sendBi result of the operation.
//---------------------------------------------------------------------------
void sendBit0();


//---------------------------------------------------------------------------
// void dcfActive()
//---------------------------------------------------------------------------
// Description     | DCF output active
// Parameter       | None
// Return value    | void
//---------------------------------------------------------------------------
void dcfActive();


//---------------------------------------------------------------------------
// void dcfInactive()
//---------------------------------------------------------------------------
// Description     | DCF output inactive
// Parameter       | None
// Return value    | void
//---------------------------------------------------------------------------
void dcfInactive();


//---------------------------------------------------------------------------
// void createDCF77Telegram(struct tm now)
//---------------------------------------------------------------------------
// Description     | Generate DCF77 telegram
// Parameter       | struct tm now: actual time
// Return value    | void
//---------------------------------------------------------------------------
void createDCF77Telegram(struct tm now);


//---------------------------------------------------------------------------
// int parity(int start, int end)
//---------------------------------------------------------------------------
// Description     | Parity calculation
// Parameter       | int start: start bit of the operation
// Parameter       | int end: end bit of the operation
// Return value    | int parity result of the operation.
//---------------------------------------------------------------------------
int parity(int start, int end);


//---------------------------------------------------------------------------
// bool getDCF77A1(const struct tm &t)
//---------------------------------------------------------------------------
// Description     | Calculation of the summer time announcement
// Parameter       | struct tm t: struct of actual time
// Return value    | bool: true  -> summer time announcement
//                 |       false -> no summer time announcement
//---------------------------------------------------------------------------
bool getDCF77A1(const struct tm &t);


#endif