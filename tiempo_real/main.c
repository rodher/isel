/*

PRACTICA 1 DE ISEL CURSO 2014/2015
Autores: Rodrigo Hernangomez Herrero y Ana Jimenez Valbuena
Febrero 2015

*/


#include <sys/select.h>
#include <sys/time.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <wiringPi.h>
#include "fsm.h"
#include "tasks.h"
#include <stdio.h>

#ifndef NDEBUG
#define DEBUG(x) x
#else
#define DEBUG(x)
#endif

#define GPIO_BUTTON	2
#define GPIO_LED	3
#define GPIO_CUP	4
#define GPIO_COFFEE	5
#define GPIO_MILK	6
#define GPIO_MONEY0   7 // Los tres pines determinan el tipo de moneda que se ha introducido
#define GPIO_MONEY1   8
#define GPIO_MONEY2   9
#define GPIO_COIN     10  // Flag que interrumpe avisandonos de que hay moneda
#define GPIO_5C       11  // Cada pin devuelve un tipo de moneda al activarse. 
#define GPIO_10C      12  // Se supone que el mecanismo para devolver monedas se activa por flanco
#define GPIO_20C      13
#define GPIO_50C      14
#define GPIO_1E       15
#define GPIO_2E       16

#define CUP_TIME	250
#define COFFEE_TIME	3000
#define MILK_TIME	3000

#define PRECIO         50 // Expresado en centimos para poder trabajar con enteros


static int dinero = 0; // Variable global que lleva la cuenta del dinero
static pthread_mutex_t dinero_mutex;

static int cobrar = 0; // Flag que indica al monedero que debemos cobrar:                         |<--|cobrar
static pthread_mutex_t cobrar_mutex;
                                                                                      //MONEDERO  |   |   MAQUINA DE CAFE
static int hay_dinero = 0;  // Flag con la que el monedero nos avisa de si hay dinero   hay_dinero|-->|
static pthread_mutex_t hay_dinero_mutex;

enum cofm_state {
  COFM_WAITING,
  COFM_CUP,
  COFM_COFFEE,
  COFM_MILK,
};

enum cashm_state {
  COFM_MONEY,
  COFM_VUELTAS,
};

enum monedas{   //Tipo de monedas que acepta la máquina
  C5=1,
  C10=2,
  C20=3,
  C50=4,
  E1=5,
  E2=6,
};

static int button = 0;
static void button_isr (void) { button = 1; }

static int money = 0;
static void money_isr (void) { 
  int monedaID = digitalRead(GPIO_MONEY2)*4+digitalRead(GPIO_MONEY1)*2+digitalRead(GPIO_MONEY0); // Combinamos tres entradas en un numero de 3 bits
  int valor;
  switch(monedaID){
    case C5 :
      valor=5;
      break;
    case C10 :
      valor=10;
      break;
    case C20 :
      valor=20;
      break;
    case C50 :
      valor=50;
      break;
    case E1 :
      valor=100;
      break;
    case E2 :
      valor=200;
      break;
    default :
      valor=0;
      break;
  }
  dinero+=valor;
  if(dinero>=PRECIO) money=1;
}    //INTERRUPCIÓN PARA LA ENTRADA DE MONEDAS


static int timer = 0;
static void timer_isr (union sigval arg) { timer = 1; }
static void timer_start (int ms)
{
  timer_t timerid;
  struct itimerspec value;
  struct sigevent se;
  se.sigev_notify = SIGEV_THREAD;
  se.sigev_value.sival_ptr = &timerid;
  se.sigev_notify_function = timer_isr;
  se.sigev_notify_attributes = NULL;
  value.it_value.tv_sec = ms / 1000;
  value.it_value.tv_nsec = (ms % 1000) * 1000000;
  value.it_interval.tv_sec = 0;
  value.it_interval.tv_nsec = 0;
  timer_create (CLOCK_REALTIME, &se, &timerid);
  timer_settime (timerid, 0, &value, NULL);
}

