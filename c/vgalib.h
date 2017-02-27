

#ifndef _VGALIB_H
#define _VGALIB_H

#include <stdio.h>
#include <stdlib.h>
#include <machine/spm.h>
#include <machine/exceptions.h>
#include <machine/patmos.h>
#include <stdbool.h>
#include <math.h>
#define MEMADDR_0 0x1E2B40
#define ROWS 300
#define COLS 400

#define sgn(x) ((x<0)?-1:((x>0)?1:0))
//#define width_ctrl(x) ((x<0)?-1:((x>(COLS-1))?-1:x))
//#define depth_ctrl(x) ((x<MEMADDR_0)?-1:((x>(ROWS-1))?-1:x))
#define wordAddress(x) (x&(-4))
#define offset(x) (x&3)

// I/O timer:
#define timer_low (*((volatile _SPM int *)0xF002000C))
#define timer_high (*((volatile _SPM int *)0xF0020008))

//###########################################################
//###################### Memory location ####################
//###########################################################

int memLoc(int depth,int width){ //depth is vertical, width horizontal
	return MEMADDR_0+depth*COLS+width;
}

//############################################################
//###################### POINT DRAWER ########################
//############################################################

void point_draw(int x, int y, int color){
    int mem = memLoc(x,y);

    if(x<0 || x>(ROWS-1) || y<0 || y>(COLS-1)){return;}
	
	asm volatile(
    "sbc [%0] = %1;"
    "nop;"
    :
    : "r" (mem), "r" (color)); 
    return;
}

void double_point_draw(int x, int y, int color){ //draws two pixels in the same color next to each other
    int mem = memLoc(x,y);
    
    if(x<0 || x>(ROWS-1) || y<0 || y>(COLS-1)){return;}
	
	asm volatile(
    "shc [%0] = %1;"
    "nop;"
    : 
    : "r" (mem), "r" (color+(color<<8))); 
    return;
}

//############################################################
//####################### LINE DRAWER ########################
//############################################################

void line_draw(int x1, int y1, int x2, int y2, int color){

	int dx,dy,sdx,sdy,xt,yt,xp,yp,prev_yp,curr_yp;
	float step;
	float slope;

	//case in which lenght=1pixel
	if(x1==x2 && y1==y2){
		point_draw(x1,y1,color); return;	
	}	

  dx=x2-x1;      
  dy=y2-y1; 
	sdx=sgn(dx);
  sdy=sgn(dy);
	
	//rearrange extreme points
	if ((sdx==-1 && sdy==-1) || (sdx==-1 && (sdy==1 || sdy ==0)) || (sdx==0 && sdy==-1)){
		xt=x1; yt=y1;
		x1=x2; y1=y2;
		x2=xt; y2=yt;
	}

	xp=x1;
	step=y1;
	yp = y1;

	//return if line is totally out of screen boundaries
	if(x2<0 || x1>(ROWS-1) || (y1<0 && y2<0) || (y1>(COLS-1) && y2>(COLS-1))){return;}	
 
	if (dx==0){ //case for horizontal line
		
		//controls for line exceeding boundaries
		if(y1<0){y1=0;}
		if(y2>(COLS-1)){y2=(COLS-1);}

		//case in which line length is less than 4
		if((y2-y1)<4){
			for(yp=y1; yp<=y2; yp++){
				point_draw(x1,yp,color);
			}
			return;
		}		

		//write the line without beginning and final points (due to word misalignment).
		for(yp=(y1+4-offset(memLoc(x1,y1))); yp<(y2-offset(memLoc(x2,y2))); yp=yp+4){
			*(int *)(wordAddress(memLoc(x1,yp))) = color+(color<<8)+(color<<16)+(color<<24);
		}		
	
		//beginning of the line
		for(yp=y1; yp<(y1+4-offset(memLoc(x1,y1))); yp++){
			point_draw(x1,yp,color);
		}

		//end of the line
		for(yp=(y2-offset(memLoc(x2,y2))); yp<=y2; yp++){
			point_draw(x2,yp,color);
		}

	}else{ //non horizontal lines
		slope=(float)dy / (float)dx;
		prev_yp = (int) step;
		while(xp <= x2){
			curr_yp = (int) step;
			if (prev_yp == curr_yp){
				yp=curr_yp;
				if(xp>=0 && xp <= (ROWS-1) && yp>=0 && yp<=(COLS-1)){
		  	  	point_draw(xp,yp,color);
					}
			}
			if (sgn(slope)==1){
				for(yp=prev_yp+1; yp <= curr_yp; yp++){
					if(xp>=0 && xp <= (ROWS-1) && yp>=0 && yp<=(COLS-1)){
		  	  	point_draw(xp,yp,color);
					}			
				}
			}else{
				for(yp=curr_yp; yp < prev_yp; yp++){
					if(xp>=0 && xp <= (ROWS-1) && yp>=0 && yp<=(COLS-1)){
		  	  	point_draw(xp,yp,color);
					}			
				}
			}

			prev_yp = curr_yp;
			step = step + slope;
			xp = xp + 1;
		}
	}		
	return;
}

//###########################################################
//###################### FLUSH SCREEN #######################
//###########################################################

void flushScreen(int color){

	for(int i=0; i<ROWS; i++){
		for(int j=0; j<COLS; j=j+4){
			*(int *)(wordAddress(memLoc(i,j))) = color+(color<<8)+(color<<16)+(color<<24);;
		}
	}
}


//###########################################################
//##################### Draw triangle #######################
//###########################################################

void triangle_draw(int x1, int y1, int x2, int y2, int x3, int y3, int color){

	line_draw(x1,y1,x2,y2,color); //A
	line_draw(x2,y2,x3,y3,color); //B
	line_draw(x3,y3,x1,y1,color); //C
}

//###########################################################
//################ Draw equilatelar triangle ################
//###########################################################

void equitriangle_draw(int x1, int y1, int x2, int y2, int color){
	
	int xt,yt,length,height;
	length = y2-y1;
	height = 0.87*length;
	xt = x1 - height;
	yt = y1 + (length>>1);
	triangle_draw(x1,y1,x2,y2,xt,yt,color);
}

//###########################################################
//################## Draw fractal structure #################
//###########################################################

void sierpinski_draw(int x1, int y1, int x2, int y2, int color, int iter){	

	int b1x, b1y, vx, vy, b2x, b2y, length, height,xt,yt,xl,yl,xr,yr,h;
	length = y2-y1;
	height = 0.87*length; //0.87
	xt = x1 - height;
	yt = y1 + (length>>1);
	line_draw(x1,y1,xt,yt,color); //A
	line_draw(xt,yt,x2,y2,color); //B
	line_draw(x1,y1,x2,y2,color); //base of the sierpinski
	for(int i=0; i<30;i++){printf(" ");}
	if(iter != 0){
		h= height>>1;
		xl = x1-h;
		yl = y1 + (length>>2);
		xr = xl;
		yr = y2 - (length>>2);	
		iter = iter-1;
		sierpinski_draw(x1,y1,x1,yt,color, iter);
		sierpinski_draw(x1,yt,x1,y2,color, iter);
		sierpinski_draw(xl,yl,xl,yr,color, iter);
	}else{
		return;
	}	
}

//#########################################################
//################### RECTANGLE DRAW ######################
//#########################################################

void rectangle_draw(int xp, int yp, int height, int width, int color){

	height = height -1;
	width = width -1;

	line_draw(xp,yp+width,xp+height,yp+width,color); // B
	//for(int k=0; k<6000; k++){printf("ciao");}
	
	line_draw(xp,yp,xp+height,yp,color); // D
	//for(int k=0; k<6000; k++){printf("ciao");}
	
	line_draw(xp+height,yp,xp+height,yp+width,color); // C
	//for(int k=0; k<6000; k++){printf("ciao");}
	
	line_draw(xp,yp,xp,yp+width,color); // A
}

//#########################################################
//################ FILLED RECTANGLE DRAW ##################
//#########################################################

void filledrectangle_draw(int xp, int yp, int height, int width, int color){

	for(int i=xp; i<xp+height; i++){
		line_draw(i,yp,i,yp+width-1,color);
	}
	 // B
	
}

//#########################################################
//################## CHAR ENCODING ########################
//#########################################################

const char character_data[95][5] = {
  {0x00, 0x00, 0x00, 0x00, 0x00},
  {0x00, 0x5F, 0x5F, 0x00, 0x00},
  {0x00, 0x07, 0x00, 0x07, 0x00},
  {0x14, 0x7F, 0x14, 0x7F, 0x14},
  {0x24, 0x2A, 0x7F, 0x2A, 0x12},
  {0x23, 0x13, 0x08, 0x64, 0x62},
  {0x36, 0x49, 0x55, 0x22, 0x50},
  {0x00, 0x05, 0x03, 0x00, 0x00},
  {0x00, 0x1C, 0x22, 0x41, 0x00},
  {0x00, 0x41, 0x22, 0x1C, 0x00},
  {0x14, 0x08, 0x3E, 0x08, 0x14},
  {0x08, 0x08, 0x3E, 0x08, 0x08},
  {0x00, 0x50, 0x30, 0x00, 0x00},
  {0x08, 0x08, 0x08, 0x08, 0x08},
  {0x00, 0x60, 0x60, 0x00, 0x00},
  {0x20, 0x10, 0x08, 0x04, 0x02},
  {0x3E, 0x51, 0x49, 0x45, 0x3E},
  {0x00, 0x42, 0x7F, 0x40, 0x00},
  {0x42, 0x61, 0x51, 0x49, 0x46},
  {0x22, 0x49, 0x49, 0x49, 0x36},
  {0x18, 0x14, 0x12, 0x7F, 0x10},
  {0x2F, 0x49, 0x49, 0x49, 0x31},
  {0x3E, 0x49, 0x49, 0x49, 0x32},
  {0x03, 0x01, 0x71, 0x09, 0x07},
  {0x36, 0x49, 0x49, 0x49, 0x36},
  {0x26, 0x49, 0x49, 0x49, 0x3E},
  {0x00, 0x36, 0x36, 0x00, 0x00},
  {0x00, 0x56, 0x36, 0x00, 0x00},
  {0x08, 0x14, 0x22, 0x41, 0x00},
  {0x14, 0x14, 0x14, 0x14, 0x14},
  {0x00, 0x41, 0x22, 0x14, 0x08},
  {0x02, 0x01, 0x51, 0x09, 0x06},
  {0x32, 0x49, 0x79, 0x41, 0x3E},
  {0x7C, 0x0A, 0x09, 0x0A, 0x7C},
  {0x7F, 0x49, 0x49, 0x49, 0x36},
  {0x3E, 0x41, 0x41, 0x41, 0x22},
  {0x7F, 0x41, 0x41, 0x41, 0x3E},
  {0x7F, 0x49, 0x49, 0x49, 0x41},
  {0x7F, 0x09, 0x09, 0x09, 0x01},
  {0x3E, 0x41, 0x49, 0x49, 0x7A},
  {0x7F, 0x08, 0x08, 0x08, 0x7F},
  {0x00, 0x41, 0x7F, 0x41, 0x00},
  {0x30, 0x40, 0x40, 0x40, 0x3F},
  {0x7F, 0x08, 0x14, 0x22, 0x41},
  {0x7F, 0x40, 0x40, 0x40, 0x40},
  {0x7F, 0x02, 0x0C, 0x02, 0x7F},
  {0x7F, 0x02, 0x04, 0x08, 0x7F},
  {0x3E, 0x41, 0x41, 0x41, 0x3E},
  {0x7F, 0x09, 0x09, 0x09, 0x06},
  {0x3E, 0x41, 0x51, 0x21, 0x5E},
  {0x7F, 0x09, 0x09, 0x09, 0x76},
  {0x26, 0x49, 0x49, 0x49, 0x32},
  {0x01, 0x01, 0x7F, 0x01, 0x01},
  {0x3F, 0x40, 0x40, 0x40, 0x3F},
  {0x1F, 0x20, 0x40, 0x20, 0x1F},
  {0x3F, 0x40, 0x38, 0x40, 0x3F},
  {0x63, 0x14, 0x08, 0x14, 0x63},
  {0x03, 0x04, 0x78, 0x04, 0x03},
  {0x61, 0x51, 0x49, 0x45, 0x43},
  {0x7F, 0x41, 0x41, 0x00, 0x00},
  {0x02, 0x04, 0x08, 0x10, 0x20},
  {0x00, 0x41, 0x41, 0x7F, 0x00},
  {0x04, 0x02, 0x01, 0x02, 0x04},
  {0x40, 0x40, 0x40, 0x40, 0x40},
  {0x00, 0x01, 0x02, 0x04, 0x00},
  {0x20, 0x54, 0x54, 0x54, 0x78},
  {0x7F, 0x48, 0x44, 0x44, 0x38},
  {0x38, 0x44, 0x44, 0x44, 0x20},
  {0x38, 0x44, 0x44, 0x48, 0x7F},
  {0x38, 0x54, 0x54, 0x54, 0x18},
  {0x08, 0x7E, 0x09, 0x01, 0x02},
  {0x0C, 0x52, 0x52, 0x52, 0x3E},
  {0x7F, 0x08, 0x04, 0x04, 0x78},
  {0x00, 0x44, 0x7D, 0x40, 0x00},
  {0x20, 0x40, 0x44, 0x3D, 0x00},
  {0x7F, 0x10, 0x28, 0x44, 0x00},
  {0x00, 0x41, 0x7F, 0x40, 0x00},
  {0x7C, 0x04, 0x18, 0x04, 0x78},
  {0x7C, 0x08, 0x04, 0x04, 0x78},
  {0x38, 0x44, 0x44, 0x44, 0x38},
  {0x7C, 0x14, 0x14, 0x14, 0x08},
  {0x08, 0x14, 0x14, 0x18, 0x7C},
  {0x7C, 0x08, 0x04, 0x04, 0x08},
  {0x48, 0x54, 0x54, 0x54, 0x20},
  {0x04, 0x3F, 0x44, 0x40, 0x20},
  {0x3C, 0x40, 0x40, 0x20, 0x7C},
  {0x1C, 0x20, 0x40, 0x20, 0x1C},
  {0x3C, 0x40, 0x38, 0x40, 0x3C},
  {0x44, 0x28, 0x10, 0x28, 0x44},
  {0x0C, 0x50, 0x50, 0x50, 0x3C},
  {0x44, 0x64, 0x54, 0x4C, 0x44},
  {0x00, 0x08, 0x36, 0x41, 0x00},
  {0x00, 0x00, 0x7F, 0x00, 0x00},
  {0x00, 0x41, 0x36, 0x08, 0x00},
  {0x08, 0x04, 0x08, 0x10, 0x08}};

//#########################################################
//################### DRAW LETTERS ########################
//#########################################################

void drawLetter(int beginX, int beginY, unsigned char letID, int color){
  unsigned char letter[5];
  char i; char j;
  //Assign character data to letter array
  for (i = 0; i<5; i++){
    letter[i] = character_data[(letID-0x20)][i];
  }
  for(i = 0; i<7; i++){ //rows
    for (j=0; j<5; j++){ //columns + 3 zeroes
      //print bit j of row i at position x+j,y+i
      if(((letter[j] & (1<<i)) == (1<<i))) {
				point_draw(beginX+i,beginY+j,color);
			}
    }
  }
}

//#########################################################
//################### DRAW STRING #########################
//#########################################################

void string_draw( char phrase[], int x, int y, int color){
	
	int xBegin = x;
	int yBegin = y;
	//int size = sizeof(*phrase) - 1;
	for(int i=0; phrase[i] != '\0'; i++){
		drawLetter(x,y, phrase[i], color);
		y = y + 8;
		if(y > 390){y = yBegin; x = x + 8;}
		if(x >291){x = 2;}
	}
	
}

//########################################################
//################## CIRCLE DRAWER #######################
//########################################################

void circle_drawMaja(int x0, int y0, int radius, int color)
{
	int x = 0, y = radius;
	int dp = 1 - radius;
	do
	{
		if (dp < 0)
			dp = dp + 2 * (++x) + 3;
		else
			dp = dp + 2 * (++x) - 2 * (--y) + 5;
		//for(int i=0; i< 350; i++){printf("ff");}
				
		point_draw(x0 + x, y0 + y,color); //putpixel(x0 + x, y0 + y, 15);     //For the 8 octants	
		
		point_draw(x0 - x, y0 + y,color);
		point_draw(x0 + x, y0 - y,color);
		point_draw(x0 - x, y0 - y,color);
		point_draw(x0 + y, y0 + x,color);
		point_draw(x0 - y, y0 + x,color);
		point_draw(x0 + y, y0 - x,color);
		point_draw(x0 - y, y0 - x,color);
		
	} while (x < y);
}

