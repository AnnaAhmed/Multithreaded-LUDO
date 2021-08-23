

#include "util.h"
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string>
#include <cstring>
#include <pthread.h>
#include <semaphore.h>
using namespace std;

pthread_t MainThread;	//For basic implementation
pthread_t Players[4];	//For all four players
pthread_t RoundCheck;	// Testing game rounds
pthread_t MasterThread; //Master thread features
pthread_t HitThread;	//Testing player hits

sem_t DiceRollSem; // To allow one player to roll the dice
sem_t GridAccess;  // GridAccess semaphore
sem_t HitCheck;	   //Used to check when to release hit condition

int diceroll = 0; //Global Dice Variable

int playername = 0;			  //Used to display name (using glut) of token currently in player
bool playerRound[4];		  //To ensure that each player gets a turn in each round
int playerHitRate[4];		  //Keeping track of each player's hitrate
bool BoardPlayer[4];		  //Tracking which is currently on grid in order to confirm a hit
bool HomeEnter[4];			  //Check if a player can enter Home
bool signalCompletion[4];	  //Send signal to master thread after a player completes its game
int winningPlayers[3] = {-1}; //Testing which player won the game
string input_playername[4];	  //Taking name of players
int NumberofTokens;
int no_six_check[4];	  //Used to check if a thread doesn't score a 6 for 20 times then cancel it
int hit_rate_nochange[4]; //Used to check if a thread doesn't hit another token after 20 tries
bool signalCancel[4];	  //Send signal to master thread in order to cancel
int cancelledthread[4];	  //Stores indexes of cancelled threads in order to cout them in Main Thread

//Token coordinate 3D array
int ***token_coordinate;

int home[4][4][2] = {
	//Green
	{{335, 685}, {415, 685}, {415, 585}, {335, 585}},
	//Yellow
	{{755, 685}, {835, 685}, {835, 585}, {755, 585}},
	//Blue
	{{755, 260}, {835, 260}, {835, 160}, {755, 160}},
	//Red
	{{335, 260}, {415, 260}, {415, 160}, {335, 160}}

};

