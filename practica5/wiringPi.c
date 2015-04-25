#include <stdio.h>
#include "wiringPi.h"

void pinMode(int pin, int mode){}
void wiringPiISR(int pin, int edge, void function(void)){}

void digitalWrite(int pin, int level){
	printf("Salida del pin %i : %i\n",pin,level);
}

void  wiringPiSetup(){}