/*
Authors: Arvin Sartillo, Abigail Fernandex
Section: CMSC 125 - 3L
Project: Sudoku on Console
*/

#include <ctype.h>
#include "../../sdk/dexsdk.h"
#include "../../sdk/time.h"

#define boardRow 9
#define boardCol 9
#define screen_XSTRT 4
#define screen_YSTRT 15

#define keyUP 'w'
#define keyDOWN 's'
#define keyLEFT 'a'
#define keyRIGHT 'd'

int screen_grid[boardRow][boardCol]={0}; //screen grid is used as the referenced table for each game
//Initial 3 puzzles to be used in-game
//Puzzle 1
int puzzle1[boardRow][boardCol]={
	{0,0,3,7,0,0,5,0,0},
	{0,2,7,0,0,0,0,0,8},
	{1,0,0,9,8,0,0,0,6},
	{8,4,0,0,0,6,0,0,0},
	{0,0,0,5,0,0,0,1,0},
	{5,0,0,0,3,1,0,9,2},
	{0,6,9,0,0,4,3,0,7},
	{0,3,0,0,2,0,0,0,1},
	{0,0,0,0,0,0,4,0,0}
};

//Puzzle 2
int puzzle2[boardRow][boardCol]={
	{4,5,0,2,6,0,9,0,0},
	{0,9,8,7,0,0,0,4,0},
	{0,0,0,0,0,0,3,1,0},
	{2,0,0,0,0,0,0,8,3},
	{0,6,0,0,1,0,0,0,0},
	{3,0,7,0,5,0,2,0,9},
	{0,3,0,0,9,0,0,0,6},
	{0,8,0,5,4,0,0,0,1},
	{9,0,1,0,0,8,0,0,0}
};

//Puzzle 2
int puzzle3[boardRow][boardCol]={
	{0,0,5,8,7,0,6,1,0},
	{0,0,0,0,2,1,0,0,3},
	{0,3,0,4,0,0,0,9,0},
	{7,0,0,0,0,8,1,0,0},
	{0,4,0,0,0,0,9,0,0},
	{0,0,6,5,4,0,0,0,2},
	{0,5,4,0,0,9,7,0,0},
	{0,0,0,3,0,0,0,0,8},
	{1,0,2,0,6,0,0,5,0}
};

int write_Number[boardRow][boardCol]={0};	//board slots that are editable
int checker(int col, int row, int value);
int boardChecker();
int finaltime=0;

void gameOver(int win);
void init_board();
void draw_line(int col, int row);
void display_menu();
void clearQuit();
void getInstructions();
void clearBoard();

char quitPrompt();