int safeSquares[4][2][2] = {
	//First one is starting safe square
	{{315, 490}, {355, 365}}, //Green
	{{650, 700}, {530, 660}}, //Yellow
	{{860, 370}, {820, 490}}, //Blue
	{{530, 155}, {650, 195}}, //Red
};
int token_mov_grid[4][57][2] =
	{
		//Green
		{
			{275, 490},
			{315, 490},
			{355, 490},
			{395, 490},
			{435, 490},
			{475, 490}, //Green starting path
			{530, 540},
			{530, 580},
			{530, 620},
			{530, 660},
			{530, 700},
			{530, 740}, //yellow path starting
			{590, 740}, //yellow middle
			{650, 740},
			{650, 700},
			{650, 660},
			{650, 620},
			{650, 580},
			{650, 540}, //yellow last column
			{700, 490},
			{740, 490},
			{780, 490},
			{820, 490},
			{860, 490},
			{900, 490}, //blue start column
			{900, 430}, //blue middle
			{900, 370},
			{860, 370},
			{820, 370},
			{780, 370},
			{740, 370},
			{700, 370}, //blue last column
			{650, 315},
			{650, 275},
			{650, 235},
			{650, 195},
			{650, 155},
			{650, 115}, //red start column
			{590, 115}, //red middle
			{530, 115},
			{530, 155},
			{530, 195},
			{530, 235},
			{530, 275},
			{530, 315}, //red last column
			{475, 365},
			{435, 365},
			{395, 365},
			{355, 365},
			{315, 365},
			{275, 365}, //green last column
			{275, 425}, //green middle
			{315, 425},
			{355, 425},
			{395, 425},
			{435, 425},
			{475, 425} //green home column
		},

		//Yellow
		{
			{650, 740},
			{650, 700},
			{650, 660},
			{650, 620},
			{650, 580},
			{650, 540}, //yellow start column
			{700, 490},
			{740, 490},
			{780, 490},
			{820, 490},
			{860, 490},
			{900, 490}, //blue start column
			{900, 430}, //blue middle
			{900, 370},
			{860, 370},
			{820, 370},
			{780, 370},
			{740, 370},
			{700, 370}, //blue last column
			{650, 315},
			{650, 275},
			{650, 235},
			{650, 195},
			{650, 155},
			{650, 115}, //red start column
			{590, 115}, //red middle
			{530, 115},
			{530, 155},
			{530, 195},
			{530, 235},
			{530, 275},
			{530, 315}, //red last column
			{475, 365},
			{435, 365},
			{395, 365},
			{355, 365},
			{315, 365},
			{275, 365}, //green start column
			{275, 425}, //green middle
			{275, 490},
			{315, 490},
			{355, 490},
			{395, 490},
			{435, 490},
			{475, 490}, //green last column
			{530, 540},
			{530, 580},
			{530, 620},
			{530, 660},
			{530, 700},
			{530, 740}, //yellow last column
			{590, 740}, //yellow middle
			{590, 700},
			{590, 660},
			{590, 620},
			{590, 580},
			{590, 540} //yellow home column
		},

		//Blue
		{
			{900, 370},
			{860, 370},
			{820, 370},
			{780, 370},
			{740, 370},
			{700, 370}, //blue start column
			{650, 315},
			{650, 275},
			{650, 235},
			{650, 195},
			{650, 155},
			{650, 115}, //red start column
			{590, 115}, //red middle
			{530, 115},
			{530, 155},
			{530, 195},
			{530, 235},
			{530, 275},
			{530, 315}, //red last column
			{475, 365},
			{435, 365},
			{395, 365},
			{355, 365},
			{315, 365},
			{275, 365}, //green start column
			{275, 425}, //green middle
			{275, 490},
			{315, 490},
			{355, 490},
			{395, 490},
			{435, 490},
			{475, 490}, //green last column
			{530, 540},
			{530, 580},
			{530, 620},
			{530, 660},
			{530, 700},
			{530, 740}, //yellow start column
			{590, 740}, //yellow middle
			{650, 740},
			{650, 700},
			{650, 660},
			{650, 620},
			{650, 580},
			{650, 540}, //yellow last column
			{700, 490},
			{740, 490},
			{780, 490},
			{820, 490},
			{860, 490},
			{900, 490}, //blue start column
			{900, 430}, //blue middle
			{860, 430},
			{820, 430},
			{780, 430},
			{740, 430},
			{700, 430} //blue home column
		},

		//RED
		{
			{530, 115},
			{530, 155},
			{530, 195},
			{530, 235},
			{530, 275},
			{530, 315}, //red start column
			{475, 365},
			{435, 365},
			{395, 365},
			{355, 365},
			{315, 365},
			{275, 365}, //green start column
			{275, 425}, //green middle
			{275, 490},
			{315, 490},
			{355, 490},
			{395, 490},
			{435, 490},
			{475, 490}, //green last column
			{530, 540},
			{530, 580},
			{530, 620},
			{530, 660},
			{530, 700},
			{530, 740}, //yellow start column
			{590, 740}, //yellow middle
			{650, 740},
			{650, 700},
			{650, 660},
			{650, 620},
			{650, 580},
			{650, 540}, //yellow last column
			{700, 490},
			{740, 490},
			{780, 490},
			{820, 490},
			{860, 490},
			{900, 490}, //blue start column
			{900, 430}, //blue middle
			{900, 370},
			{860, 370},
			{820, 370},
			{780, 370},
			{740, 370},
			{700, 370}, //blue last column
			{650, 315},
			{650, 275},
			{650, 235},
			{650, 195},
			{650, 155},
			{650, 115}, //red start column
			{590, 115}, //red middle
			{590, 155},
			{590, 195},
			{590, 235},
			{590, 275},
			{590, 315} //red home column
		},

};

int Completion[4][4][2] = {
	{{550, 425}, {520, 455}, {520, 425}, {520, 395}}, //green triangle
	{{590, 460}, {620, 490}, {590, 490}, {560, 490}}, //yellow triangle
	{{620, 425}, {650, 395}, {650, 425}, {650, 455}}, //blue triangle
	{{590, 390}, {560, 360}, {590, 360}, {620, 360}}, //red triangle
};

