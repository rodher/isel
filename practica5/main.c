#include <stdlib.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <wiringPi.h>
#include "timing.h"
#include "fsm.h"
#include "display.h"

#define GPIO_IR 7

#define GPIO_LED0 0
#define GPIO_LED1 1
#define GPIO_LED2 2
#define GPIO_LED3 3
#define GPIO_LED4 4
#define GPIO_LED5 5
#define GPIO_LED6 6

#define FREQUENCY 9

#define SWEEP_US (1000000/(2*FREQUENCY))

#define N_ROW 7
#define NUM_WIDTH 4
#define N_DIGITS 6
#define N_COL ((NUM_WIDTH+1)*N_DIGITS-1)

#define COL_TIME (SWEEP_US/N_COL)
#define N_CYCLES (N_COL*2*FREQUENCY)

#ifndef NDEBUG
  #define DEBUG(x) x
#else 
  #define DEBUG(x)
#endif

static int       topCol[N_ROW] = {1,0,0,0,0,0,0};
static int    centerCol[N_ROW] = {0,0,0,1,0,0,0};
static int       triCol[N_ROW] = {1,0,0,1,0,0,1};
static int  verticalCol[N_ROW] = {1,1,1,1,1,1,1};
static int lVerticalCol[N_ROW] = {1,0,0,1,1,1,1};
static int hVerticalCol[N_ROW] = {1,1,1,1,0,0,1};
static int  triUpperCol[N_ROW] = {1,1,1,1,0,0,0};
static int     emptyCol[N_ROW] = {0,0,0,0,0,0,0};

static int* map0[NUM_WIDTH] ={emptyCol, emptyCol, emptyCol, emptyCol};
static int* map1[NUM_WIDTH] ={emptyCol, emptyCol, emptyCol, verticalCol};
static int* map2[NUM_WIDTH] ={lVerticalCol, triCol, triCol, hVerticalCol};
static int* map3[NUM_WIDTH] ={triCol, triCol, triCol, verticalCol};
static int* map4[NUM_WIDTH] ={triUpperCol, centerCol, centerCol, verticalCol};
static int* map5[NUM_WIDTH] ={hVerticalCol, triCol, triCol, lVerticalCol};
static int* map6[NUM_WIDTH] ={verticalCol, triCol, triCol, lVerticalCol};
static int* map7[NUM_WIDTH] ={topCol, topCol, topCol, verticalCol};
static int* map8[NUM_WIDTH] ={verticalCol, triCol, triCol, verticalCol};
static int* map9[NUM_WIDTH] ={hVerticalCol, triCol, triCol, verticalCol};

static int** numMap[10] = {map0, map1, map2, map3, map4, map5, map6, map7, map8, map9};

enum clockm_state {
  CLOCKM_WAIT,
};

typedef struct clock_fsm_t
{
  fsm_t fsm;
  display_t* display;
} clock_fsm_t;

clock_fsm_t* clock_fsm_new (fsm_trans_t* tt, display_t* disp){
  clock_fsm_t* this = (clock_fsm_t*) malloc (sizeof (clock_fsm_t));
  fsm_init ((fsm_t*) this, tt);
  this->display = disp;
  return this;
}

enum ledm_state {
  LEDM_WAIT,
};

typedef struct led_fsm_t
{
  fsm_t fsm;
  int* leds;
  display_t* display;
} led_fsm_t;

led_fsm_t* led_fsm_new (fsm_trans_t* tt, int* ledArray, display_t* disp){
  led_fsm_t* this = (led_fsm_t*) malloc (sizeof (led_fsm_t));
  fsm_init ((fsm_t*) this, tt);
  this->leds = ledArray;
  this->display = disp;
  return this;
}

void writeX(display_t* this){
  int col1[N_ROW] = {1, 0, 0, 0, 0, 0, 1};
  int col2[N_ROW] = {0, 1, 0, 0, 0, 1, 0};
  int col3[N_ROW] = {0, 0, 1, 0, 1, 0, 0};
  int col4[N_ROW] = {0, 0, 0, 1, 0, 0, 0};

  int* x[7]={col1, col2, col3, col4, col3, col2, col1};

  write_display(this, x, 7);
}

