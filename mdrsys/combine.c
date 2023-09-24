#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "usage: %s file_l file_h\n", argv[0]);
    exit(1);
  }

  int fd1 = open(argv[1], O_RDONLY);
  if (fd1 < 0) {
    fprintf(stderr, "%s: open %s failed: %m\n", argv[0], argv[1]);
    exit(1);
  }

  int fd2 = open(argv[2], O_RDONLY);
  if (fd2 < 0) {
    fprintf(stderr, "%s: open %s failed: %m\n", argv[0], argv[2]);
    exit(1);
  }

  while (1) {
    char cc1, cc2;
    int nr1 = read(fd1, &cc1, 1);
    if (nr1 < 0) {
      fprintf(stderr, "%s: read file_l: %m\n", argv[0]);
      exit(1);
    }
    
    int nr2 = read(fd2, &cc2, 1);
    if (nr2 < 0) {
      fprintf(stderr, "%s: read file_h: %m\n", argv[0]);
      exit(1);
    }

    if (nr1 == 1 && nr2 == 1) {
      write(1, &cc1, 1);
      write(1, &cc2, 1);
    } else if (nr1 == 0 && nr2 == 0) {
      exit(0);
    } else if (nr1 == 0) {
      fprintf(stderr, "%s: file_l is shorter than file_h\n", argv[0]);
      exit(1);
    } else {
      assert(nr2 == 0);
      fprintf(stderr, "%s: file_h is shorter than file_l\n", argv[0]);
      exit(1);
    }
  }
}


/* Local Variables: */
/* compile-command: "cc -Wall -o combine combine.c" */
/* End: */