//Setting size of canvas board
void SetCanvasSize(int width, int height)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width, 0, height, -1, 1); // set the screen size to given width and height.
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void createboard()
{

	//Showing player turns
	DrawString(30, 480, "Player's Turn: ", colors[BLACK]);
	if (playername == 1)
	{

		DrawString(30, 450, input_playername[0], colors[BLACK]);
		DrawString(30, 420, "Green", colors[BLACK]);
	}

	else if (playername == 2)
	{

		DrawString(30, 450, input_playername[1], colors[BLACK]);
		DrawString(30, 420, "Yellow", colors[BLACK]);
	}

	else if (playername == 3)
	{

		DrawString(30, 450, input_playername[2], colors[BLACK]);
		DrawString(30, 420, "Blue", colors[BLACK]);
	}

	else if (playername == 4)
	{
		DrawString(30, 450, input_playername[3], colors[BLACK]);
		DrawString(30, 420, "Red", colors[BLACK]);
	}

	//BOUNDARY
	DrawLine(260, 100, 940, 100, 10, colors[BLACK]); //bottom line
	DrawLine(260, 100, 260, 780, 10, colors[BLACK]); // left line
	DrawLine(260, 780, 940, 780, 10, colors[BLACK]); //top line
	DrawLine(940, 100, 940, 780, 10, colors[BLACK]); //right line

	//BIGGER RECTANGLES
	DrawSquare(262, 103, 250, colors[RED]);
	DrawSquare(262, 529, 250, colors[GREEN]);
	DrawSquare(688, 103, 250, colors[BLUE]);
	DrawSquare(688, 529, 250, colors[ORANGE]);

	DrawSquare(310, 143, 150, colors[WHITE]);
	DrawSquare(310, 575, 150, colors[WHITE]);
	DrawSquare(736, 143, 150, colors[WHITE]);
	DrawSquare(736, 575, 150, colors[WHITE]);

	//Ending Area (Triangles in Centre)
	DrawTriangle(509, 350, 691, 350, 600, 440, colors[RED]);
	DrawTriangle(509, 350, 509, 530, 600, 440, colors[GREEN]);
	DrawTriangle(509, 530, 691, 530, 600, 440, colors[ORANGE]);
	DrawTriangle(691, 350, 691, 530, 600, 440, colors[BLUE]);

	//Drawing Inner

	DrawRectangle(308, 410, 200, 60, colors[GREEN]);
	DrawRectangle(690, 410, 200, 60, colors[BLUE]);
	DrawRectangle(570, 530, 60, 200, colors[ORANGE]);
	DrawRectangle(570, 150, 60, 200, colors[RED]);

	//Safe Areas

	DrawRectangle(510, 150.6, 60.6, 42, colors[RED]);
	DrawRectangle(630, 188, 61, 42, colors[RED]);
	DrawRectangle(630, 690, 61, 41.8, colors[ORANGE]);
	DrawRectangle(510, 650, 61, 43, colors[ORANGE]);
	DrawRectangle(850, 350, 42, 60.6, colors[BLUE]);
	DrawRectangle(810, 471, 42, 60.6, colors[BLUE]);
	DrawRectangle(306, 470, 42, 60.6, colors[GREEN]);
	DrawRectangle(346, 350, 42, 60.8, colors[GREEN]);

	//Inner Board Lines

	for (int i = 530; i >= 350; i -= 60)
	{
		DrawLine(260, i, 510, i, 2, colors[BLACK]);
		DrawLine(690, i, 935, i, 2, colors[BLACK]);
	}

	for (int i = 308; i <= 508; i += 40)
	{

		DrawLine(i, 530, i, 350, 2, colors[BLACK]);
		DrawLine(i + 382, 530, i + 382, 350, 2, colors[BLACK]);
	}

	for (int i = 530; i <= 730; i += 40)
	{
		DrawLine(510, i, 690, i, 2, colors[BLACK]);
		DrawLine(510, i - 380, 690, i - 380, 2, colors[BLACK]);
	}

	for (int i = 510; i <= 690; i += 60)
	{

		DrawLine(i, 780, i, 530, 2, colors[BLACK]);
		DrawLine(i, 350, i, 100, 2, colors[BLACK]);
	}
}

void createdice()
{

	//input is equal to diceroll value selected in the thread, hence it allows the dice to be printed using glut
	int input = diceroll;
	int xcor = 25;
	int ycor = 150;

	DrawString(25, 260, "Dice: ", colors[BLACK]);

	if (input == 1)
	{

		DrawSquare(xcor, ycor, 90, colors[BLACK]);
		DrawCircle(xcor + 45, ycor + 45, 5, colors[WHITE]);
	}
	else if (input == 2)
	{

		DrawSquare(xcor, ycor, 90, colors[BLACK]);
		DrawCircle(xcor + 20, ycor + 70, 5, colors[WHITE]);
		DrawCircle(xcor + 68, ycor + 18, 5, colors[WHITE]);
	}
	else if (input == 3)
	{

		DrawSquare(xcor, ycor, 90, colors[BLACK]);
		DrawCircle(xcor + 20, ycor + 70, 5, colors[WHITE]);
		DrawCircle(xcor + 68, ycor + 18, 5, colors[WHITE]);
		DrawCircle(xcor + 45, ycor + 45, 5, colors[WHITE]);
	}
	else if (input == 4)
	{

		DrawSquare(xcor, ycor, 90, colors[BLACK]);
		DrawCircle(xcor + 20, ycor + 18, 5, colors[WHITE]);
		DrawCircle(xcor + 68, ycor + 70, 5, colors[WHITE]);
		DrawCircle(xcor + 20, ycor + 70, 5, colors[WHITE]);
		DrawCircle(xcor + 68, ycor + 18, 5, colors[WHITE]);
	}
	else if (input == 5)
	{

		DrawSquare(xcor, ycor, 90, colors[BLACK]);
		DrawCircle(xcor + 20, ycor + 18, 5, colors[WHITE]);
		DrawCircle(xcor + 68, ycor + 70, 5, colors[WHITE]);
		DrawCircle(xcor + 20, ycor + 70, 5, colors[WHITE]);
		DrawCircle(xcor + 68, ycor + 18, 5, colors[WHITE]);
		DrawCircle(xcor + 45, ycor + 45, 5, colors[WHITE]);
	}
	else if (input == 6)
	{

		DrawSquare(xcor, ycor, 90, colors[BLACK]);
		DrawCircle(xcor + 20, ycor + 18, 5, colors[WHITE]);
		DrawCircle(xcor + 68, ycor + 70, 5, colors[WHITE]);
		DrawCircle(xcor + 20, ycor + 70, 5, colors[WHITE]);
		DrawCircle(xcor + 68, ycor + 18, 5, colors[WHITE]);
		DrawCircle(xcor + 68, ycor + 45, 5, colors[WHITE]);
		DrawCircle(xcor + 20, ycor + 45, 5, colors[WHITE]);
	}
}

