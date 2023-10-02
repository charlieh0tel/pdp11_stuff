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

  int fd_l = open(argv[1], O_RDONLY);
  if (fd_l < 0) {
    fprintf(stderr, "%s: open %s failed: %m\n", argv[0], argv[1]);
    exit(1);
  }

  int fd_h = open(argv[2], O_RDONLY);
  if (fd_h < 0) {
    fprintf(stderr, "%s: open %s failed: %m\n", argv[0], argv[2]);
    exit(1);
  }

  while (1) {
    char cc_l, cc_h;
    int nr_l = read(fd_l, &cc_l, 1);
    if (nr_l < 0) {
      fprintf(stderr, "%s: read file_l: %m\n", argv[0]);
      exit(1);
    }
    
    int nr_h = read(fd_h, &cc_h, 1);
    if (nr_h < 0) {
      fprintf(stderr, "%s: read file_h: %m\n", argv[0]);
      exit(1);
    }

    if (nr_l == 1 && nr_h == 1) {
      write(1, &cc_l, 1);
      write(1, &cc_h, 1);
    } else if (nr_l == 0 && nr_h == 0) {
      exit(0);
    } else if (nr_l == 0) {
      fprintf(stderr, "%s: file_l is shorter than file_h\n", argv[0]);
      exit(1);
    } else {
      assert(nr_h == 0);
      fprintf(stderr, "%s: file_h is shorter than file_l\n", argv[0]);
      exit(1);
    }
  }
}