main() {
	char keypress_menu, keypress_game;
	int timer=0;
	int timeLeft=0;

	while(1) {
		int x, y;
		display_menu();
		keypress_menu = (char)getch();
		if(keypress_menu == 'e') break;
		else if(keypress_menu =='q') {			//this part shows the instruction for the game
			getInstructions();
			keypress_menu=(char)getch();
			if(keypress_menu =='b') continue;
			else continue;
		}
		else if(keypress_menu == 'p') {
			getDescription();
			keypress_menu=(char)getch();
			if(keypress_menu =='b') continue;
			else continue;
		}
		else if(keypress_menu =='a') {			//this part is for the easy level of the game
		 	timer = 12000;
			timeLeft = timer;
			for (x=0; x<9; x++) {
				for (y=0; y<9; y++) {
					screen_grid[x][y] = puzzle1[x][y];
					write_Number[x][y] = 0;
				}
			}
		}	
		else if(keypress_menu == 's') {			//this part is for the moderate level of the game
			timer = 9000;
			timeLeft = timer;
			for (x=0; x<9; x++) {
				for (y=0; y<9; y++) {
					screen_grid[x][y] = puzzle2[x][y];
					write_Number[x][y] = 0;
				}
			}
		}	
		else if(keypress_menu == 'd') { 			//this part is for the difficult level of the game
			timer = 6000;
			timeLeft = timer;
			for (x=0; x<9; x++) {
				for (y=0; y<9; y++) {
					screen_grid[x][y] = puzzle3[x][y];
					write_Number[x][y] = 0;
				}
			}
		} else {
			continue;
		}
		clrscr();
		init_board();

		int y_boardCoor = 20;	//board coordinates
		int x_boardCoor = 6;
		int boardADDY = 0;
		int boardADDX = 0;
		int cl_x = 0;
		int cl_y = 0;
		int valid = 0;		//valid bit to determine the number inputted by user
		int win = 0;		//win bit, if user has won
		
		while(timer!=0) {
			gotoxy(y_boardCoor+boardADDY, x_boardCoor+boardADDX);
			if (kb_ready()) {		
				keypress_game=(char)getch();
				if(keypress_game == 'q') {		// press q to quit
					if(quitPrompt() == 'y') break;	//display quitPrompt; break or continue
					else clearQuit();
				} else if(keypress_game =='d') {	//keypress right
					if(boardADDY<20) {
						boardADDY+=2;
						cl_y++;		//update position
						if(boardADDY==6 || boardADDY==14) boardADDY+=2;
					}
				} else if(keypress_game =='a') {	//keypress left
					if(boardADDY>0) {
						boardADDY-=2;
						cl_y--;		//update position
						if(boardADDY==6 || boardADDY==14) boardADDY-=2;
					}
				} else if(keypress_game =='s') {	//keypress down
					if(boardADDX<10) {
						boardADDX++;
						cl_x++;		//update position
						if(boardADDX==3 || boardADDX==7) boardADDX++;
					}
				} else if(keypress_game =='w') {	//keypress up
					if(boardADDX>0) {
						boardADDX--;
						cl_x--;		//update position
						if(boardADDX==3 || boardADDX==7) boardADDX--;
					}
				} else if(keypress_game =='r') {	//reset game
					int rowers, columners;
					for (rowers=0;rowers<boardRow;rowers++){
						for (columners=0;columners<boardRow;columners++){
							if(write_Number[rowers][columners]==0){
								screen_grid[rowers][columners]=0;
							}
						}
					}

					timer=timeLeft;	//once reset, will reset timer value
					init_board();

				} else if(keypress_game==48) {		// keypress is '0'
					if(write_Number[cl_x][cl_y]==0) {
						screen_grid[cl_x][cl_y]=0;	// deletes number on the cell
						textcolor(WHITE);
						printf("_");
						textcolor(LIGHTGRAY);
					}
				} else if(keypress_game>=49 && keypress_game<=57) {	//keypress value accepted from 1 to 9
					if(write_Number[cl_x][cl_y]==0) {
						int x;
						valid = checker(cl_y,cl_x,(keypress_game-48)); // if number is valid, color LIGHTGREEN. else, LIGHTRED
						if(valid==1) {
							textcolor(LIGHTGREEN);
							printf("%c",keypress_game);
							screen_grid[cl_x][cl_y] = keypress_game-48;		// put number on grid
							textcolor(LIGHTGRAY);
							x=boardChecker();

							if(x==1) {	// if boardChecker returns 1, game is finished and player won
								if(timer<0){
									finaltime=0;
								}else{
									finaltime=timeLeft-timer;
								}

								win=1;
								break;
							}
						} 
						else {
							textcolor(LIGHTRED);
							printf("%c",keypress_game);
							screen_grid[cl_x][cl_y] = keypress_game-48;		// put number on grid
							textcolor(LIGHTGRAY);
							continue;
						} 
					}
				}
			}

			if(win==1) break;
			gotoxy(y_boardCoor+boardADDY,x_boardCoor+boardADDX); // go to the cell

			if(timer>=0) {
				gotoxy(18,18);
				// display remaining time in minutes:seconds
				if((timer/10)%60<10) printf("Time remaining: %d:0%d",((timer/10)/60),((timer/10)%60));
				else printf("Time remaining: %d:%d",((timer/10)/60),((timer/10)%60));
				gotoxy(y_boardCoor+boardADDY,x_boardCoor+boardADDX);	
			}
			delay(5);
			timer--;
		}
		gameOver(win);
		clearBoard();
	}
	clrscr();
}

void clearQuit() {
	gotoxy(screen_YSTRT+5+24, screen_XSTRT+4);
	printf("                                  ");
	gotoxy(screen_YSTRT+5+24, screen_XSTRT+5);
	printf("                                  ");
	gotoxy(screen_YSTRT+5+24, screen_XSTRT+6);
	printf("                                  ");
	gotoxy(screen_YSTRT+5+24, screen_XSTRT+7);
	printf("                                  ");
	gotoxy(screen_YSTRT+5+24, screen_XSTRT+8);
	printf("                                  ");
	gotoxy(screen_YSTRT+5+24, screen_XSTRT+9);
	printf("                                  ");
}

char quitPrompt() {		//quit prompt; asks user if he/she wants to quit the game
	gotoxy(screen_YSTRT+6+24,screen_XSTRT+5);
	printf("Quit game? Progress will be lost.");
	gotoxy(screen_YSTRT+6+24,screen_XSTRT+7);
	printf("Y/N? ");
	return getch();
}

void gameOver(int win) {	//this will prompt if the game is finished whether completed or not
	gotoxy(screen_YSTRT+5, screen_XSTRT+4);
	printf("                      ");
	gotoxy(screen_YSTRT+5, screen_XSTRT+5);
	printf("                      ");
	gotoxy(screen_YSTRT+5, screen_XSTRT+6);
	printf("                      ");
	gotoxy(screen_YSTRT+5, screen_XSTRT+7);
	printf("                      ");
	gotoxy(screen_YSTRT+5, screen_XSTRT+8);
	printf("                      ");
	gotoxy(screen_YSTRT+5, screen_XSTRT+9);
	printf("                      ");
	gotoxy(screen_YSTRT+6, screen_XSTRT+5);

	if(win==1) {
		textcolor(LIGHTGREEN);
		printf("WINNER! Time finished: %d:%d", ((finaltime/10)/60), ((finaltime/10)%60));
	} else if(win==0) {
		textcolor(LIGHTRED);
		printf("GAME OVER!");		
	}

	gotoxy(screen_YSTRT+6,screen_XSTRT+7);
	textcolor(WHITE);
	printf("Press any key to");
	gotoxy(screen_YSTRT+6,screen_XSTRT+8);
	printf("return to main menu.");
	textcolor(LIGHTGRAY);
	getch();
}

void clearBoard(){				//setting the screen grid to 0
	int col,row;
	for(row=0;row<boardRow;row++) {
    	for(col=0;col<boardCol;col++) {
			screen_grid[row][col]= 0;
    	}
	}
}

void draw_line(int col, int row) {	// borrowed from snake.exe game
	gotoxy(col,row); textcolor(WHITE);
	for (col=0;col<(boardCol+2)*3;col++) {
		printf("#");
	}
}

void display_menu() {				//this is for displaying the menu
	int row, col;

	clrscr();

	draw_line(screen_YSTRT,screen_XSTRT);
	textcolor(WHITE);
	gotoxy(screen_YSTRT+3,screen_XSTRT+2);
	printf("SUDOKU");
	gotoxy(screen_YSTRT+4,screen_XSTRT+4);
	printf("-Select Difficulty-");
	gotoxy(screen_YSTRT+4,screen_XSTRT+6);
	printf("[a] Easy");
	gotoxy(screen_YSTRT+4,screen_XSTRT+7);
	printf("[s] Medium");
	gotoxy(screen_YSTRT+4,screen_XSTRT+8);
	printf("[d] Hard");
	gotoxy(screen_YSTRT+4,screen_XSTRT+9);
	printf("[q] Instructions");
	gotoxy(screen_YSTRT+4,screen_XSTRT+10);
	printf("[p] Game Description");
	gotoxy(screen_YSTRT+4,screen_XSTRT+11);
	printf("[e] Exit");

	draw_line(screen_YSTRT,screen_XSTRT+20);

	gotoxy(screen_YSTRT+4,screen_XSTRT+13);
	printf("Choice: ");
}

void getInstructions() {				//this is for displaying the instructions of the game
	clrscr();
	
	draw_line(screen_YSTRT, screen_XSTRT);
	textcolor(WHITE);
	gotoxy(screen_YSTRT+7, screen_XSTRT+2);
	printf("--INSTRUCTIONS PLEASE READ--");
	gotoxy(screen_YSTRT, screen_XSTRT+4);
	printf("A console-based game that follows standard");
	gotoxy(screen_YSTRT, screen_XSTRT+5);
	printf("game rules in a normal sudoku game. Has difficulty");
	gotoxy(screen_YSTRT, screen_XSTRT+6);
	printf("modes of Easy, Moderate and Difficult.");
	gotoxy(screen_YSTRT+4, screen_XSTRT+7);
	printf("Difficulty Descriptions:");
	gotoxy(screen_YSTRT+7, screen_XSTRT+8);
	printf("Easy 		- 20 minutes");
	gotoxy(screen_YSTRT+7, screen_XSTRT+9);
	printf("Moderate 	- 15 minutes");
	gotoxy(screen_YSTRT+7, screen_XSTRT+10);
	printf("Difficult 	- 10 minutes");
	
	gotoxy(screen_YSTRT+4, screen_XSTRT+12);
	printf("Moving the position on board: ");
	gotoxy(screen_YSTRT+7, screen_XSTRT+13);
	printf("W- UP, A- LEFT, S- DOWN, D- DOWN");
	gotoxy(screen_YSTRT+7, screen_XSTRT+14);
	printf("Q- Quit     R- Reset\n");

	gotoxy(screen_YSTRT, screen_XSTRT+16);
	printf("Press \"B\" to go back to Main Menu ");
	draw_line(screen_YSTRT,screen_XSTRT+20);
}

