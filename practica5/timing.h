
#ifndef TIMING_H
#define TIMING_H

void timeval_sub  (struct timeval *res, struct timeval *a, struct timeval *b);
void timeval_add  (struct timeval *res, struct timeval *a, struct timeval *b);
void delay_until  (struct timeval* next_activation);
void timespec_sub (struct timespec *res, struct timespec *a, struct timespec *b);
void timespec_add (struct timespec *res, struct timespec *a, struct timespec *b);
void timespec_max (struct timespec *max, struct timespec *a, struct timespec *b);

#endif