
#define INPUT 0
#define OUTPUT 1
#define INT_EDGE_FALLING 5
#define HIGH 1
#define LOW 0

void digitalWrite(int pin, int level);
int digitalRead(int pin);
void pinMode(int pin, int mode);
void wiringPiISR(int pin, int edge, void function(void));
void  wiringPiSetup();
void actualizaMoney(int mon0, int mon1, int mon2);

