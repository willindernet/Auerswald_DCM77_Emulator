#ifndef WATCHDOG_H
#define WATCHDOG_H

//---------------------------------------------------------------------------
// void initNtpWatchdog()
//---------------------------------------------------------------------------
// Description     | init watchdog task
// Parameter       | None
// Return value    | void
//---------------------------------------------------------------------------
void initNtpWatchdog();


//---------------------------------------------------------------------------
// bool isDcf77TransmissionAllowed()
//---------------------------------------------------------------------------
// Description     | Check whether DCF77 transmission is possible
// Parameter       | None
// Return value    | bool: 1 -> DCF77 transmission is possible
//                 | bool: 0 -> DCF77 transmission is not possible
//---------------------------------------------------------------------------
bool isDcf77TransmissionAllowed();


//---------------------------------------------------------------------------
// bool isDcf77TransmissionAbortRequested()
//---------------------------------------------------------------------------
// Description     | Check whether DCF77 transmission is requested
// Parameter       | None
// Return value    | bool: 1 -> DCF77 transmission is requested
//                 | bool: 0 -> DCF77 transmission is not requested
//---------------------------------------------------------------------------
bool isDcf77TransmissionAbortRequested();

//---------------------------------------------------------------------------
// void clearDcf77TransmissionAbortRequest()
//---------------------------------------------------------------------------
// Description     | clear DCF77 transmission abort request
// Parameter       | None
// Return value    | void
//---------------------------------------------------------------------------
void clearDcf77TransmissionAbortRequest();

#endif
