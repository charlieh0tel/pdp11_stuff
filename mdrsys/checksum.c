#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <fcntl.h>

int main(int argc, char **argv) {
  int fd = 0;
  switch (argc) {
  case 1:
    break;
  case 2:
    fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
      fprintf(stderr, "open: %m\n");
      exit(1);
    }
    break;
  default:
    fprintf(stderr, "usage: %s [file]\n", argv[0]);
    exit(1);
  }

  uint16_t sum = 0;
  while (1) {
    uint8_t cc;
    int nr = read(fd, &cc, 1);
    if (nr == 0) {
      break;
    }
    if (nr != 1) {
      fprintf(stderr, "read failed: %m\n");
      exit(1);
    }

    sum += cc;
  }

  printf("%04x\n", sum);
}
