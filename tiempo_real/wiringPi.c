

#include <stdio.h>
#include "wiringPi.h"

static int money0;
static int money1;
static int money2;

void pinMode(int pin, int mode){}
void wiringPiISR(int pin, int edge, void function(void)){}

void digitalWrite(int pin, int level){
	//printf("Salida del pin %i : %i\n",pin,level);
}

int digitalRead(int pin){
	if(pin==7) return money0;
	else if(pin==8) return money1;
	else if(pin==9) return money2;
	else return 0;
}

void  wiringPiSetup(){}

void actualizaMoney(int mon0, int mon1, int mon2){
	money0 =mon0;
	money1 = mon1;
	money2 = mon2;
}
