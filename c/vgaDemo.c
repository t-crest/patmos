#include <stdio.h>
#include <stdlib.h>
#include <machine/spm.h>
#include <machine/exceptions.h>
#include <machine/patmos.h>
#include <stdbool.h>
#include <math.h>
#include "vgalib.h"

// I/O timer:
#define timer_low (*((volatile _SPM int *)0xF002000C))
#define timer_high (*((volatile _SPM int *)0xF0020008))

int main() {
	
	flushScreen(0);
	
	

	printf("Press ENTER to proceed to the next demo:\n");
  char c;
	while(c	!= '\n'){c = getchar();}
	c = 'd';

	int select = 1;

			flushScreen(0);
			line_draw(31,20,31,237,28); //horizontal line
			line_draw(40,25,130,249,30); //normal line
			line_draw(29,130,187,130,255); //vertical line
			line_draw(280,20,150,300,51); //normal line
			line_draw(291,385,2,329,199);
			line_draw(290,300,225,100,216);
			line_draw(70,44,199,23,200);	

	printf("Press ENTER to proceed to the next demo:\n");
	while(c	!= '\n'){c = getchar();}
	c = 'd';

			flushScreen(0);	
			filledrectangle_draw(50,260,70,70,28);
			filledrectangle_draw(150,230,11,89,216);
			rectangle_draw(13,17,100,300,200);	
			rectangle_draw(71,100,200,33,51);	
			rectangle_draw(230,310,140,140,30);

	printf("Press ENTER to proceed to the next demo:\n");
	while(c	!= '\n'){c = getchar();}
	c = 'd';

			flushScreen(0);	
			triangle_draw(240,13,174,160,50,120,255);
			triangle_draw(200,250,200,340,75,250,30);
			triangle_draw(140,30,200,70,140,100,216);
			equitriangle_draw(270,300,270,345,199);	
			equitriangle_draw(49,350,49,450,51);	

	printf("Press ENTER to proceed to the next demo:\n");
	while(c	!= '\n'){c = getchar();}
	c = 'd';

			flushScreen(0);
			circle_draw(150,200,23,216);	
			circle_draw(279,21,67,199);
			circumference_draw(210,257,65,51);	
			circumference_draw(34,350,51,30); 
			circle_drawMaja(100,100,45,255);
	

	printf("Press ENTER to proceed to the next demo:\n");
	while(c	!= '\n'){c = getchar();}
	c = 'd';

			flushScreen(0);
			string_draw("This is a string. We can show letters, as well as 133959104140, as well as @#[""%&/(*+-.:,-. Strings automatically go to a new line when the limit of the screen is reached. ",0,0,199);
			string_draw("Strings can be placed in arbitrary position within the screen!",47,270,216);
			string_draw("Last but not least, the color can be selected.",175,123,51);

	printf("Press ENTER to proceed to the next demo:\n");
	while(c	!= '\n'){c = getchar();}
	c = 'd';

			
			colorpalette();

	printf("Press ENTER to proceed to the next demo:\n");
	while(c	!= '\n'){c = getchar();}
	c = 'd';


			colorpalette_labeled();

	printf("Press ENTER to proceed to the next demo:\n");
	while(c	!= '\n'){c = getchar();}
	c = 'd';

			flushScreen(0);
			string_draw("Sierpinski fractal",0,220,255);	
			sierpinski_draw(299,30,299,370,28,5);

	printf("Press ENTER to proceed to the next demo:\n");
	while(c	!= '\n'){c = getchar();}
	c = 'd';

			flushScreen(0);
			stickman(255);

}






































































