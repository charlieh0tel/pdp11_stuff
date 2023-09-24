CFLAGS=-Wall -Werror

all: mdrsys_le16.bin checksum
.PHONY: clean

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

clean:
	rm -f mdrsys_le16.bin r4_19_apr_83_le16.bin r5_19_apr_83_le16.bin r6_19_apr_83_le16.bin
	rm -f combine checksum

.PHONY: clean

.DELETE_ON_ERROR:
