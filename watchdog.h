#ifndef WATCHDOG_H
#define WATCHDOG_H

void initNtpWatchdog();
bool isDcf77TransmissionAllowed();
bool isDcf77TransmissionAbortRequested();
void clearDcf77TransmissionAbortRequest();

#endif
