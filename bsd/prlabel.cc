#include <cassert>
#include <cerrno>
#include <cinttypes>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include <boost/endian/conversion.hpp>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sysexits.h>
#include <unistd.h>

#define DKTYPENAMES
#include "disklabel.h"

std::string argv0;

constexpr int kSectorSize = 512;

void Usage() {
  std::cerr << "usage: " << argv0 << " file" << std::endl;
  std::exit(EX_USAGE);
}

void Fail(int exit_code, const std::string &message) {
  std::cerr << argv0 << ": " << message << std::endl;
  std::exit(exit_code);
}

void ReadOrLose(int fd, void *buf, ssize_t count) {
  ssize_t rc = read(fd, buf, count);
  if (rc != count) {
    std::ostringstream out;
    out << "failed to read bytes, rc=" << rc << ", errno=" << errno << " ("
        << std::strerror(errno) << ")";
    Fail(EX_IOERR, out.str());
  }
}

std::string Dump(const void *buf, size_t n, bool decorations=true) {
  const int kBytesPerRow = 16;
  assert(buf != nullptr);
  const uint8_t *p = static_cast<const uint8_t *>(buf);
  std::ostringstream dump;
  for (uint32_t i = 0; i < n;) {
    if (decorations) {
      dump << "0x" << std::hex << std::setfill('0') << std::setw(8) << i << ": ";
    }
    for (int j = 0; j < kBytesPerRow; j++) {
      uint32_t o = i + j;
      if (o < n) {
        unsigned char b = p[o];
	dump << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(b) << " ";
      } else {
	dump << "   ";
      }
    }
    if (decorations) {
      dump << " | ";
      for (int j = 0; j < kBytesPerRow; j++) {
	uint32_t o = i + j;
	if (o == n) {
	  break;
	}
	unsigned char b = p[o];
	if (b >= ' ' && b <= '~') {
	  dump << static_cast<char>(b);
	} else {
	  dump << '.';
	}
      }
    }
    dump << std::endl;
    i += kBytesPerRow;
  }
  return dump.str();
}

std::string convert_d_type(uint8_t d_type) {
  if (d_type < DKMAXTYPES) {
    return dktypenames[d_type];
  } else {
    std::ostringstream out;
    out << "BAD(0x" << std::hex << std::setfill('0') << std::setw(2) << d_type << ")";
    return out.str();
  }
}

std::string convert_d_flags(uint16_t d_flags) {
  std::ostringstream out;
  bool first = true;
  out << "{";
  if (d_flags & D_REMOVABLE) {
    out << "REMOVABLE";
    first = false;
  }
  if (d_flags & D_ECC) {
    if (!first) out << ", ";
    out << "ECC";
  }
  if (d_flags & D_BADSECT) {
    if (!first) out << ", ";
    out << "BADSECT";
  }
  if (d_flags & D_RAMDISK) {
    if (!first) out << ", ";
    out << "RAMIDSK";
  }

  if (d_flags & ~(D_REMOVABLE | D_ECC | D_BADSECT | D_RAMDISK)) {
    if (!first) out << ", ";
    out << "(others) flags=0x" << std::hex << std::setfill('0') << std::setw(4) << d_flags;
  }
  out << "}";

  return out.str();
}

std::string convert_fstype(uint8_t fstype) {
 if (fstype < FSMAXTYPES) {
    return fstypenames[fstype];
  } else {
    std::ostringstream out;
    out << "BAD(0x" << std::hex << std::setfill('0') << std::setw(2) << fstype << ")";
    return out.str();
  }
}


uint16_t dkcksum(const struct disklabel *lp)
{
  auto start = reinterpret_cast<const uint8_t *>(lp);
  auto end = reinterpret_cast<const uint8_t *>(&lp->d_partitions[lp->d_npartitions]);

  uint16_t sum = 0;
  while (start < end)
    sum ^= *start++;
  return sum;
}


int main(int argc, char **argv) {
  argv0 = argv[0];
  
  if (argc != 2) {
    Usage();
  }

  std::string path(argv[1]);
  const int fd = open(path.c_str(), O_RDONLY);
  if (fd < 0) {
    Fail(EX_NOINPUT, "failed to open " + path + " for reading");
  }
  
  int nsectors = LABELSECTOR + 1;
  std::vector<uint8_t> buf(nsectors * kSectorSize);
  ReadOrLose(fd, &buf[0], nsectors * kSectorSize);
  
  assert(sizeof(struct disklabel) < kSectorSize);

  const void *label_start = &buf[LABELSECTOR * kSectorSize];
  const struct disklabel *disklabel = reinterpret_cast<const struct disklabel *>(label_start);

  assert(sizeof(DISKMAGIC) - 1 == 4);

  if (strncmp(disklabel->d_magic, DISKMAGIC, sizeof(DISKMAGIC) - 1) != 0) {
    std::cerr << Dump(disklabel, sizeof(*disklabel));
    Fail(EX_DATAERR, "d_magic not matched");
  }
  
  if (strncmp(disklabel->d_magic2, DISKMAGIC, sizeof(DISKMAGIC) - 1) != 0) {
    std::cerr << Dump(disklabel, sizeof(*disklabel));
    Fail(EX_DATAERR, "d_magic2 not matched");
  }

  const auto d_type = boost::endian::little_to_native(disklabel->d_type);
  std::cout << "d_type=" << convert_d_type(d_type) << std::endl;
  const auto d_subtype = boost::endian::little_to_native(disklabel->d_subtype);
  std::cout << "d_subtype=" << convert_d_type(d_subtype) << std::endl;
  std::cout << std::endl;

  std::cout << "d_typename=" << disklabel->d_typename << std::endl;
  std::cout << "d_un.un_d_packname=\"" << disklabel->d_un.un_d_packname << "\"" <<  std::endl;
  std::cout << std::endl;

  std::cout << "d_secsize=" << boost::endian::little_to_native(disklabel->d_secsize) << std::endl;
  std::cout << "d_nsectors=" << boost::endian::little_to_native(disklabel->d_nsectors) << std::endl;
  std::cout << "d_ntracks=" << boost::endian::little_to_native(disklabel->d_ntracks) << std::endl;
  std::cout << "d_ncylinders=" << boost::endian::little_to_native(disklabel->d_ncylinders) << std::endl;
  std::cout << "d_secpercyl=" << boost::endian::little_to_native(disklabel->d_secpercyl) << std::endl;
  std::cout << "d_secperunit=" << boost::endian::little_to_native(disklabel->d_secperunit) << std::endl;
  std::cout << std::endl;

  std::cout << "d_sparespertrack=" << boost::endian::little_to_native(disklabel->d_sparespertrack) << std::endl;
  std::cout << "d_sparespercyl=" << boost::endian::little_to_native(disklabel->d_sparespercyl) << std::endl;
  std::cout << std::endl;

  std::cout << "d_acylinders=" << boost::endian::little_to_native(disklabel->d_acylinders) << std::endl;
  std::cout << std::endl;
  
  std::cout << "d_rpm=" << boost::endian::little_to_native(disklabel->d_rpm) << std::endl;
  std::cout << "d_interleave=" << int(boost::endian::little_to_native(disklabel->d_interleave)) << std::endl;
  std::cout << "d_trackskew=" << int(boost::endian::little_to_native(disklabel->d_trackskew)) << std::endl;
  std::cout << "d_cylskew=" << int(boost::endian::little_to_native(disklabel->d_cylskew)) << std::endl;
  std::cout << "d_headswitch=" << int(boost::endian::little_to_native(disklabel->d_headswitch)) << std::endl;
  std::cout << "d_trkseek=" << int(boost::endian::little_to_native(disklabel->d_trkseek)) << std::endl;
  std::cout << std::endl;

  std::cout << "d_flags=" << convert_d_flags(boost::endian::little_to_native(disklabel->d_flags)) << std::endl;
  std::cout << std::endl;

  std::cout << "d_drivedata=" << Dump(&(disklabel->d_drivedata), NDDATA, false) << std::endl;
  std::cout << "d_spare=" << Dump(&(disklabel->d_spare), NSPARE, false) << std::endl;

  std::cout << "d_bbsize=" << int(boost::endian::little_to_native(disklabel->d_bbsize)) << std::endl;
  std::cout << "d_sbsize=" << int(boost::endian::little_to_native(disklabel->d_sbsize)) << std::endl;
  std::cout << std::endl;

  auto d_npartitions = boost::endian::little_to_native(disklabel->d_npartitions);
  std::cout << "d_npartitions=" << d_npartitions << std::endl;
  if (d_npartitions > MAXPARTITIONS) {
    std::cout << "* illegal number of partitions; clamping to " << MAXPARTITIONS << std::endl;
    d_npartitions = MAXPARTITIONS;
  }

  const auto checksum = dkcksum(disklabel);
  if (checksum) {
    std::cout << std::hex << "checksum=" << std::hex << checksum << std::endl;
    std::cerr << "Partition checksum is bad." << std::endl;
  }

  for (int i=0; i < d_npartitions; ++i) {
    std::cout << "part #" << i << std::endl;
    std::cout << "  p_fstype=" << convert_fstype(disklabel->d_partitions[i].p_fstype) << std::endl;
    std::cout << "  p_size=" << boost::endian::little_to_native(disklabel->d_partitions[i].p_size) << std::endl;
    std::cout << "  p_offset=" << boost::endian::little_to_native(disklabel->d_partitions[i].p_offset) << std::endl;
    std::cout << "  p_fsize=" << boost::endian::little_to_native(disklabel->d_partitions[i].p_fsize) << std::endl;
  }
}
