/*
 * Copyright (c) 2009-2012 Xilinx, Inc.  All rights reserved.
 *
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 *
 *
 *
 */
/*isspid*/
#include <stdio.h>
#include "platform.h"
#include "xparameters.h"
#include "xio.h"
#include "xil_exception.h"
#include "vga_periph_mem.h"
#include "pacman_sprites.h"
#include <stdlib.h>     /* srand, rand */
#include <time.h>
#define SIZE 9
#define UP 0b01000000
#define DOWN 0b00000100
#define LEFT 0b00100000
#define RIGHT 0b00001000
#define CENTER 0b00010000
#define SW0 0b00000001
#define SW1 0b00000010
#define BOMB '*'
#define NUM1 '1'
#define NUM2 '2'
#define NUM3 '3'
#define NUM4 '4'
#define BLANK '0'
#define FLAG '#'
#define NUMOFMINES 9
//BEG---unpened field
#define BEG '@'

int endOfGame;
int inc1;
int inc2;
int i, x, y, ii, oi, R, G, B, RGB, kolona, red, RGBgray;
int numOfFlags;
int flagTrue;
int randomCounter = 50;
int numOfMines;
int firstTimeCenter;
//map that is hidden from the user-it contains the solution
char solvedMap[9][9];
//map that has all of player's moves
char blankMap[9][9];
//map used for opening the blank fields that surround blank field selected
char indicationMap[9][9];

struct grid{
	int food;
	int x;
	int y;
	int food_b;


}pos[28][31];


//end of game
void printOutEndOfGame(char blankTable[SIZE][SIZE], char solvedMap[SIZE][SIZE]) {
	int i, j, ii, jj;
	for (i = 0; i < SIZE; i++) {
		for (j = 0; j < SIZE; j++) {
			ii = (i * 16) + 80;
			jj = (j * 16) + 80;
			if (blankTable[i][j] == FLAG) {
				if (solvedMap[i][j] != BOMB) {
					drawMap(16, 16, ii, jj, 16, 16);
				}
			} else if (blankTable[i][j] != FLAG && solvedMap[i][j] == BOMB) {
				drawMap(0, 16, ii, jj, 16, 16);
			}
		}
	}
}

//when the blank field is pressed, open all blank fields around it

void clean(int x, int y, char resultTable[SIZE][SIZE],
		char indicationMap[SIZE][SIZE]) {
	int i, j;

	indicationMap[x][y] = 'x';

	if (resultTable[x][y] == BLANK) {
		for (i = x - 1; i <= x + 1; i++) {
			for (j = y - 1; j <= y + 1; j++) {
				if (i >= 0 && j >= 0 && i < 9 && j < 9 && !(x == i && y == j)) {
					if (indicationMap[i][j] == BLANK) {
						clean(i, j, resultTable, indicationMap);
					}
				}

			}
		}
	}
}

//function for opening selected field

void openField(int x, int y, char map[9][9]) {
	int i, j;
	int x1, y1;
	x1 = (x - 80) / 16;
	y1 = (y - 80) / 16;

	switch (map[x1][y1]) {
	case NUM1:
		drawMap(16, 0, x - 1, y - 1, 16, 16);
		if (map != blankMap)
			blankMap[x1][y1] = NUM1;
		break;

	case NUM2:
		drawMap(32, 0, x - 1, y - 1, 16, 16);
		if (map != blankMap)
			blankMap[x1][y1] = NUM2;
		break;

	case NUM3:
		drawMap(48, 0, x - 1, y - 1, 16, 16);
		if (map != blankMap)
			blankMap[x1][y1] = NUM3;
		break;

	case BLANK:
		drawMap(0, 0, x - 1, y - 1, 16, 16);
		if (map != blankMap)
			blankMap[x1][y1] = BLANK;
		clean(x1, y1, solvedMap, indicationMap);
		for (i = 0; i < 9; i++) {
			for (j = 0; j < 9; j++) {
				xil_printf("%c", indicationMap[i][j]);
			}
			xil_printf("\n");
		}
		break;

	case NUM4:
		drawMap(64, 0, x - 1, y - 1, 16, 16);
		if (map != blankMap)
			blankMap[x1][y1] = NUM4;
		break;

	case BOMB:
		if (map != blankMap)
			blankMap[x1][y1] = BOMB;
		endOfGame = 1;
		printOutEndOfGame(blankMap, solvedMap);
		drawMap(32, 16, x - 1, y - 1, 16, 16);
		drawMap(77, 54, 120, 54, 27, 26);
		break;
	case '@':
		drawMap(80, 16, x - 1, y - 1, 16, 16);
		if (map != blankMap)
			blankMap[x1][y1] = BEG;
		break;

	case '#':
		drawMap(64, 16, x - 1, y - 1, 16, 16);
		if (map != blankMap)
			blankMap[x1][y1] = FLAG;
		break;
	}
}

//function that generates random game map
void makeTable(char temp[9][9]) {
	int numOfMines = NUMOFMINES, row, column, i, j, m, surroundingMines = 0;
	char table[9][9];

	srand(randomCounter);

	//popunjava matricu nulama
	for (i = 0; i < 9; i++) {
		for (j = 0; j < 9; j++) {
			table[i][j] = BLANK;
		}
	}

	//postavlja random mine
	while (numOfMines > 0) {
		row = rand() % 9;
		column = rand() % 9;
		if (table[row][column] == BLANK) {
			table[row][column] = BOMB;
			numOfMines--;
		}

	}

	//proverava poziciju mina i ispisuje brojeve na odg mesta
	for (i = 0; i < 9; i++) {
		for (j = 0; j < 9; j++) {
			surroundingMines = 0;
			if (table[i][j] != BOMB) {
				if (i > 0 && j > 0) {
					if (table[i - 1][j - 1] == BOMB)
						surroundingMines++;
				}
				if (j > 0) {
					if (table[i][j - 1] == BOMB)
						surroundingMines++;
				}
				if (i < 9 - 1 && j > 0) {
					if (table[i + 1][j - 1] == BOMB)
						surroundingMines++;
				}
				if (i > 0) {
					if (table[i - 1][j] == BOMB)
						surroundingMines++;
				}
				if (i < 9 - 1) {
					if (table[i + 1][j] == BOMB)
						surroundingMines++;
				}
				if (i > 0 && j < 9 - 1) {
					if (table[i - 1][j + 1] == BOMB)
						surroundingMines++;
				}
				if (j < 9 - 1) {
					if (table[i][j + 1] == BOMB)
						surroundingMines++;
				}
				if (i < 9 - 1 && j < 9 - 1) {
					if (table[i + 1][j + 1] == BOMB)
						surroundingMines++;
				}
				table[i][j] = surroundingMines + '0';
			}
		}

	}

	//for testing

	for (i = 0; i < 9; i++) {
		for (j = 0; j < 9; j++) {
			xil_printf("%c", table[i][j]);
		}
		xil_printf("\n");
	}

	for (i = 0; i < 9; i++) {
		for (j = 0; j < 9; j++) {
			temp[i][j] = table[j][i];

		}
	}

	xil_printf("\n");

	for (i = 0; i < 9; i++) {
		for (j = 0; j < 9; j++) {
			xil_printf("%c", temp[j][i]);
		}
		xil_printf("\n");
	}

}

//extracting pixel data from a picture for printing out on the display

void drawMap(int in_x, int in_y, int out_x, int out_y, int width, int height) {

	int ox, oy, oi, iy, ix, ii;
	for(y=0;y<height;y++){
		for(x=0;x<width;x++){
			ox=out_x+x;
			oy=out_y+y;
			oi=oy*320+ox;
			ix = in_x + x;
			iy = in_y + y;
			ii = iy * pacman_sprites.width + ix;
			R = pacman_sprites.pixel_data[ii*pacman_sprites.bytes_per_pixel]>>5;
			G = pacman_sprites.pixel_data[ii*pacman_sprites.bytes_per_pixel + 1] >> 5;
			B = pacman_sprites.pixel_data[ii* pacman_sprites.bytes_per_pixel + 2] >> 5;
			R <<= 6;
			G <<= 3;
			RGB = R | G | B;

			VGA_PERIPH_MEM_mWriteMemory(XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + GRAPHICS_MEM_OFF+ oi * 4, RGB);
					}



	}



/*	int ox, oy, oi, iy, ix, ii;
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			ox = out_x + x;
			oy = out_y + y;
			oi = oy * 320 + ox;
			ix = in_x + x;
			iy = in_y + y;
			ii = iy * pacman_sprites.width + ix;
			R = pacman_sprites.pixel_data[ii
					* pacman_sprites.bytes_per_pixel] >> 5;
			G = pacman_sprites.pixel_data[ii
					* pacman_sprites.bytes_per_pixel + 1] >> 5;
			B = pacman_sprites.pixel_data[ii
					* pacman_sprites.bytes_per_pixel + 2] >> 5;
			R <<= 6;
			G <<= 3;
			RGB = R | G | B;

			VGA_PERIPH_MEM_mWriteMemory(
					XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + GRAPHICS_MEM_OFF
							+ oi * 4, RGB);
		}
	}*/

}

//drawing cursor for indicating position
void drawingCursor(int startX, int startY, int endX, int endY) {

	for (x = startX; x < endX; x++) {
		for (y = startY; y < startY + 2; y++) {
			i = y * 320 + x;
			VGA_PERIPH_MEM_mWriteMemory(
					XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + GRAPHICS_MEM_OFF
							+ i * 4, 0x000000);
		}
	}

	for (x = startX; x < endX; x++) {
		for (y = endY - 2; y < endY; y++) {
			i = y * 320 + x;
			VGA_PERIPH_MEM_mWriteMemory(
					XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + GRAPHICS_MEM_OFF
							+ i * 4, 0x000000);
		}
	}

	for (x = startX; x < startX + 2; x++) {
		for (y = startY; y < endY; y++) {
			i = y * 320 + x;
			VGA_PERIPH_MEM_mWriteMemory(
					XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + GRAPHICS_MEM_OFF
							+ i * 4, 0x000000);
		}
	}

	for (x = endX - 2; x < endX; x++) {
		for (y = startY; y < endY; y++) {
			i = y * 320 + x;
			VGA_PERIPH_MEM_mWriteMemory(
					XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + GRAPHICS_MEM_OFF
							+ i * 4, 0x000000);
		}
	}

}
int matrix[868]={
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,
		0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,
		0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,
		0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,
		0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
		0,1,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,1,0,
		0,1,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,1,0,
		0,1,1,1,1,1,1,0,0,1,1,1,1,0,0,1,1,1,1,0,0,1,1,1,1,1,1,0,
		0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,
		0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,
		0,0,0,0,0,0,1,0,0,1,1,1,1,1,1,1,1,1,1,0,0,1,0,0,0,0,0,0,
		0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,
		0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,
		1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,
		0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,
		0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,
		0,0,0,0,0,0,1,0,0,1,1,1,1,1,1,1,1,1,1,0,0,1,0,0,0,0,0,0,
		0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,
		0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,
		0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,
		0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,
		0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,
		0,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,0,
		0,0,0,1,0,0,1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1,0,0,1,0,0,0,
		0,0,0,1,0,0,1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1,0,0,1,0,0,0,
		0,1,1,1,1,1,1,0,0,1,1,1,1,0,0,1,1,1,1,0,0,1,1,1,1,1,1,0,
		0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,
		0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,
		0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0



};