//Function used to draw a player's tokens at home index as per input
void drawHome(int player_index, int token_index)
{
	token_coordinate[player_index][token_index][0] = home[player_index][token_index][0];
	token_coordinate[player_index][token_index][1] = home[player_index][token_index][1];
}

//Function being called by glut in order to draw tokens at the token coordinates
void createtokens()
{
	for (int i = 0; i < NumberofTokens; i++)
	{
		DrawSquare(token_coordinate[0][i][0], token_coordinate[0][i][1], 25, colors[GREEN]);
		DrawString(token_coordinate[0][i][0] + 3, token_coordinate[0][i][1] + 3, Num2Str(i + 1), colors[BLACK]); //token identifiers as 1,2,3,4

		DrawSquare(token_coordinate[1][i][0], token_coordinate[1][i][1], 25, colors[ORANGE]);
		DrawString(token_coordinate[1][i][0] + 3, token_coordinate[1][i][1] + 3, Num2Str(i + 1), colors[BLACK]);

		DrawSquare(token_coordinate[2][i][0], token_coordinate[2][i][1], 25, colors[BLUE]);
		DrawString(token_coordinate[2][i][0] + 3, token_coordinate[2][i][1] + 3, Num2Str(i + 1), colors[BLACK]);

		DrawSquare(token_coordinate[3][i][0], token_coordinate[3][i][1], 25, colors[RED]);
		DrawString(token_coordinate[3][i][0] + 3, token_coordinate[3][i][1] + 3, Num2Str(i + 1), colors[BLACK]);
	}
}

//Function called by game display in order to draw the game on board
void DrawGame()
{
	createboard();
	createdice();
	createtokens();
}

//Major Function called for drawing repeatedly

void GameDisplay()
{

	//Setting background color
	glClearColor(1 /*Red Component*/, 1,						 //148.0/255/*Green Component*/,
				 1.1 /*Blue Component*/, 1 /*Alpha component*/); // Red==Green==Blue==1 --> White Colour
	glClear(GL_COLOR_BUFFER_BIT);

	DrawGame();

	glutSwapBuffers(); // do not modify this line..
}

void PrintableKeys(unsigned char key, int x, int y)
{
	if (key == 27 /* Escape key ASCII*/)
	{
		exit(1); // exit the program when escape key is pressed.
	}

	glutPostRedisplay();
}

void Timer(int m)
{
	// 1000FPS
	glutTimerFunc(1000.0, Timer, 0);
	glutPostRedisplay(); // Redisplaying
}

//Used function to test coordinates in order to draw on board
void MousePressedAndMoved(int x, int y)
{
	y = 830 - y;
	cout << x << " " << y << endl;
}


////////////////////////////////////////////////////////////FUNCTIONALITY USING THREADS///////////////////////////////////////////////////////////////////////

//Function called by each thread in order to roll the dice and return its value
int DiceRoll()
{
	diceroll = rand() % 6 + 1;
	return diceroll;
}

