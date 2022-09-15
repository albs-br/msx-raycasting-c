/*
*/

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#include "msxbios.h"
//#link "msxbios.c"

typedef uint8_t byte;
typedef uint16_t word;
typedef uint8_t bool;

#define TRUE 	    1
#define FALSE 	    0

void cursorxy(byte x, byte y) {
  POSIT(y+1+(x<<8));
}
void cputcxy(byte x, byte y, char ch) {
  cursorxy(x,y);
  CHPUT(ch);
}
void putstring(byte x, byte y, const char* str) {
  cursorxy(x,y);
  while (*str) {
    CHPUT(*str++);
  }
}
byte getchar(byte x, byte y) {
  word addr = 0x1800 | (x+1) | y*32; // TODO: use variable for base address
  byte result;
  result = RDVRM(addr);
  //LDIRMV(&result, addr, 1);
  return result;
}
void vsync() {
  __asm__("HALT");
}
void delay(int x) {
  while (x--) {
    vsync();
  }
}

#define CHAR(x) (x)

#define COLS (LINL32)
#define ROWS (LINLEN-5)

////////// GAME DATA

typedef struct {
  byte x;
  byte y;
  byte dir;
  word score;
  char head_attr;
  char tail_attr;
  char collided:1;
  char human:1;
} Player;

Player players[2];

byte credits = 0;
byte frames_per_move;

#define START_SPEED 12
#define MAX_SPEED 5
#define MAX_SCORE 7

///////////

const char BOX_CHARS[8] = {
  CHAR('+'), CHAR('+'), CHAR('+'), CHAR('+'),
  CHAR('-'), CHAR('-'), CHAR('!'), CHAR('!') };

typedef enum { D_RIGHT, D_DOWN, D_LEFT, D_UP } dir_t;
const char DIR_X[4] = { 1, 0, -1, 0 };
const char DIR_Y[4] = { 0, 1, 0, -1 };

void init_game() {
  memset(players, 0, sizeof(players));
  players[0].head_attr = CHAR('1');
  players[1].head_attr = CHAR('2');
  players[0].tail_attr = CHAR('@');
  players[1].tail_attr = CHAR('%');
  frames_per_move = START_SPEED;
}

void reset_players() {
  players[0].x = players[0].y = 5;
  players[0].dir = D_RIGHT;
  players[1].x = 25;
  players[1].y = 19;
  players[1].dir = D_LEFT;
  players[0].collided = players[1].collided = 0;
}

void draw_player(Player* p) {
  cputcxy(p->x, p->y, p->head_attr);
}

void human_control(Player* p) {
  byte dir = 0xff;
  byte joystick = GTSTCK(STCK_Joy1);
  if (!p->human) return;
  if (joystick == STCK_W) dir = D_LEFT;
  if (joystick == STCK_E) dir = D_RIGHT;
  if (joystick == STCK_N) dir = D_UP;
  if (joystick == STCK_S) dir = D_DOWN;
  // don't let the player reverse
  if (dir < 0x80 && dir != (p->dir ^ 2)) {
    p->dir = dir;
  }
}


//void play_game();


void main() {

  int nScreenWidth = 32;			// Console Screen Size X (columns)
  int nScreenHeight = 24;			// Console Screen Size Y (rows)
  int nMapWidth = 16;				// World Dimensions
  int nMapHeight = 16;
  
  float fPlayerX = 14.7f;			// Player Start Position
  float fPlayerY = 5.09f;
  float fPlayerA = 0.0f;			// Player Start Rotation
  float fFOV = 3.14159f / 4.0f;			// Field of View
  float fDepth = 16.0f;				// Maximum rendering distance
  float fSpeed = 5.0f;				// Walking Speed
  
  // pre defining some vars
  int nTestX, nTestY;

  INIT32();
  //putstring(0, 0, "Hello world");

  
  
  while(TRUE) { 
    for (int x = 0; x < nScreenWidth; x++) {
      
      
      // For each column, calculate the projected ray angle into world space
      float fRayAngle = (fPlayerA - fFOV/2.0f) + ((float)x / (float)nScreenWidth) * fFOV;
      
      // Find distance to wall
      float fStepSize = 0.1f;		  // Increment size for ray casting, decrease to increase										
      float fDistanceToWall = 0.0f;	  // resolution

      bool bHitWall = FALSE; 		  // Set when ray hits wall block

      float fEyeX = sinf(fRayAngle); 	  // Unit vector for ray in player space
      float fEyeY = cosf(fRayAngle);
      
      putstring(8, 12, "Still working"); //[debug]

      // Incrementally cast ray from player, along ray angle, testing for 
      // intersection with a block
      while (!bHitWall && fDistanceToWall < fDepth) {
        fDistanceToWall += fStepSize;
        nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
        nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

        // Test if ray is out of bounds
        if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight) {
          bHitWall = TRUE;			// Just set distance to maximum depth
          fDistanceToWall = fDepth;
        }
        else {
        }
      }
      

    }
  }
}