int s_posX=0;

int p_s=12;
int poz1=0,poz2=0,poz3=0;
int poz=0,pX=154,pY=136;
int u=13,d=23,l=0,r=0,pX_old=14,pY_old=24,br=0;
int posPx=14,posPy=23;
void drawMove(int r){







		if(r==2 )//DOLE
		{
			if(pos[u][d+1].x==1){
				poz1=1;
				for(l=0;l<6;l++){
				pY++;
				for(i=0;i<200000;i++)
						i=i;
				drawMap(39,186,pX,pY,p_s,p_s);
				}
				d++;
			}else if(poz1!=-1){
				for(l=0;l<1;l++){
								pY++;
								for(i=0;i<200000;i++)
										i=i;
								drawMap(39,186,pX,pY,p_s,p_s);
								}
				poz1=-1;
			}


		}
		if(r==1){//DESNO


			/*
			u=pX-67;
			d=pY+3;

			u=u/6;
			d=d/6;
			*/
			if(pos[u+2][d].x==1){
				poz2=1;
				for(l=0;l<6;l++){
					pX++;
				for(i=0;i<200000;i++)
					i=i;

				drawMap(39,186,pX,pY,p_s,p_s);
				}
				u++;
			}else if(poz2!=-1){
				for(l=0;l<4;l++){
								pX++;
							for(i=0;i<200000;i++)
								i=i;

							drawMap(39,186,pX,pY,p_s,p_s);
							}
				poz2=-1;
				u++;

			}



		}
		if(r==3){//LEVO

			if(pos[u-1][d].x==1){
				poz3=1;
				for(l=0;l<6;l++){
					pX--;
				for(i=0;i<200000;i++)
					i=i;

				drawMap(39,186,pX,pY,p_s,p_s);
				}
				u--;
			}else if(poz3!=-1){
				for(l=0;l<3;l++){
					pX--;
				for(i=0;i<200000;i++)
					i=i;

				drawMap(39,186,pX,pY,p_s,p_s);

				}
				poz3=-1;
			}




			}
		if(r==4){//GORE
			if(pos[u][d-1].x==1){
				poz=1;
							for(l=0;l<6;l++){

								pY--;
							for(i=0;i<200000;i++)
								i=i;

							drawMap(39,186,pX,pY,p_s,p_s);
							}
							d--;
						}else if(poz!=-1){

							for(l=0;l<1;l++){

															pY--;
														for(i=0;i<200000;i++)
															i=i;

														drawMap(39,186,pX,pY,p_s,p_s);
														}
							poz=-1;

						}

		}
}


//function that controls switches and buttons

