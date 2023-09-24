These EPROM images are from an MRV11-C/M8048 card I purchased off of
eBay.  It appears to contain an inventory, bidding and process control
program for Weirton Steel Company.

The checksums match the labels on the chips; they are believed to be
good.

The chips (Intel D2732A, 4K x 8bit) were installed in sockets XE41/42,
XE35/36, XE29/30, XE23/24.

The M8048 was configured as follows:

2-3
	RXCX to C35

6-7
	Direct addressing

21-22 to 23 (Logic 1)
18-19-20 to 24 (Logic 0)
	 Boostrap start 143000

25-26	AD11
28-29	AD12
31-32	AD13
34-35	AD14
37-38	AD15

53-55
	Enable 64KB direct mode

57-60	Bit 17
58-59	Bit 16
63-64	Bit 14
65-66	Bit 13
	17-I, 16-I, 15-R, 14-I, 13-I
	Starting Address 100000 (Bank 4)

70-71
	Enable low-byte MUX

74-75	CE6: XE29, XE30
76-77	CE5: XE35, XE36
78-79	CE4: XE41, XE41

88-89
	Enable bootstrap

98-100	Pin 18 to pin 20 for chip set 5
101-103	Pin 18 ot pin 20 for chip set 4
104-106 Pin 18 to pin 20 for chip set 6

112-113 CE bit bus to address bit A11, used as chip input A10
115-116 Cip interconn loop for chip pin 21 to address bit A12, used as chip input A11

CSR is configured for 177000:
    Bit 4 90-91 - R, Bit 3 96-97 - R, Bit 2 94-95 - R, Bit 1 92-93 - R
