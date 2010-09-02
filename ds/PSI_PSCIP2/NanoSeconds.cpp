#include "NanoSeconds.h"
#include <iomanip>
#include <sched.h>


void NanoSeconds::be_current_time()
{
        if(::clock_gettime(CLOCK_REALTIME, this))
                throw std::exception();
}


// // WHY it does NOT work properly?
// void NanoSeconds::sleep() const
// {
//      if((tv_sec > 0) && (tv_nsec > 0) )
//      {
//              NanoSeconds___timespec req;
//              NanoSeconds___timespec rem;
// 
//              req.tv_sec = this->tv_sec;
//              req.tv_nsec = this->tv_nsec;
// 
//              //Coud be more secure
//              while( ::nanosleep (&req, &rem) < 0) {
//                      req.tv_sec = rem.tv_sec;
//                      req.tv_nsec = rem.tv_nsec;
//              }
//      }
// }

void NanoSeconds::sleep() const
{
        NanoSeconds aux;
        NanoSeconds now;
        now.be_current_time();
        aux = now;
        while ( *this >= (aux - now) ) { sched_yield(); aux.be_current_time(); }
}

void NanoSeconds::set(time_t sec, unsigned long nsec)
{
        while(nsec >= MAX_NANO) {
                nsec -= MAX_NANO;
                ++sec;
        }
        this->tv_sec = sec;
        this->tv_nsec = nsec;
}

bool operator>=(const NanoSeconds &a, const NanoSeconds &b)
{
        return (a.tv_sec > b.tv_sec) ||
                        ( (a.tv_sec == b.tv_sec) &&
                        (a.tv_nsec >= b.tv_nsec) );
}

bool operator>(const NanoSeconds &a, const NanoSeconds &b)
{
        return (a.tv_sec > b.tv_sec) ||
                        ( (a.tv_sec == b.tv_sec) &&
                        (a.tv_nsec > b.tv_nsec) );
}

NanoSeconds operator-(const NanoSeconds &end, const NanoSeconds &begin)
{
        //assumes begin < end, else 0
        NanoSeconds dif;
        if (begin >= end) {
                dif.set(0,0);
                return dif;
}
        dif.tv_sec = end.tv_sec - begin.tv_sec;
        if (end.tv_nsec > begin.tv_nsec)
                dif.tv_nsec = end.tv_nsec - begin.tv_nsec;
        else {
                dif.tv_nsec = NanoSeconds::MAX_NANO -
                        (begin.tv_nsec - end.tv_nsec);
                --dif.tv_sec;
}
        return dif;
}

NanoSeconds operator+(const NanoSeconds &a, const NanoSeconds &b)
{
        NanoSeconds dif;
        dif.set(a.tv_sec + b.tv_sec, ((unsigned long)a.tv_nsec)+b.tv_nsec);
        return dif;
}


std::ostream & operator<< ( std::ostream & os, const NanoSeconds &ts)
{
        os << std::setw(2) << std::setfill('0') << ts.tv_sec << ":"
                        << std::setw(9) << std::setfill('0') << ts.tv_nsec;
        return os;
}
