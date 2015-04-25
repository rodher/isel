#include <sys/time.h>
#include <time.h>
#include "timing.h"

// res = a - b
void
timeval_sub (struct timeval *res, struct timeval *a, struct timeval *b)
{
  res->tv_sec = a->tv_sec - b->tv_sec;
  res->tv_usec = a->tv_usec - b->tv_usec;
  if (res->tv_usec < 0) {
    --res->tv_sec;
    res->tv_usec += 1000000;
  }
}

// res = a + b
void
timeval_add (struct timeval *res, struct timeval *a, struct timeval *b)
{
  res->tv_sec = a->tv_sec + b->tv_sec
    + a->tv_usec / 1000000 + b->tv_usec / 1000000; 
  res->tv_usec = a->tv_usec % 1000000 + b->tv_usec % 1000000;
}

// wait until next_activation (absolute time)
void delay_until (struct timeval* next_activation)
{
  struct timeval now, timeout;
  gettimeofday (&now, NULL);
  timeval_sub (&timeout, next_activation, &now);
  select (0, NULL, NULL, NULL, &timeout);
}

// res = a - b
void
timespec_sub (struct timespec *res, struct timespec *a, struct timespec *b)
{
  res->tv_sec = a->tv_sec - b->tv_sec;
  res->tv_nsec = a->tv_nsec - b->tv_nsec;
  if (res->tv_nsec < 0) {
    --res->tv_sec;
    res->tv_nsec += 1000000000;
  }
}

// res = a + b
void
timespec_add (struct timespec *res, struct timespec *a, struct timespec *b)
{
  res->tv_sec = a->tv_sec + b->tv_sec
    + a->tv_nsec / 1000000000 + b->tv_nsec / 1000000000; 
  res->tv_nsec = a->tv_nsec % 1000000000 + b->tv_nsec % 1000000000;
}

// max = max{a,b}
void
timespec_max (struct timespec *max, struct timespec *a, struct timespec *b)
{
  if(a->tv_sec > b->tv_sec) (*max)=(*a);
  else if(a->tv_sec < b->tv_sec) (*max)=(*b);
  else{
    if(a->tv_nsec > b->tv_nsec) (*max)=(*a);
    else (*max)=(*b);
  }
}