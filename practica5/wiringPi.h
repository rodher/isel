#ifndef WIRING_PI_H
#define WIRING_PI_H

#define INPUT 0
#define OUTPUT 1
#define INT_EDGE_FALLING 5
#define HIGH 1
#define LOW 0

void digitalWrite(int pin, int level);
void pinMode(int pin, int mode);
void wiringPiISR(int pin, int edge, void (*function)(void));
void  wiringPiSetup();

#endif


