#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>

uint16_t checksum(uint8_t *data, size_t size) {
  uint16_t sum = 0;

  for (size_t i = 0; i < size; i++) {
    sum += data[i];
  }

  return sum;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s file\n", argv[0]);
    exit(1);
  }

  int fd = open(argv[1], O_RDONLY);
  if (fd < 0) {
    fprintf(stderr, "open: %m\n");
    exit(1);
  }

  struct stat stat;
  int rc = fstat(fd, &stat);
  if (fd < 0) {
    fprintf(stderr, "stat failed: %m\n");
    exit(1);
  }

  uint8_t *buf = malloc(stat.st_size);
  ssize_t nr = read(fd, buf, stat.st_size);
  if (nr < 0) {
    fprintf(stderr, "read failed: %m\n");
    exit(1);
  }
  if (nr != stat.st_size) {
    fprintf(stderr, "failed to read all the bytes\n");
    exit(1);
  }

  uint16_t cs = checksum(buf, nr);
  printf("%04x\n", cs);

  return 0;
}
