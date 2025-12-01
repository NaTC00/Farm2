//
//Created by Natalia Ceccarini
//
#include <util.h>




int isdot(const char dir[])
{
    int l = strlen(dir);
    if ((l > 0 && dir[l - 1] == '.')) // se la lunghezza della stringa non è 0 e il penultimo carattere è '.' (l'ultimo è il terminatore obv )
        return 1;
    return 0;
}



int msleep(long tms)
{

    // The timespec struct contains two member variables:
    // tv_sec – The variable of the time_t type made to store time in seconds.
    // tv_nsec – The variable of the long type used to store time in nanoseconds.

    struct timespec ts;
    int ret;

    if (tms < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = tms / 1000;
    ts.tv_nsec = (tms % 1000) * 1000000;


    do
    {
        ret = nanosleep(&ts, &ts);
    } while (ret && errno == EINTR);

    return ret;
}

