#include <stdio.h>
#include <syscall.h>

int
main (int argc, char **argv)
{
  printf("fibonacci(%d): %d\n", atoi(*(argv+1)), fibonacci(atoi(*(argv+1))));
  printf("max of four int: %d\n", max_of_four_int(atoi(*(argv+1)), atoi(*(argv+2)), atoi(*(argv+3)), atoi(*(argv+4))));

	return EXIT_SUCCESS;
}