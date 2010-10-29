
#pragma once

#include <ctime>
#include <iostream>

typedef struct timespec NanoSeconds___timespec;

/**
* @brief NanoSecond representation of time.
*
* Has methods to set it to the current time and for send the
* thread to sleep, but both have microsecond precision.
*/
class NanoSeconds : public NanoSeconds___timespec
{
        public:
                static const unsigned MAX_NANO = 1000000000;

                /** @brief Sets object time as the current time. */
                void be_current_time(); ///Microsecond precision
                /** @brief Sleeps the thread during object time */
                void sleep() const;     ///Microsecond precision
                void set(time_t sec, unsigned long nsec);
};


bool operator>=(const NanoSeconds &a, const NanoSeconds &b);
bool operator>(const NanoSeconds &a, const NanoSeconds &b);

NanoSeconds operator-(const NanoSeconds &end, const NanoSeconds &begin);
NanoSeconds operator+(const NanoSeconds &a, const NanoSeconds &b);

std::ostream & operator<< ( std::ostream & os, const NanoSeconds &ts);