//Thread Function being called by all player threads, argument is player number sent during pthread_Create()
void *Player(void *arg)
{

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);  //Allows thread to be cancelled
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL); // Thread cancels at a cancellation point

	int *player_arg = (int *)arg; //Stores the player number

	int movement[NumberofTokens] = {0}; //For choosing token place

	while (1)
	{

		//Testing if no six was scored in 20 turns, then signal master thread to kill
		if (no_six_check[*player_arg] >= 20)
		{
			signalCancel[*player_arg] = true;
		}
		//Testing if hit rate of that player is 0 for 20 consecutive turns, then signal master thread to kill
		if (hit_rate_nochange[*player_arg] >= 20)
		{
			signalCancel[*player_arg] = true;
		}

		//Testing if all the players's tokens are inside home, then signal master thread for completion
		int check_comp[4] = {0};
		for (int j = 0; j < NumberofTokens; j++)
		{
			if (token_coordinate[*player_arg][j][0] == Completion[*player_arg][j][0] && token_coordinate[*player_arg][j][1] == Completion[*player_arg][j][1])
			{
				check_comp[*player_arg]++;
			}
		}

		if (check_comp[*player_arg] == NumberofTokens)
		{
			signalCompletion[*player_arg] = true;
		}

		pthread_testcancel(); ////Acts as the cancellation point for defferred cancelation;

		if (playerRound[*player_arg] == false && signalCompletion[*player_arg] == false && signalCancel[*player_arg] == false)
		{

			//Dice roll Critical Section
			sem_wait(&DiceRollSem);

			int dicevalue[3]; //Three consective dice rolls
			for (int i = 0; i < 3; i++)
			{
				dicevalue[i] = 0;
			}

			playername = *player_arg + 1; //Display name of the player with dice in hand
			bool AllSixes = true;
			for (int i = 0; i < 3; i++)
			{
				int diceoutcome = DiceRoll(); //Dice outcome calculated from calling the dice function
				dicevalue[i] = diceoutcome;
				if (diceoutcome != 6)
				{
					no_six_check[*player_arg]++; //Testing if six wasn't scored
					sleep(1);
					diceroll = 0;
					AllSixes = false;
					break;
				}
				no_six_check[*player_arg] = 0; //If six scored set value to zero
				sleep(1);
				diceroll = 0;
			}
			sem_post(&DiceRollSem);

			//Grid Access Critical Section
			sem_wait(&GridAccess);

			{
				//Checking which player currently has board access (used by hit rate thread)
				for (int i = 0; i < 4; i++)
				{
					BoardPlayer[i] = false;
				}

				BoardPlayer[*player_arg] = true;

				// First checking if all tokens are inside home and player doesn't get a six

				if (AllSixes == false)
				{
					bool skipturn = false;
					if (dicevalue[0] != 6)
					{
						skipturn = true;
						for (int i = 0; i < NumberofTokens; i++)
						{
							if (token_coordinate[*player_arg][i][0] != home[*player_arg][i][0] && token_coordinate[*player_arg][i][1] != home[*player_arg][i][1])
							{
								skipturn = false;
								break;
							}
						}
					}

					if (skipturn == false)
					{
						for (int i = 0; i < 3; i++) //For checking diceroll values
						{
							if (dicevalue[i] != 0)
							{
								//Testing if a player has all tokens in home or completion and they don't roll a six
								//then skip their turn
								if (dicevalue[i] != 6)
								{
									int skip_turnnosix = 0;
									for (int j = 0; j < NumberofTokens; j++)
									{
										if (token_coordinate[*player_arg][j][0] == home[*player_arg][j][0] && token_coordinate[*player_arg][j][1] == home[*player_arg][j][1])
										{
											skip_turnnosix++;
										}
										else if (token_coordinate[*player_arg][j][0] == Completion[*player_arg][j][0] && token_coordinate[*player_arg][j][1] == Completion[*player_arg][j][1])
										{
											skip_turnnosix++;
										}
									}
									if (skip_turnnosix == NumberofTokens)
									{
										break;
									}
								}

								int alltokensmovecheck = 0;						  //This acts as a counter, if all tokens can't be moved, then the turn is dismissed
								bool token_chosen_rand[NumberofTokens] = {false}; // Such that in each turn of the outer while loop unique token is chosen

								while (alltokensmovecheck < NumberofTokens)
								{
									//Testing if all the tokens have reached the completion during a dice roll
									int testtokens = 0;
									for (int j = 0; j < NumberofTokens; j++)
									{
										if (token_coordinate[*player_arg][j][0] == Completion[*player_arg][j][0] && token_coordinate[*player_arg][j][1] == Completion[*player_arg][j][1])
										{
											testtokens++;
										}
									}
									if (testtokens == NumberofTokens)
									{
										break;
									}

									//Selecting tokens and testing if they can be moved
									int token_selected = -1;
									while (1) //This loop ensures unique token is selected at random
									{
										token_selected = rand() % NumberofTokens + 0;
										if (token_chosen_rand[token_selected] == false)
										{
											token_chosen_rand[token_selected] = true;
											break;
										}
									}

									//If the selected token is at home
									if (token_coordinate[*player_arg][token_selected][0] == home[*player_arg][token_selected][0] && token_coordinate[*player_arg][token_selected][1] == home[*player_arg][token_selected][1])
									{

										if (dicevalue[i] == 6) //If dice rolls a six then move the token from home
										{
											movement[token_selected] = 1;
											token_coordinate[*player_arg][token_selected][0] = token_mov_grid[*player_arg][movement[token_selected]][0];
											token_coordinate[*player_arg][token_selected][1] = token_mov_grid[*player_arg][movement[token_selected]][1];
											break;
										}
									}
									else if ((token_coordinate[*player_arg][token_selected][0] != Completion[*player_arg][token_selected][0]) || (token_coordinate[*player_arg][token_selected][1] != Completion[*player_arg][token_selected][1]))
									{ // Play normally if the chosen token isn't already inside completion zone

										if (movement[token_selected] + dicevalue[i] >= 51) // If token is right outside home
										{
											if (HomeEnter[*player_arg] == false) // If player has zero hitrate, then make them run another round
											{
												for (int q = 0; q < dicevalue[i]; q++)
												{
													if (movement[token_selected] + 1 > 51)
													{
														movement[token_selected] = 0; //Movement from 0th index in another loop
													}
													else
													{
														movement[token_selected] += 1;
													}
												}

												token_coordinate[*player_arg][token_selected][0] = token_mov_grid[*player_arg][movement[token_selected]][0];
												token_coordinate[*player_arg][token_selected][1] = token_mov_grid[*player_arg][movement[token_selected]][1];
												break;
											}
											else // Hit Rate >=1
											{
												if ((movement[token_selected] + dicevalue[i]) == 57) //if token reaches completion by diceroll
												{
													token_coordinate[*player_arg][token_selected][0] = Completion[*player_arg][token_selected][0];
													token_coordinate[*player_arg][token_selected][1] = Completion[*player_arg][token_selected][1];
													break;
												}
												else if ((movement[token_selected] + dicevalue[i]) < 57) // if the dice value allows token to remain in home
												{
													movement[token_selected] += dicevalue[i];
													token_coordinate[*player_arg][token_selected][0] = token_mov_grid[*player_arg][movement[token_selected]][0];
													token_coordinate[*player_arg][token_selected][1] = token_mov_grid[*player_arg][movement[token_selected]][1];
													break;
												}
											}
										}
										else //Move Normally
										{
											movement[token_selected] += dicevalue[i];
											token_coordinate[*player_arg][token_selected][0] = token_mov_grid[*player_arg][movement[token_selected]][0];
											token_coordinate[*player_arg][token_selected][1] = token_mov_grid[*player_arg][movement[token_selected]][1];
											break;
										}
									}

									alltokensmovecheck++;
								}
								sem_post(&HitCheck); //Send signal to check for hit detecting
							}
						}
					}
				}
			}
			sem_post(&GridAccess);
			playerRound[*player_arg] = true;
		}
	}
}

