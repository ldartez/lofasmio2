#include <time.h>
#include "lofasmio/time.h"
#include "math.h"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/gregorian/gregorian.hpp"

double get_time(){
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    return tp.tv_sec + (tp.tv_nsec / pow(10,9));
}

/*
 * CONVERSION FROM JULIAN DATE TO GREGORIAN CALENDAR
 * formula taken from: https://aa.usno.navy.mil/faq/docs/JD_Formula.php
 */



