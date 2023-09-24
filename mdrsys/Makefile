CFLAGS:=-Wall -Werror
LOAD_ADDRESS:=0100000

all: mdrsys.abs checksum
.PHONY: clean

mdrsys.abs: mdrsys_le16.bin bin2abs
	./bin2abs $(LOAD_ADDRESS) mdrsys_le16.bin > $@

mdrsys_le16.bin: r4_19_apr_83_le16.bin r5_19_apr_83_le16.bin r6_19_apr_83_le16.bin
	cat $^ > $@

r4_19_apr_83_le16.bin: combine r4l_19_apr_83_cs3103.bin r4h_19_apr_83_cs89e5.bin
	./combine r4l_19_apr_83_cs3103.bin r4h_19_apr_83_cs89e5.bin > $@

r5_19_apr_83_le16.bin: combine r5l_19_apr_83_cs959c.bin r5h_19_apr_83_cse327.bin
	./combine r5l_19_apr_83_cs959c.bin r5h_19_apr_83_cse327.bin > $@

r6_19_apr_83_le16.bin: combine r6l_19_apr_83_cs8009.bin r6h_19_apr_83_cs71a1.bin
	./combine r6l_19_apr_83_cs8009.bin r6h_19_apr_83_cs71a1.bin > $@

combine: combine.c

checksum: checksum.c

bin2abs: bin2abs.c

clean:
	rm -f mdrsys_le16.bin mdrsys.abs
	rm -f r4_19_apr_83_le16.bin
	rm -f r5_19_apr_83_le16.bin
	rm -f r6_19_apr_83_le16.bin mdrsys.abs
	rm -f combine checksum bin2abs

.PHONY: clean

.DELETE_ON_ERROR:
