#include <stdlib.h>

#define list_size 5

/*void a();
void b();
void init(int numbers[]);
void bubbleSort(int numbers[]);
void fibonacci(int numbers[]);
*/

__attribute__((noreturn)) void exit(int r) {
  asm("brcf $r0");
  for(;;);
  }

__attribute__((noinline)) void a() {
  int x, y, z;
  y = 100;
  for (int i = 0; i < 5; i++) {
    x = i + 1;
    z = x + y;
  }
}

__attribute__((noinline)) void init(int numbers[]) {
  for (int i = 0; i < list_size; i++) {
    numbers[i] = i % 7;
  }
}

__attribute__((noinline)) void bubbleSort(int numbers[])
{
  //init(numbers);
  int i, j, temp;
  for (i = (list_size - 1); i > 0; i--)
    {
      for (j = 1; j <= i; j++)
	{
	  if (numbers[j-1] > numbers[j])
	    {
	      temp = numbers[j-1];
	      numbers[j-1] = numbers[j];
	      numbers[j] = temp;
	    }
	}
    }
}

__attribute__((noinline)) void b() {
  int x, y, z;
  y = 30;
  for (int i = 0; i < 5; i++) {
    x = i + 1;
    z = x + y;
  }
}

__attribute__((noinline)) void fibonacci(int numbers[]) {
  int b = 1;
  int a, fib = 0;

  for (int i = 0; i < list_size; i++)
    {
      fib = a + b;
      a = b;
      b = fib;
      numbers[i] = fib;
    }
}

__attribute__((noinline)) int main() {
  int my_list[list_size];
  while (1) {
    init(my_list);
    a();
    b();
    bubbleSort(my_list);
    fibonacci(my_list);
  }
}
