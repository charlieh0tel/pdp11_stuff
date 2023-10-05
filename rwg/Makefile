CFLAGS:=-Wall -Werror
LOAD_ADDRESS:=0100000
START_ADDRESS:=0143000

all: rwg_le16.bin
.PHONY: clean

#mdrsys.abs: mdrsys_le16.bin bin2abs
#	./bin2abs $(LOAD_ADDRESS) $(START_ADDRESS) mdrsys_le16.bin > $@

rwg_le16.bin: 0_le16.bin 1_le16.bin 2_le16.bin 3_le16.bin 4_le16.bin
	cat $^ > $@

0_le16.bin: combine 0l.bin 0h.bin
	./combine 0l.bin 0h.bin > $@

1_le16.bin: combine 1l.bin 1h.bin
	./combine 1l.bin 1h.bin > $@

2_le16.bin: combine 2l.bin 2h.bin
	./combine 2l.bin 2h.bin > $@

3_le16.bin: combine 3l.bin 3h.bin
	./combine 3l.bin 3h.bin > $@

4_le16.bin: combine 4l.bin 4h.bin
	./combine 4l.bin 4h.bin > $@

combine: ../mdrsys/combine.c
	cc -o $@ $^

bin2abs: ../mdrsys/bin2abs.c
	cc -o $@ $^

clean:
	rm -f rwg_le16.bin rwg.abs
	rm -f 0_le16.bin 1_le16.bin 2_le16.bin 3_le16.bin 4_le16.bin
	rm -f combine bin2abs

.PHONY: clean

.DELETE_ON_ERROR:
