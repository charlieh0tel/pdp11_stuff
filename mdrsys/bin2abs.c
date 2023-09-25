#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <fcntl.h>
#include <endian.h>
#include <assert.h>

// https://www.pcjs.org/software/dec/pdp11/tapes/absloader/

const int kSignature = 0x0001;
const int kChunkSize = 0x6000;
const uint16_t kNoStartAddress = 0x0001;

struct BlockHeader {
  uint16_t signature_le;
  uint16_t block_length_le;
  uint16_t load_address_le;
} __attribute__((packed));

uint8_t checksum(uint8_t *data, size_t length) {
  uint8_t sum = 0;
  for (size_t i = 0; i < length; ++i) {
    sum += data[i];
  }
  return sum;
}

int main(int argc, char **argv) {
  assert(sizeof(struct BlockHeader) == 6);

  int fd = 0;
  switch (argc) {
  case 2:
    break;
  case 3:
    fd = open(argv[2], O_RDONLY);
    if (fd < 0) {
      fprintf(stderr, "%s: open: %m\n", argv[0]);
      exit(1);
    }
    break;
  default:
    fprintf(stderr, "usage: %s load_addr [file]\n", argv[0]);
    exit(1);
  }

  unsigned long load_address = strtoul(argv[1], NULL, 0);
  uint16_t start_address = kNoStartAddress;
  
  fprintf(stderr, "load address = 0x%04lx = 0%06lo\n",
	  load_address, load_address);
  fprintf(stderr, "start address = 0x%04x = 0%06o\n",
	  start_address, start_address);

  while (1) {
    if (load_address > 0xffff) {
      fprintf(stderr, "%s: load address too large\n", argv[0]);
      exit(0);
    }

    uint8_t data[kChunkSize];
    int nr = read(fd, data, kChunkSize);
    if (nr == 0) {
      break;
    }
    if (nr < 0) {
      fprintf(stderr, "%s: read: %m\n", argv[0]);
      exit(1);
    }

    struct BlockHeader header;
    header.signature_le = htole16(kSignature);
    header.block_length_le = htole16(nr + sizeof(header));
    header.load_address_le = htole16(load_address);

    uint8_t cs =
      0 - (checksum(data, nr) +
	   checksum((uint8_t *) &header, sizeof(header)));

    write(1, &header, sizeof(header));
    write(1, data, nr);
    write(1, &cs, 1);

    load_address += nr;
  }

  struct BlockHeader final_header;
  final_header.signature_le = htole16(kSignature);
  final_header.block_length_le = htole16(sizeof(final_header));
  final_header.load_address_le = htole16(start_address);
  write(1, &final_header, sizeof(final_header));
}