static int getTime (fsm_t* this){ return 1;}

static void drawTime(clock_fsm_t* this){
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  int hour[N_DIGITS] = {tm.tm_hour/10, tm.tm_hour%10, tm.tm_min/10, tm.tm_min%10, tm.tm_sec/10, tm.tm_sec%10,};
  DEBUG(printf("%d%d:%d%d:%d%d\n", hour[0], hour[1], hour[2], hour[3], hour[4], hour[5] );)
  clear_display(this->display);
  int i;
  int res;

  res = write_display(this->display,numMap[hour[0]],NUM_WIDTH);
  if(res) exit(res);
  for (i = 1; i < N_DIGITS; ++i)
  {
    res = writeSpace(this->display);
    if(res) exit(res);
    res = write_display(this->display,numMap[hour[i]],NUM_WIDTH);
    if(res) exit(res);
  }
}

static int infrared= 0;
static void infrared_isr (void) { infrared = 1; }

static int ir_triggered (fsm_t* this)
{
  int ret= infrared;
  infrared = 0;
  return ret;
}

static void draw_display(led_fsm_t* this)
{
  int i,j;
  struct timeval next_activation;
  struct timeval delay = { 0, COL_TIME};
  gettimeofday (&next_activation, NULL);

  for (i = 0; i < this->display->nCol; ++i)
  {
    for (j = 0; j < this->display->nRow; ++j)
    {
      digitalWrite(this->leds[j], this->display->map[i][j]);
    }
    timeval_add (&next_activation, &next_activation, &delay);
    delay_until (&next_activation);
  }
  for (j = 0; j < this->display->nRow; ++j)
  {
    digitalWrite(this->leds[j], 0);
  }
}

// Explicit FSM description

static fsm_trans_t clockm[] = {
  { CLOCKM_WAIT, getTime, CLOCKM_WAIT, (fsm_output_func_t) drawTime },
  {-1, NULL, -1, NULL },
};

static fsm_trans_t ledm[] = {
  { LEDM_WAIT, ir_triggered, LEDM_WAIT, (fsm_output_func_t) draw_display },
  {-1, NULL, -1, NULL },
};

int main()
{
  struct timeval clk_period = { 0, COL_TIME};
  struct timeval next_activation;

  wiringPiSetup();
  pinMode (GPIO_IR, INPUT);
  wiringPiISR (GPIO_IR, INT_EDGE_FALLING, infrared_isr);
  pinMode (GPIO_LED0, OUTPUT);
  pinMode (GPIO_LED1, OUTPUT);
  pinMode (GPIO_LED2, OUTPUT);
  pinMode (GPIO_LED3, OUTPUT);
  pinMode (GPIO_LED4, OUTPUT);
  pinMode (GPIO_LED5, OUTPUT);
  pinMode (GPIO_LED6, OUTPUT);

  int leds[N_ROW] = {GPIO_LED0, GPIO_LED1, GPIO_LED2, GPIO_LED3, GPIO_LED4, GPIO_LED5, GPIO_LED6};

  display_t* display = new_display(N_ROW, N_COL);

  clock_fsm_t* clockm_fsm = clock_fsm_new (clockm, display);
  led_fsm_t* ledm_fsm = led_fsm_new (ledm, leds, display);

  int cycle = 0;
  gettimeofday (&next_activation, NULL);
  while (1) {
    DEBUG({
      if(cycle%(N_COL*2)==0) infrared = 1;
    })  
    switch(cycle){
      case 0 :
        fsm_fire ( (fsm_t*) ledm_fsm);
        fsm_fire ( (fsm_t*) clockm_fsm);
        break;
      default :
        fsm_fire ( (fsm_t*) ledm_fsm);
        break;
    }
    cycle = (cycle+1)%N_CYCLES;

    timeval_add (&next_activation, &next_activation, &clk_period);
    delay_until (&next_activation);
  }

  free_display(display);

  return 0;
}