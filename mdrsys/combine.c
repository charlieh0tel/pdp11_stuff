#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char **argv) {
  int fd1 = open(argv[1], O_RDONLY);
  int fd2 = open(argv[2], O_RDONLY);
  while (1) {
    char cc1, cc2;
    int nr1 = read(fd1, &cc1, 1);
    int nr2 = read(fd2, &cc2, 1);
    if (nr1 == 1 && nr2 == 1) {
      write(1, &cc1, 1);
      write(1, &cc2, 1);
    } else if (nr1 == 0 && nr2 == 0) {
      exit(0);
    } else  {
      fprintf(stderr, "nr1=%d\n", nr1);
      fprintf(stderr, "nr2=%d\n", nr2);
      exit(1);
    }
  }
}


/* Local Variables: */
/* compile-command: "cc -Wall -o combine combine.c" */
/* End: */