//#########################################################
//##################### CIRCLE DRAWER #####################
//#########################################################

bool is_circle(int radius, int x, int y){
    int square_dist = x*x + y*y;
		return (square_dist < (radius*radius));
}

void circle_draw(int xc, int yc, int r, int color){
	int j;	
	for(int i=-r+1; i<r; i++){
		j=-r;
		while(is_circle(r,i,j) == false){j++;} 
		line_draw(xc+i,yc+j,xc+i,yc-j,color);
	}	
}

void circumference_draw(int xc, int yc, int r, int color){
	int j;	
	for(int i=-r+1; i<r; i++){
		j=-r;
		while(is_circle(r,i,j) == false){j++;} 
		if(i==(-r+1) || i==(r-1)){line_draw(xc+i,yc+j,xc+i,yc-j,color);}else{
			point_draw(xc+i,yc+j,color);	
			point_draw(xc+i,yc-j,color);				
		}
	}	
}

//#########################################################
//############## DISPLAY COLOR PALETTE ####################
//#########################################################

void colorpalette(){
  flushScreen(0);
	string_draw( "256 COLOR PALETTE", 70, 135, 255);
	int tinta = 0;
	int tinta0 = 0;
	for(int j=8; j<392; j=j+12){	
		for(int i=102; i<198; i=i+12){		
			filledrectangle_draw(i,j,10,10,tinta);
			tinta = tinta + 4;
			if(tinta >= 256){tinta0 = tinta0+1; tinta = tinta0;}
		}
	}
}

//#########################################################
//############## DISPLAY COLOR PALETTE ####################
//#########################################################

void colorpalette_labeled(){
  flushScreen(0);
	string_draw( "256 COLOR PALETTE DEC", 0, 120, 28);
	int tinta = 0;
	int tinta0 = 0;
	char nr[4];
	for(int j=8; j<400; j=j+50){	
		for(int i=12; i<300; i=i+9){		
			filledrectangle_draw(i,j,7,7,tinta);
			sprintf(nr,"%d",tinta);
			string_draw(nr,i,j+10,255);
			tinta = tinta + 1;
			if(tinta >= 256){tinta0 = tinta0+1; tinta = tinta0;}
		}
	}
}

//#########################################################
//############### WE ALL LOVE STICKMAN ####################
//#########################################################

void stickman(int color){
	int xtop, ytop, xcenter, ycenter, xbot, ybot;
	xtop=100;
	ytop=200;
	xcenter=130;
	ycenter=200;
	xbot=150;
	ybot=200;
	circumference_draw(75,200,25,color);
	line_draw(xtop,ytop,xbot,ybot,color);
	line_draw(xcenter,ycenter,xcenter-20,180,color);
	line_draw(xcenter,ycenter,xcenter,220,color);
	line_draw(xbot,ybot,280,180,color);
	line_draw(xbot,ybot,280,220,color);
	string_draw("Thank you for your attention!", 20 , 80 ,color);
}


//#########################################################
//################## DIGITAL CLOCK ########################
//#########################################################

void digiClock(){

	char hh[3];
	char mm[3];
	char ss[3];

	int timeNow, timeNow_1, timeDiff;
	timeNow_1 = 0;		

	int y = 160;
	int x = 0;

	int h=0; int m=0; int s=0;	
	
  char c;

	while(1){
		
		sprintf(ss,"%d",s);
		string_draw(ss,x,y+64,255);
		string_draw("s",x,y+80,255);	

		sprintf(mm,"%d",m);
		string_draw(mm,x,y+32,255);
		string_draw("m",x,y+48,255);	

		sprintf(hh,"%d",h);
		string_draw(hh,x,y,255);
		string_draw("h",x,y+16,255);	

		timeNow = timer_low;
		while(timer_low < timeNow + 10){}
		sprintf(ss,"%d",s);
		string_draw(ss,x,y+64,0);
		string_draw("s",x,y+80,0);	

		sprintf(mm,"%d",m);
		string_draw(mm,x,y+32,0);
		string_draw("m",x,y+48,0);	

		sprintf(hh,"%d",h);
		string_draw(hh,x,y,0);
		string_draw("h",x,y+16,0);
		s = s + 1;			
		if(s==59){s=0; m=m+1;}		
		if(m==59){m=0;h=h+1;}		
		if(h==24){h=0; break;}		
	}

}

//#########################################################
//####################### HELP ############################
//#########################################################

void help(){

	char str[] = "Patmos VGA library functions:";
	char str0[]="- point_draw(int x, int y, int color);";
	char str1[]="- line_draw(int x1, int y1, int x2, int y2, int color);";
	char str2[]="- triangle_draw(int x1, int y1, int x2, int y2, int x3, int y3, int color):";
	char str3[]="- equitriangle_draw(int x1, int y1, int x2, int y2, int color);";
	char str4[]="- rectangle_draw(int xp, int yp, int height, int width, int color);";
	char str5[]="- filledrectangle_draw(int xp, int yp, int height, int width, int color);";
	char str6[]="- circle_draw(int xc, int yc, int r, int color);";
	char str7[]="- circumference_draw(int xc, int yc, int r, int color);";
	char str8[]="- sierpinski_draw(int x1, int y1, int x2, int y2, int color, int iter);";
	char str9[]= "- string_draw(char phrase[], int x, int y, int color);";
	char str10[]="- colorpalette();";
	char str11[]="- colorpalette_labeled();";
	char str12[]="- stickman(int color);";
	char str13[]="- flushScreen();";

	
	string_draw(str,   0,   4,28);	
	string_draw(str0,  20,  4,255);	
	string_draw(str1,  40,  4,255);	
	string_draw(str2,  60,  4,255);	
	string_draw(str3,  80,  4,255);	
	string_draw(str4,  100, 4,255);	
	string_draw(str5,  120, 4,255);	
	string_draw(str6,  140, 4,255);	
	string_draw(str7,  160, 4,255);	
	string_draw(str8,  180, 4,255);	
	string_draw(str9,  200, 4,255);	
	string_draw(str10, 220, 4,255);	
	string_draw(str11, 240, 4,255);	
	string_draw(str12, 260, 4,255);	
	string_draw(str13, 280, 4,255);	

}



#endif /* _MACHSPM_H */
