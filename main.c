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
/*
byte getchar(byte x, byte y) {
  word addr = 0x1800 | (x+1) | y*32; // why (x+1) ????    // TODO: use variable for base address
  byte result;
  result = RDVRM(addr);
  //LDIRMV(&result, addr, 1);
  return result;
}
*/
void putchar(byte x, byte y, char ch) {
  word addr = 0x1800 | x | (y<<5);
  WRTVRM(addr, ch);
  return;
}
void vsync() {
  __asm__("HALT");
}
void delay(int x) {
  while (x--) {
    vsync();
  }
}



/*
void readInput(Player* p) {
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
*/

//void play_game();


void main() {

  int nScreenWidth = 32;			// Console Screen Size X (columns)
  int nScreenHeight = 24;			// Console Screen Size Y (rows)
  int nMapWidth = 16;				// World Dimensions
  int nMapHeight = 16;
  
  // original position 14.7, 5.09
  float fPlayerX = 9.0f;			// Player Start Position
  float fPlayerY = 5.0f;
  float fPlayerA = 0.0F; //3.14159f; //0.0f;			// Player Start Rotation
  const float fFOV = 3.14159f / 4.0f;		// Field of View (is this equal to 45º?)
  const float fDepth = 16.0f;				// Maximum rendering distance
  const float fSpeed = 5.0f;				// Walking Speed
  
  // pre defining some vars
  int nTestX, nTestY;

  // Create Map of world space # = wall block, . = space
  const char* map[16] = { 
    "#########.......",
    "#...............",
    "#.....4567######",
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "###............#",
    "##.............#",
    "#......4321..###",
    "#......#.......#",
    "#......#.......#",
    "#..............#",
    "#......#####3###",
    "#..............#",
    "################"
    }; 


  //const wchar_t small_char[10] = { L'锕', L'吖', L'啊', L'阿', L'呵', L'嗄', L'埃', L'哀', L'哎'}; 
  //const char* t1 = "test\r\n1sdsd\r\ndwdw\r\nwfwfwef";

  //char test = '1';
  //test += '3';

  

  INIT32();
  
  //putstring(2, 1, "ad");
  //CHPUT(GetChFromMap(0, 2);

  
  // TODO; fiz bug: is plotting the columns on reverse order
  
  while(TRUE) { 
    for (int x = 0; x < nScreenWidth; x++) {
      // pre define vars
      int nCeiling, nFloor;
      
      char wallType = '#';
      
      // For each column, calculate the projected ray angle into world space
      float fRayAngle = (fPlayerA - fFOV/2.0f) + (((float)x / (float)nScreenWidth) * fFOV);
      
      // Find distance to wall
      float fStepSize = 0.1f;		  // Increment size for ray casting, decrease to increase resolution
      float fDistanceToWall = 0.0f;

      bool bHitWall = FALSE; 		  // Set when ray hits wall block

      float fEyeX = sinf(fRayAngle); 	  // Unit vector for ray in player space
      float fEyeY = cosf(fRayAngle);
      
      /*
      float fStepX = fEyeX * fStepSize;
      float fStepY = fEyeY * fStepSize;
      
      float fDistanceToWallX = 0.0f;
      float fDistanceToWallY = 0.0f;
      */
      
      //putstring(8, 12, "Still working"); //[debug]

      // Incrementally cast ray from player, along ray angle, testing for 
      // intersection with a block
      while (!bHitWall && (fDistanceToWall < fDepth)) {
        fDistanceToWall += fStepSize;

        nTestX = (int)(fPlayerX + (fEyeX * fDistanceToWall));
        nTestY = (int)(fPlayerY + (fEyeY * fDistanceToWall));
        
        /*
        fDistanceToWallX += fStepX;
        nTestX = (int)(fPlayerX + fDistanceToWallX);
	fDistanceToWallY += fStepY;
        nTestY = (int)(fPlayerY + fDistanceToWallY);
        */
        


        // Test if ray is out of bounds
        if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight) {
          bHitWall = TRUE;			// Just set distance to maximum depth
          fDistanceToWall = fDepth;
        }
        else {
          // Ray is inbounds so test to see if the ray cell is a wall block
          //if (map.c_str()[nTestX * nMapWidth + nTestY] == '#') { //OLD
          char temp = map[nTestY][nTestX];
          if (temp != '.') {
            // Ray has hit wall
            bHitWall = TRUE;
            wallType = temp;
          } 
        }
      }
      
      // Calculate distance to ceiling and floor
      nCeiling = (float)(nScreenHeight/2.0) - nScreenHeight / ((float)fDistanceToWall);
      nFloor = nScreenHeight - nCeiling;

      /*
      // Shader walls based on distance
      short nShade = ' ';
      if (fDistanceToWall <= fDepth / 4.0f)			nShade = 0x2588;	// Very close	
      else if (fDistanceToWall < fDepth / 3.0f)		nShade = 0x2593;
      else if (fDistanceToWall < fDepth / 2.0f)		nShade = 0x2592;
      else if (fDistanceToWall < fDepth)				nShade = 0x2591;
      else											nShade = ' ';		// Too far away
      */
      
      // Draw Column
      for (int y = 0; y < nScreenHeight; y++) {
        // Each Row
        if(y <= nCeiling) {
          //screen[y*nScreenWidth + x] = ' ';
	  //putstring(x, y, " ");
          putchar(x, y, ' ');
        }
        else if(y > nCeiling && y <= nFloor) {
          //screen[y*nScreenWidth + x] = nShade;
          //putchar(x, y, '#');
          putchar(x, y, wallType);
        }
        else { // Floor
          /*
          // Shade floor based on distance
          float b = 1.0f - (((float)y -nScreenHeight/2.0f) / ((float)nScreenHeight / 2.0f));
          if (b < 0.25)		nShade = '#';
          else if (b < 0.5)	nShade = 'x';
          else if (b < 0.75)	nShade = '.';
          else if (b < 0.9)	nShade = '-';
          else				nShade = ' ';
          screen[y*nScreenWidth + x] = nShade;
          */
        }
      }
      
    }
  }
}