//Thread to check when to allocate a new round to players, excluding the ones who are kicked out or completed
void *RoundCheckFunc(void *arg)
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);	  //Allows thread to be cancelled
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL); // Thread cancels when cancel is called by Master Function

	//Loop runs infinite to allot rounds, thread is cancelled by master thread after completion of the game
	while (1)
	{

		//Testing players only if they aren't kicked out or completed
		bool reset = true;
		for (int i = 0; i < 4; i++)
		{
			if (signalCompletion[i] == false && signalCancel[i] == false)
			{
				if (playerRound[i] == false)
				{
					reset = false;
				}
			}
		}

		int check_players = 0; //Testing if three players have passed the game or cancelled then don't allow the fourth to play
		for (int i = 0; i < 4; i++)
		{
			if (signalCompletion[i] == true || signalCancel[i] == true)
			{
				check_players++;
			}
		}

		if (check_players < 3)
		{
			if (reset == true)
			{
				for (int i = 0; i < 4; i++)
				{
					if (signalCompletion[i] == false && signalCancel[i] == false)
					{
						playerRound[i] = false;
					}
				}
			}
		}
	}

	pthread_exit(NULL);
}

//Thread testing player hits
void *HitFunc(void *arg)
{

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);	  //Allows thread to be cancelled
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL); //Thread cancels when cancel is called by Master Function

	while (1)
	{
		sem_wait(&HitCheck);
		int player_No;

		//Selecting the player currently holding the board
		for (int i = 0; i < 4; i++)
		{
			if (BoardPlayer[i] == true)
			{
				player_No = i;
			}
		}

		hit_rate_nochange[player_No]++;

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < NumberofTokens; j++)
			{
				for (int k = 0; k < NumberofTokens; k++)
				{
					if (i != player_No)
					{
						if (token_coordinate[player_No][j][0] == token_coordinate[i][k][0] && token_coordinate[player_No][j][1] == token_coordinate[i][k][1])
						{
							//First testing if the hit token is in its safe square or not
							if (token_coordinate[i][k][0] == safeSquares[i][0][0] && token_coordinate[i][k][1] == safeSquares[i][0][1])
							{

								break;
							}
							else if (token_coordinate[i][k][0] == safeSquares[i][1][0] && token_coordinate[i][k][1] == safeSquares[i][1][1])
							{

								break;
							}
							else // If it is outside safe square then confirm kill
							{
								//Testing if a block is formed then don't kill
								bool block_check = false;
								for (int m = 0; m < NumberofTokens; m++)
								{
									if (m != k) //Don't form a block with itself
									{
										if (token_coordinate[i][k][0] == token_coordinate[i][m][0] && token_coordinate[i][k][1] == token_coordinate[i][m][1])
										{
											block_check = true;
										}
									}
								}

								if (block_check == false)
								{
									drawHome(i, k); //Sending the killed token to its home
									playerHitRate[player_No] += 1;
									hit_rate_nochange[player_No] = 0;
								}

								break;
							}
						}
					}
				}
			}
		}
	}
	pthread_exit(NULL);
}