void move() {
	int startX = 81, startY = 81, endX = 153, endY = 174;
	int oldStartX, oldStartY, oldEndX, oldEndY;
	int x, y, ic, ib, i, j;
	int prethodnoStanje;
	int iX=0;
	int r=0;

	typedef enum {
		NOTHING_PRESSED, SOMETHING_PRESSED
	} btn_state_t;
	btn_state_t btn_state = NOTHING_PRESSED;

//	makeTable(solvedMap);
	//drawingCursor(startX, startY, endX, endY);
	drawMap(39,186,pX,pY,p_s,p_s);

	while(endOfGame != 1){
		if((Xil_In32(XPAR_MY_PERIPHERAL_0_BASEADDR)& DOWN)==0){

			if(pos[u][d+1].x==1)
				r=2;

				//drawingCursor(startX,startY,endX,endY);



		}else if ((Xil_In32(XPAR_MY_PERIPHERAL_0_BASEADDR) & RIGHT)==0){
			if(pos[u+2][d].x==1)
				r=1;

				//drawingCursor(startX,startY,endX,endY);



		}else if ((Xil_In32(XPAR_MY_PERIPHERAL_0_BASEADDR) & LEFT)==0){
			if(pos[u+1][d].x==1)
				r=3;

		}else if((Xil_In32(XPAR_MY_PERIPHERAL_0_BASEADDR) & UP)==0){

			if(pos[u][d-1].x==1)
				r=4;
		}




		drawMove(r);
	}
}


	/*while (endOfGame != 1) {

		if (btn_state == NOTHING_PRESSED) {
			btn_state = SOMETHING_PRESSED;
			if ((Xil_In32(XPAR_MY_PERIPHERAL_0_BASEADDR) & DOWN) == 0) {
				if (endY < 224) {
					oldStartY = startY;
					oldEndY = endY;
					startY += 16;
					endY += 16;
					drawingCursor(startX, startY, endX, endY);
					openField(startX, oldStartY, blankMap);
				}

			}

			else if ((Xil_In32(XPAR_MY_PERIPHERAL_0_BASEADDR) & RIGHT) == 0) {
				randomCounter++;
				if (endX < 224) {
					oldStartX = startX;
					startX += 16;
					endX += 16;
					drawingCursor(startX, startY, endX, endY);
					openField(oldStartX, startY, blankMap);

				}
			} else if ((Xil_In32(XPAR_MY_PERIPHERAL_0_BASEADDR) & LEFT) == 0) {
				if (startX > 81) {
					oldStartX = startX;
					startX -= 16;
					endX -= 16;
					drawingCursor(startX, startY, endX, endY);
					openField(oldStartX, startY, blankMap);
				}

			} else if ((Xil_In32(XPAR_MY_PERIPHERAL_0_BASEADDR) & UP) == 0) {
				if (startY > 81) {
					oldStartY = startY;
					startY -= 16;
					endY -= 16;
					drawingCursor(startX, startY, endX, endY);
					openField(startX, oldStartY, blankMap);
				}

			} else if ((Xil_In32(XPAR_MY_PERIPHERAL_0_BASEADDR) & CENTER)
					== 0) {
				int m = (startX - 80) / 16;
				int n = (startY - 80) / 16;
				firstTimeCenter++;
				if (firstTimeCenter == 1) {
					randomCounter++;
					while (solvedMap[m][n] == BOMB)
						makeTable(solvedMap);
				}
				openField(startX, startY, solvedMap);
				int ii = 0, jj = 0;

				for (i = 0; i < SIZE; i++) {
					for (j = 0; j < SIZE; j++) {
						if (indicationMap[i][j] == 'x') {
							ii = (i * 16) + 80;
							jj = (j * 16) + 80;

							if (solvedMap[i][j] == BLANK) {
								drawMap(0, 0, ii, jj, 16, 16);
								blankMap[i][j] = BLANK;
							}
							if (solvedMap[i][j] == NUM2) {
								drawMap(32, 0, ii, jj, 16, 16);
								blankMap[i][j] = NUM2;
							}
							if (solvedMap[i][j] == NUM1) {
								drawMap(16, 0, ii, jj, 16, 16);
								blankMap[i][j] = NUM1;
							}
							if (solvedMap[i][j] == NUM3) {
								drawMap(48, 0, ii, jj, 16, 16);
								blankMap[i][j] = NUM3;
							}
							if (solvedMap[i][j] == NUM4) {
								drawMap(64, 0, ii, jj, 16, 16);
								blankMap[i][j] = NUM4;
							}
						}
					}
				}

			} else if ((Xil_In32(XPAR_MY_PERIPHERAL_0_BASEADDR) & SW0) != 0) { //flag

				if (numOfFlags > 0 && numOfFlags <= NUMOFMINES) {
					int x = (startX - 80) / 16;
					int y = (startY - 80) / 16;
					if (blankMap[x][y] != FLAG && blankMap[x][y] == BEG) {
						drawMap(64, 16, startX - 1, startY - 1, 16, 16);

						blankMap[x][y] = FLAG;

						numOfFlags--;
						//checks if the flag is in the right place
						if (solvedMap[x][y] == BOMB) {
							flagTrue++;
							if (flagTrue == NUMOFMINES) {

								endOfGame = 1;
								drawMap(103, 54, 120, 54, 27, 26);
							}
						}

					}
					//prints out flag counter
					switch (numOfFlags) {
					case 9:
						drawMap(116, 32, 168, 54, 13, 23);
						break;
					case 8:
						drawMap(103, 32, 168, 54, 13, 23);
						break;
					case 7:
						drawMap(90, 32, 168, 54, 13, 23);
						break;
					case 6:
						drawMap(77, 32, 168, 54, 13, 23);
						break;
					case 5:
						drawMap(64, 32, 168, 54, 14, 23);
						break;
					case 4:
						drawMap(51, 32, 168, 54, 13, 23);
						break;
					case 3:
						drawMap(38, 32, 168, 54, 13, 23);
						break;
					case 2:
						drawMap(25, 32, 168, 54, 13, 23);
						break;
					case 1:
						drawMap(13, 32, 168, 54, 13, 23);
						break;
					case 0:
						drawMap(0, 32, 168, 54, 13, 23);
						break;

					}

				}
			} else if ((Xil_In32(XPAR_MY_PERIPHERAL_0_BASEADDR) & SW1) != 0) {
				if (numOfFlags < NUMOFMINES) {

					int x = (startX - 80) / 16;
					int y = (startY - 80) / 16;
					if (blankMap[x][y] == FLAG) {
						drawMap(80, 16, startX - 1, startY - 1, 16, 16);

						blankMap[x][y] = BEG;

						numOfFlags++;

						if (solvedMap[x][y] == BOMB) {
							flagTrue--;
						}

						switch (numOfFlags) {
						case 9:
							drawMap(116, 32, 168, 54, 13, 23);
							break;
						case 8:
							drawMap(103, 32, 168, 54, 13, 23);
							break;
						case 7:
							drawMap(90, 32, 168, 54, 13, 23);
							break;
						case 6:
							drawMap(77, 32, 168, 54, 13, 23);
							break;
						case 5:
							drawMap(64, 32, 168, 54, 13, 23);
							break;
						case 4:
							drawMap(51, 32, 168, 54, 13, 23);
							break;
						case 3:
							drawMap(38, 32, 168, 54, 13, 23);
							break;
						case 2:
							drawMap(25, 32, 168, 54, 13, 23);
							break;
						case 1:
							drawMap(13, 32, 168, 54, 13, 23);
							break;
						case 0:
							drawMap(0, 32, 168, 54, 13, 23);
							break;
						}
					}
				}

			} else {
				btn_state = NOTHING_PRESSED;
			}
		} else { // SOMETHING_PRESSED
			if ((Xil_In32(XPAR_MY_PERIPHERAL_0_BASEADDR) & DOWN) == 0) {
			} else if ((Xil_In32(XPAR_MY_PERIPHERAL_0_BASEADDR) & RIGHT) == 0) {
			} else if ((Xil_In32(XPAR_MY_PERIPHERAL_0_BASEADDR) & LEFT) == 0) {
			} else if ((Xil_In32(XPAR_MY_PERIPHERAL_0_BASEADDR) & UP) == 0) {
			} else if ((Xil_In32(XPAR_MY_PERIPHERAL_0_BASEADDR) & CENTER)
					== 0) {
			} else if ((Xil_In32(XPAR_MY_PERIPHERAL_0_BASEADDR) & SW0) != 0) {
			} else if ((Xil_In32(XPAR_MY_PERIPHERAL_0_BASEADDR) & SW1) != 0) {
			} else {
				btn_state = NOTHING_PRESSED;
			}
		}

	}*/