void getDescription() {
	clrscr();

	draw_line(screen_YSTRT, screen_XSTRT);
	textcolor(WHITE);
	gotoxy(screen_YSTRT+7, screen_XSTRT+2);
	printf("!GAME DESCRIPTION!");
	gotoxy(screen_YSTRT, screen_XSTRT+4);
	printf("Sudoku is a puzzle game known by many. The objective is");
	gotoxy(screen_YSTRT, screen_XSTRT+5);
	printf("to fill the empty cells in the grid so that every row,");
	gotoxy(screen_YSTRT, screen_XSTRT+6);
	printf("column, and bold region contains each digit once.");
	gotoxy(screen_YSTRT+4, screen_XSTRT+8);
	printf("Developers:");		
	gotoxy(screen_YSTRT+7, screen_XSTRT+9);
	printf("Arvin Sartillo");
	gotoxy(screen_YSTRT+7, screen_XSTRT+10);
	printf("Abigail Fernandez");
	
	gotoxy(screen_YSTRT, screen_XSTRT+12);
	printf("Press \"B\" to go back to the menu ");
	draw_line(screen_YSTRT,screen_XSTRT+14);
}

void init_board() {						//this is for initializing the board of the game with corresponding values each cell
	int row=0,col=0;
	int i,e=0;

	clrscr();
	draw_line(screen_YSTRT,screen_XSTRT);
	gotoxy(screen_YSTRT+3,screen_XSTRT+1);
	printf("+-------+-------+-------+");
	int px=0,py=0;

	for(row=screen_XSTRT; row<boardRow+screen_XSTRT+2; row++)
	{
		if((row-screen_XSTRT-e)!=0 && (row-screen_XSTRT-e)%3==0) {
			gotoxy(screen_YSTRT+3,row+2);
			printf("+-------+-------+-------+");
			row++;
			e++;
		}

		gotoxy(screen_YSTRT+3,row+2);
		textcolor(LIGHTGRAY); printf("|");
		textcolor(WHITE);
		py=0;

		for(col=screen_YSTRT;col<boardCol+screen_YSTRT;col++) {
			if((col-screen_YSTRT)!=0 && (col-screen_YSTRT)%3==0) {
				textcolor(LIGHTGRAY);
				printf(" |");
				textcolor(WHITE);
			}
			if(screen_grid[px][py]==0) {
				textcolor(WHITE);
				printf(" _");
				textcolor(LIGHTGRAY);
				py++;
			} else {
				textcolor(YELLOW);
				printf(" %d",screen_grid[px][py]);
				write_Number[px][py]=1;
				textcolor(LIGHTGRAY);
				py++;
			}
		}
		textcolor(LIGHTGRAY);
		printf(" |\n");
		px++;
	}

	//displays instructions/
	//control reminders when in-game
	gotoxy(screen_YSTRT+3, screen_XSTRT+13);
	printf("+-------+-------+-------+");
	gotoxy(screen_YSTRT+3, screen_XSTRT+15);
	printf("Controls: [W] Up  [S] Down");
	gotoxy(screen_YSTRT+3, screen_XSTRT+16);
	printf("          [A] Left  [D] Right");
	gotoxy(screen_YSTRT+3, screen_XSTRT+17);
	printf("          [0] Delete");
	gotoxy(screen_YSTRT+3, screen_XSTRT+18);
	printf("          [R] Reset");
	gotoxy(screen_YSTRT+3, screen_XSTRT+19);
	printf("          [Q] Quit");
	draw_line(screen_YSTRT, boardRow+screen_XSTRT+11);
}


//this section will check if there are any duplicate numbers
//values on the board for both rows and columns
int checker(int col, int row, int value) {
	int rowchecker, columnchecker, counterChecker=0, rowA, colA, rowB, colB, rowbox, colbox;
	rowB = row/3;
	colB = col/3;
	for(rowchecker=0; rowchecker<9; rowchecker++) {
		columnchecker=col;
		if(rowchecker==row) {
			rowchecker++;
		} else {
			if(value==screen_grid[rowchecker][columnchecker]) {
				return 0;
			}
		}
	}
	for(columnchecker=0; columnchecker<9; columnchecker++) {
		rowchecker=row;
		if(columnchecker==col) {
			columnchecker++;
		}else {
			if(value==screen_grid[rowchecker][columnchecker]) {
				return 0;
			}
		}
	}
	for(columnchecker=0; columnchecker<3; columnchecker++) {
		colbox = colB*3+columnchecker;
		for(rowchecker =0; rowchecker<3; rowchecker++) {
			rowbox = rowB*3+rowchecker;
			if(value==screen_grid[rowbox][colbox]) {
				if(rowbox==row&&colbox==col) {
					rowchecker++;
				} else {
					return 0;
				}
			}
		}
	}	return 1;
}

int boardChecker() {							//check if board is flilled
	int rowChecker, columnChecker;
	for(rowChecker=0; rowChecker<9; rowChecker++) {
		for(columnChecker=0; columnChecker<9; columnChecker++) {
			if(screen_grid[rowChecker][columnChecker]==0) {
				return 0;
			}
		}
	}	return 1;
}