//Master thread to test home enter and test completion and or kicking out and then ending the game when all players are done
void *MasterFunc(void *arg)
{
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL); //Master thread will not be cancelled by any other thread

	int thread_cancel_check[4] = {0}; //To test that if thread is already cancelled then don't generate another cancellation request
	int win_index = 0;				  //Used to store player no in the winning thread
	int count_cancelled = 0;		  // To count the number of cancelled threads to test loop cancellation of master thread
	int cancel_index = 0;
	while (1)
	{
		//Testing hit rate of players to allow them to enter home
		for (int i = 0; i < 4; i++)
		{
			if (playerHitRate[i] > 0)
			{
				HomeEnter[i] = true;
			}
		}

		//If a player doesn't score six or a hit in 20 tries, kick him out
		for (int i = 0; i < 4; i++)
		{
			if (signalCancel[i] == true)
			{
				if (thread_cancel_check[i] != 1)
				{
					cancelledthread[cancel_index] = i;
					cancel_index++;
					count_cancelled++;

					thread_cancel_check[i] = 1;
					for (int j = 0; j < NumberofTokens; j++)
					{
						drawHome(i, j);
					}
					cout << "Thread " << i << " cancelled due to kicking out" << endl;
					pthread_cancel(Players[i]);
				}
			}
		}

		//Testing thread completion in order to cancel it
		int loopbreak = 0;
		for (int i = 0; i < 4; i++)
		{
			if (signalCompletion[i] == true)
			{
				loopbreak++;
				if (thread_cancel_check[i] != 1)
				{
					cout << "Thread " << i;
					thread_cancel_check[i] = 1;
					winningPlayers[win_index] = i;
					win_index++;

					if (pthread_cancel(Players[i]) == 0)
					{
						cout << " Cancelled successfully" << endl;
					}
					else
					{
						cout << " Cancelled unsuccessfully" << endl;
					}
				}
			}
		}

		//If three threads are cancelled then automatically allocate first place to the last remaining threads
		if (count_cancelled >= 3)
		{
			for (int i = 0; i < 4; i++)
			{
				if (thread_cancel_check[i] != 1)
				{
					winningPlayers[0] = i;
					if (pthread_cancel(Players[i]) == 0)
					{
						cout << "Eliminated last thread " << i << endl;
					}
					count_cancelled++;
				}
			}
			break;
		}

		if (loopbreak == 3 - count_cancelled) //Testing if players have finnished the game then cancel the remaining the player
		{
			for (int i = 0; i < 4; i++)
			{
				if (thread_cancel_check[i] != 1)
				{
					cout << "Loosing thread " << i;
					if (pthread_cancel(Players[i]) == 0)
					{
						cout << " Cancelled successfully" << endl;
					}
				}
			}
			break;
		}
	}
	pthread_cancel(RoundCheck);
	pthread_cancel(HitThread);
	pthread_exit(NULL);
}