int main() {

	int j, p, r;
	inc1 = 0;
	inc2 = 0;
	numOfFlags = NUMOFMINES;
	flagTrue = 0;
	numOfMines = NUMOFMINES;
	firstTimeCenter = 0;

	init_platform();

	//helping map for cleaning the table when blank button is pressed
	for (p = 0; p < SIZE; p++) {
		for (r = 0; r < SIZE; r++) {
			indicationMap[p][r] = BLANK;
		}
	}

	//map which contains all the moves of the player
	for (i = 0; i < 9; i++) {
		for (j = 0; j < 9; j++) {
			blankMap[i][j] = BEG;
		}
	}

	VGA_PERIPH_MEM_mWriteMemory(
			XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x00, 0x0); // direct mode   0
	VGA_PERIPH_MEM_mWriteMemory(
			XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x04, 0x3); // display_mode  1
	VGA_PERIPH_MEM_mWriteMemory(
			XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x08, 0x0); // show frame      2
	VGA_PERIPH_MEM_mWriteMemory(
			XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x0C, 0xff); // font size       3
	VGA_PERIPH_MEM_mWriteMemory(
			XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x10, 0xFFFFFF); // foreground 4
	VGA_PERIPH_MEM_mWriteMemory(
			XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x14, 0x0000FF); // background color 5
	VGA_PERIPH_MEM_mWriteMemory(
			XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x18, 0xFF0000); // frame color      6
	VGA_PERIPH_MEM_mWriteMemory(
			XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x20, 1);

	//black background
	for (x = 0; x < 320; x++) {
		for (y = 0; y < 240; y++) {
			i = y * 320 + x;
			VGA_PERIPH_MEM_mWriteMemory(
					XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + GRAPHICS_MEM_OFF
							+ i * 4, 0x000000);
		}
	}

	//drawing a map
/*	for (kolona = 0; kolona < 9; kolona++) {
		for (red = 0; red < 9; red++) {
			drawMap(80, 16, 80 + red * 16, 80 + kolona * 16, 16, 16);
		}
	}*/
for(kolona=0;kolona<186;kolona++){//prva pola
		for(red=0;red<84;red++){
			drawMap(red,kolona,red+76,kolona,1,1);
		}
	}

	for(kolona=185;kolona>=0;kolona--){//druga pola
			for(red=83;red>=0;red--){
				drawMap(red,kolona,243-red,kolona,1,1);
			}
		}


	p=0;
	r=0;
	red=0;
	kolona=0;
/*	for(j=0;j<868;j++){

		if(p==28){
					kolona+=5;
					p=0;
					red+=152;
					r++;

		}								///CRTANJE RANE

		if(matrix[j]==1){
			drawMap(67,203,78+red,kolona+2,3,3);
			pos[p][r].x=1;


		}
		red+=6;
		p++;





	}
*/
	/*int pola;
	for(pola=0;pola<=104;pola+=104){//prvi red
		for(red=0;red<=88;red+=8){
			drawMap(86,261,63+red+pola,10,3,3);
			drawMap(86,261,63+red+pola,156,3,3);
		}
	}

	for(pola=0;pola<=193;pola+=193){//prva kolona
		for(kolona=0;kolona<=56;kolona+=8){
			drawMap(86,261,63+pola,10+kolona,3,3);
		}

	}

	for(pola=0;pola<=113;pola+=113){//druga kolona
		for(kolona=0;kolona<=192;kolona+=8)
			drawMap(86,261,102+pola,10+kolona,3,3);
	}

	for(pola=0;pola<=184;pola+=184){//drugi red
		for(red=0;red<=192;red+=8){
			drawMap(86,261,63+red,42+pola,3,3);
		}
	}*/


	//Pacman

	//drawMap(0,0,0,0,112,240);

	//smiley
/*	drawMap(0, 55, 120, 54, 27, 26);

	//flag
	drawMap(65, 17, 154, 60, 13, 13);

	//counter
	drawMap(116, 32, 168, 54, 14, 23);*/

	//moving through the table
	move();

	cleanup_platform();

	return 0;
}
