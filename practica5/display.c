#include <stdlib.h>
#include <stdio.h>
#include "display.h"



display_t* new_display(int rows, int cols){

	display_t* this = (display_t*) malloc(sizeof(display_t));

	this->nRow = rows;
	this->nCol = cols;

  	this->map = (int**) malloc(this->nCol*sizeof(int *));
 	int i,j;
	for (i = 0; i < this->nCol; ++i)
	{
		this->map[i] = (int *) malloc(this->nRow*sizeof(int));

		for (j = 0; j < this->nRow; ++j)
		{
			this->map[i][j] = 0;
		}
	}

	this->i = 0;

  	return this; 	
}

void free_display(display_t* this){
	int i;
  	for (i = 0; i < this->nCol; ++i) free(this->map[i]);
  	free(this->map);
	free(this);
}

int writeLine(display_t* this, int* line){
  if((this->i)>=(this->nCol)) return -1;
  else{
  	int j;
  	for (j = 0; j < this->nRow; ++j)
  	{
  		this->map[this->i][j] = line[j];
  	}
  	this->i++;
  	return 0;
  }
}

int writeSpace(display_t* this){
  if((this->i)>=(this->nCol)) return -1;
  else{
  	int j;
  	for (j = 0; j < this->nRow; ++j)
  	{
  		this->map[this->i][j] = 0;
  	}
  	this->i++;
  	return 0;
  }
}

void clear_display(display_t* this){
	int i, j;
	for (i = 0; i < this->nCol; ++i)
	{
		for (j = 0; j < this->nRow; ++j)
		{
			this->map[i][j] = 0;
		}
	}
	this->i = 0;
}

void show_display(display_t* this){
	int i,j;
	for (i = 0; i < this->i; ++i)
	{
		for (j = this->nRow-1; j >= 0; --j)
		{
			if(this->map[i][j]) printf("*");
			else printf(" ");
		}
		printf("\n");
	}
}

int write_display(display_t* this, int** figure, int width){
	int k;
	for (k = 0; k < width; ++k)
	{
		if(writeLine(this, figure[k])) return -1;
	}
	return 0;
}