void *MainFunction(void *arg)
{

	pthread_detach(pthread_self()); //Main Thread has detached itself from the main() func as return value is not required

	//Setting cancel checks and winning player checks to zero
	for (int i = 0; i < 4; i++)
	{
		no_six_check[i] = 0;
		signalCancel[i] = false;
		cancelledthread[i] = -1;
		winningPlayers[i] = -1;
		hit_rate_nochange[i] = 0;
	}

	//Setting all signals for completion to zero
	for (int i = 0; i < 4; i++)
	{
		signalCompletion[i] = false;
	}

	//Draw all tokens at home in the start
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < NumberofTokens; j++)
		{
			drawHome(i, j);
		}
	}

	//All Players intialized with unable to enter home
	for (int i = 0; i < 4; i++)
	{
		HomeEnter[i] = false;
	}

	//Hit Rate for all players initialized with zero
	for (int i = 0; i < 4; i++)
	{
		playerHitRate[i] = 0;
	}

	//Intializing all player rounds with False
	for (int i = 0; i < 4; i++)
	{
		playerRound[i] = false;
	}

	//Semaphores for solving mutual exclusion issue of Players playing
	sem_init(&DiceRollSem, 0, 1);
	sem_init(&GridAccess, 0, 1);

	//Sempahore for hit check
	sem_init(&HitCheck, 0, 0);

	//All Player threads created
	int n[4];
	for (int i = 0; i < 4; i++)
	{
		n[i] = i; //Each thread is sent a player no
		pthread_create(&Players[i], NULL, &Player, &n[i]);
		pthread_detach(Players[i]); //Detached because master function will cancel the thread based on completion or kicking out, hence no link to main func
	}

	//Thread for testing that each player gets a turn in a random round
	pthread_create(&RoundCheck, NULL, &RoundCheckFunc, NULL);
	pthread_detach(RoundCheck);

	//Collision Detection Thread
	pthread_create(&HitThread, NULL, &HitFunc, NULL);
	pthread_detach(HitThread);

	//Check If a player can enter home, checks when to kick out a player, and it cancels all other threads used in implementatiom
	pthread_create(&MasterThread, NULL, &MasterFunc, NULL);
	pthread_join(MasterThread, NULL);

	cout << "Game Completed" << endl;
	for (int i = 0; i < 3; i++)
	{
		if (winningPlayers[i] == 0)
		{
			cout << i + 1 << " Place : " << input_playername[0] << " Hit Rate : " << playerHitRate[0] << " Thread id: " << Players[0] << endl;
		}
		else if (winningPlayers[i] == 1)
		{
			cout << i + 1 << " Place : " << input_playername[1] << " Hit Rate : " << playerHitRate[1] << " Thread id: " << Players[1] << endl;
		}
		else if (winningPlayers[i] == 2)
		{
			cout << i + 1 << " Place : " << input_playername[2] << " Hit Rate : " << playerHitRate[2] << " Thread id: " << Players[2] << endl;
		}
		else if (winningPlayers[i] == 3)
		{
			cout << i + 1 << " Place : " << input_playername[3] << " Hit Rate : " << playerHitRate[3] << " Thread id: " << Players[3] << endl;
		}
	}

	cout << endl;

	for (int i = 0; i < 4; i++)
	{
		if (cancelledthread[i] == 0)
		{
			cout << "Player " << input_playername[0] << " was kicked out "
				 << " Hit Rate : " << playerHitRate[0] << " Thread id: " << Players[0] << endl;
		}
		else if (cancelledthread[i] == 1)
		{
			cout << "Player " << input_playername[1] << " was kicked out "
				 << " Hit Rate : " << playerHitRate[1] << " Thread id: " << Players[1] << endl;
		}
		else if (cancelledthread[i] == 2)
		{
			cout << "Player " << input_playername[2] << " was kicked out "
				 << " Hit Rate : " << playerHitRate[2] << " Thread id: " << Players[2] << endl;
		}
		else if (cancelledthread[i] == 3)
		{
			cout << "Player " << input_playername[3] << " was kicked out "
				 << " Hit Rate : " << playerHitRate[3] << " Thread id: " << Players[3] << endl;
		}
	}

	cout << "Number of tokens: " << NumberofTokens << endl;

	cout << "Press esc on ludo board to exit" << endl;

	sem_destroy(&DiceRollSem);
	sem_destroy(&GridAccess);
	sem_destroy(&HitCheck);
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	//Taking player names
	for (int i = 0; i < 4; i++)
	{
		cout << "Enter name of player " << i + 1 << " :";
		cin >> input_playername[i];
	}

	cout << endl;

	while (1)
	{
		cout << "Enter number of tokens: ";
		cin >> NumberofTokens;
		cout << endl;
		if (NumberofTokens >= 1 && NumberofTokens <= 4)
		{
			break;
		}
		else
		{
			cout << "Invalid token count" << endl;
		}
	}

	token_coordinate = new int **[4];
	for (int i = 0; i < 4; i++)
	{
		token_coordinate[i] = new int *[NumberofTokens];
		for (int j = 0; j < NumberofTokens; j++)
		{
			token_coordinate[i][j] = new int[2];
		}
	}

	srand(time(0));

	//Creating main thread for coordinate manipulation using threads and sempahores
	pthread_create(&MainThread, NULL, &MainFunction, NULL);

	int width = 1020, height = 840;
	InitRandomizer();							  // seed the random number generator...
	glutInit(&argc, argv);						  // Graphics Library
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); 
	glutInitWindowPosition(50, 50);				  // Window Positions
	glutInitWindowSize(width, height);
	glutCreateWindow("LUDO GAME");
	SetCanvasSize(width, height);

	//Glut function to display
	glutDisplayFunc(GameDisplay);

	//Glut functions for timer and mouse and key access
	glutKeyboardFunc(PrintableKeys);
	glutTimerFunc(1000.0, Timer, 0);

	glutMotionFunc(MousePressedAndMoved);

	//Handle function to call other functions
	glutMainLoop();

	return 1;
}
