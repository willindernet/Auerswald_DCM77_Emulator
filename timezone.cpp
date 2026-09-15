#include "timezone.h"

#include <time.h>


bool getDCF77A1(const struct tm &t)
{
    // Berechnung der Ankündigung der Sommerzeit

    // ----------------------------------------
    // Letzter Sonntag im März
    // ----------------------------------------

    int marchLastDay = 31;

    struct tm marchLast = {};

    marchLast.tm_year = t.tm_year;
    marchLast.tm_mon  = 2;           // März
    marchLast.tm_mday = marchLastDay;
    marchLast.tm_hour = 12;

    mktime(&marchLast);

    int marchLastSunday =
        marchLastDay - marchLast.tm_wday;


    // ----------------------------------------
    // Letzter Sonntag im Oktober
    // ----------------------------------------

    int octoberLastDay = 31;

    struct tm octoberLast = {};

    octoberLast.tm_year = t.tm_year;
    octoberLast.tm_mon  = 9;         // Oktober
    octoberLast.tm_mday = octoberLastDay;
    octoberLast.tm_hour = 12;

    mktime(&octoberLast);

    int octoberLastSunday =
        octoberLastDay - octoberLast.tm_wday;


    // ----------------------------------------
    // Wechsel MEZ -> MESZ
    // Ankündigungsstunde: 01:00 - 01:59 MEZ
    // ----------------------------------------

    if (t.tm_mon == 2 &&
        t.tm_mday == marchLastSunday &&
        t.tm_hour == 1 &&
        t.tm_isdst == 0)
    {
        return true;
    }


    // ----------------------------------------
    // Wechsel MESZ -> MEZ
    // Ankündigungsstunde: 02:00 - 02:59 MESZ
    // ----------------------------------------

    if (t.tm_mon == 9 &&
        t.tm_mday == octoberLastSunday &&
        t.tm_hour == 2 &&
        t.tm_isdst > 0)
    {
        return true;
    }

    return false;
}