static int button_pressed (fsm_t* this)
{
  int ret1 = button;
  pthread_mutex_lock(&hay_dinero_mutex);
  int ret2 = hay_dinero;
  pthread_mutex_unlock(&hay_dinero_mutex);
  if(ret1&&ret2){         // Comprueba que ambos se cumplen antes de resetearlos
    button  = 0;
    pthread_mutex_lock(&hay_dinero_mutex);
    hay_dinero = 0;
    pthread_mutex_unlock(&hay_dinero_mutex);
    return 1;
  } else return 0;
}

static int insert_coin (fsm_t* this)
{
  int ret = money;
  money = 0;
  return ret;
}

static int coffee_served (fsm_t* this)
{
  pthread_mutex_lock (&cobrar_mutex);
  int ret = cobrar;
  pthread_mutex_unlock (&cobrar_mutex);
  return ret; // En este caso es la funcion de salida getChange la que resetea el flag de cobrar
}

static int timer_finished (fsm_t* this)
{
  int ret = timer;
  timer = 0;
  return ret;
}

static void cup (fsm_t* this)
{
  digitalWrite (GPIO_LED, LOW);
  digitalWrite (GPIO_CUP, HIGH);
  timer_start (CUP_TIME);
}

static void coffee (fsm_t* this)
{
  digitalWrite (GPIO_CUP, LOW);
  digitalWrite (GPIO_COFFEE, HIGH);
  timer_start (COFFEE_TIME);
}

static void milk (fsm_t* this)
{
  digitalWrite (GPIO_COFFEE, LOW);
  digitalWrite (GPIO_MILK, HIGH);
  timer_start (MILK_TIME);
}

static void finish (fsm_t* this)
{
  digitalWrite (GPIO_MILK, LOW);
  digitalWrite (GPIO_LED, HIGH);
  pthread_mutex_lock (&cobrar_mutex);
  cobrar = 1;
  pthread_mutex_unlock (&cobrar_mutex);
  pthread_mutex_lock(&dinero_mutex);
  dinero -= PRECIO;
  pthread_mutex_unlock(&dinero_mutex);
}

static void enough_money(fsm_t* this)
{
  pthread_mutex_lock(&hay_dinero_mutex);
  hay_dinero = 1;
  pthread_mutex_unlock(&hay_dinero_mutex);
}

static void getChange(fsm_t* this)
{ 
  // Estructura de control que va devolviendo el cambio. Cada "palanca devuelve-cambio" se activa por flanco
  // Quiza sea mas elegante usar un bucle y una estructura de arrays, pero esta forma es mas entendible
  pthread_mutex_lock(&dinero_mutex);
  if(dinero>=200){
    dinero-=200;
    digitalWrite(GPIO_2E, HIGH);
    digitalWrite(GPIO_2E, LOW);
  }else if(dinero>=100){
    dinero-=100;
    digitalWrite(GPIO_1E, HIGH);
    digitalWrite(GPIO_1E, LOW);
  }else if(dinero>=50){
    dinero-=50;
    digitalWrite(GPIO_50C, HIGH);
    digitalWrite(GPIO_50C, LOW);
  }else if(dinero>=20){
    dinero-=20;
    digitalWrite(GPIO_20C, HIGH);
    digitalWrite(GPIO_20C, LOW);
  }else if(dinero>=10){
    dinero-=10;
    digitalWrite(GPIO_10C, HIGH);
    digitalWrite(GPIO_10C, LOW);
  }else if(dinero>=5){
    dinero-=5;
    digitalWrite(GPIO_5C, HIGH);
    digitalWrite(GPIO_5C, LOW);
  }
  if(dinero>=0) this->current_state=COFM_VUELTAS;
  else{
    pthread_mutex_lock (&cobrar_mutex);
    cobrar=0;
    pthread_mutex_unlock (&cobrar_mutex);
  }
  pthread_mutex_unlock(&dinero_mutex);
}


// Explicit FSM description
static fsm_trans_t cofm[] = {
  { COFM_WAITING, button_pressed, COFM_CUP,     cup    },
  { COFM_CUP,     timer_finished, COFM_COFFEE,  coffee },
  { COFM_COFFEE,  timer_finished, COFM_MILK,    milk   },
  { COFM_MILK,    timer_finished, COFM_WAITING, finish },
  {-1, NULL, -1, NULL },
};

