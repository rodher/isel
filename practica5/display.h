#ifndef DISPLAY_H
#define DISPLAY_H


typedef struct display_t {
  int** map;
  int nRow;
  int nCol;
  int i;
} display_t;

display_t* new_display(int rows, int cols);
void free_display(display_t* this);
int writeLine(display_t* this, int* line);
int writeSpace(display_t* this);
void clear_display(display_t* this);
void show_display(display_t* this);
int write(display_t* this, int** figure, int width);

#endif