static fsm_trans_t cashm[] = {
  { COFM_MONEY, insert_coin, COFM_VUELTAS, enough_money},
  { COFM_VUELTAS, coffee_served, COFM_MONEY, getChange},   //DEVUELVE EL DINERO CUANDO HA ACABADO DE ECHAR LA LECHE
  {-1, NULL, -1, NULL },
};

static
void
create_task (pthread_t* tid, void *(*f)(void *), void* arg,
             int period_ms, int prio, int stacksize)
{
  pthread_attr_t attr;
  struct sched_param sparam;
  struct timespec next_activation;
  struct timespec period = { 0, 0L };

  sparam.sched_priority = sched_get_priority_min (SCHED_FIFO) + prio;
  clock_gettime (CLOCK_REALTIME, &next_activation);
  next_activation.tv_sec += 1;
  period.tv_sec  = period_ms / 1000;
  period.tv_nsec = (period_ms % 1000) * 1000000L;

  pthread_attr_init (&attr);
  pthread_attr_setstacksize (&attr, stacksize);
  pthread_attr_setscope (&attr, PTHREAD_SCOPE_SYSTEM);
  pthread_attr_setschedpolicy (&attr, SCHED_FIFO);
  pthread_attr_setschedparam (&attr, &sparam);
  pthread_create (tid, &attr, f, arg);
  pthread_make_periodic_np (pthread_self(), &next_activation, &period);
}

static
void
init_mutex (pthread_mutex_t* m)
{
  pthread_mutexattr_t attr;
  pthread_mutexattr_init (&attr);
  // pthread_mutexattr_setprotocol (&attr, PTHREAD_PRIO_PROTECT);
  pthread_mutexattr_setprotocol (&attr, PTHREAD_PRIO_INHERIT);
  pthread_mutex_init (m, &attr);
  // pthread_mutex_setprioceiling
  //   (&m, sched_get_priority_min (SCHED_FIFO) + prioceiling);
}

int main ()
{
  fsm_t* cofm_fsm = fsm_new (cofm);
  fsm_t* cashm_fsm = fsm_new (cashm);

  void*
  coff_func (void* arg)
  {
    while(1){
      pthread_wait_np ((unsigned long*) arg);
      fsm_fire (cofm_fsm);  
    }
  }

  void*
  cash_func (void* arg)
  {
    while(1){
      pthread_wait_np ((unsigned long*) arg);
      fsm_fire (cashm_fsm);  
    }
  }
  wiringPiSetup();
  pinMode (GPIO_BUTTON, INPUT);
  pinMode (GPIO_MONEY0, INPUT);
  pinMode (GPIO_MONEY1, INPUT);
  pinMode (GPIO_MONEY2, INPUT);
  pinMode (GPIO_COIN, INPUT);
  wiringPiISR (GPIO_BUTTON, INT_EDGE_FALLING, button_isr);
  wiringPiISR (GPIO_COIN, INT_EDGE_FALLING, money_isr);
  pinMode (GPIO_CUP, OUTPUT);
  pinMode (GPIO_COFFEE, OUTPUT);
  pinMode (GPIO_MILK, OUTPUT);
  pinMode (GPIO_LED, OUTPUT);
  pinMode (GPIO_5C, OUTPUT);
  pinMode (GPIO_10C, OUTPUT);
  pinMode (GPIO_20C, OUTPUT);
  pinMode (GPIO_50C, OUTPUT);
  pinMode (GPIO_1E, OUTPUT);
  pinMode (GPIO_2E, OUTPUT);
  digitalWrite (GPIO_LED, HIGH);

	/*Variables para pruebas*/
	int mon0;
	int mon1;
	int mon2;

  pthread_t tcoff, tcash;
  void* ret;

  init_mutex (&dinero_mutex);
  init_mutex (&hay_dinero_mutex);
  init_mutex (&cobrar_mutex);
  create_task (&tcoff, coff_func, NULL, 1, 2, 1024);
  create_task (&tcash, cash_func, NULL, 3, 1, 1024);

  pthread_join(tcoff, &ret);
  pthread_join(tcash, &ret);
  
  while (1) {
    scanf("%d %d %d %d \n", &button, &mon2, &mon1, &mon0);
    actualizaMoney(mon0, mon1, mon2);
    money_isr();
  }

  return 0;
}
