/*
 * Copyright (c) 1987, 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)disklabel.h	8.1.1 (2.11BSD) 1995/04/13
 */

#ifndef	_SYS_DISKLABEL_H_
#define	_SYS_DISKLABEL_H_

#include <inttypes.h>

/*
 * Disk description table, see disktab(5)
 */
#define	_PATH_DISKTAB	"/etc/disktab"
#define	DISKTAB		"/etc/disktab"		/* deprecated */

/*
 * Each disk has a label which includes information about the hardware
 * disk geometry, filesystem partitions, and drive specific information.
 * The label is in block 0 or 1, possibly offset from the beginning
 * to leave room for a bootstrap, etc.
 */

/* XXX these should be defined per controller (or drive) elsewhere, not here! */
#define LABELSECTOR	1			/* sector containing label */
#define LABELOFFSET	0			/* offset of label in sector */

#define DISKMAGIC	"V\x82WE"		/* The disk magic number */
#define	MAXPARTITIONS	8

/*
 * 2.11BSD's disklabels are different than 4.4BSD for a couple reasons:
 *
 *	1) D space is precious in the 2.11 kernel.  Many of the fields do
 *	   not need to be a 'long' (or even a 'short'), a 'short' (or 'char')
 *	   is more than adequate.  If anyone ever ports the FFS to a PDP11 
 *	   changing the label format will be the least of the problems.
 *
 *	2) There is no need to support a bootblock more than 512 bytes long.
 *	   The hardware (disk bootroms) only read the first sector, thus the
 *	   label is always at sector 1 (the second half of the first filesystem
 *	   block).
 *
i * Almost all of the fields have been retained but with reduced sizes.  This
 * is for future expansion and to ease the porting of the various utilities
 * which use the disklabel structure.  The 2.11 kernel uses very little other
 * than the partition tables.  Indeed only the partition tables are resident
 * in the kernel address space, the actual label block is allocated external to
 * the kernel and mapped in as needed.
*/

struct disklabel {
	char	d_magic[4];		/* the magic number */
	uint8_t	d_type;			/* drive type */
	uint8_t	d_subtype;		/* controller/d_type specific */
	char	d_typename[16];		/* type name, e.g. "eagle" */
	/* 
	 * d_packname contains the pack identifier and is returned when
	 * the disklabel is read off the disk or in-core copy.
	 * d_boot0 is the (optional) name of the primary (block 0) bootstrap
	 * as found in /mdec.  This is returned when using
	 * getdiskbyname(3) to retrieve the values from /etc/disktab.
	 */
#if defined(KERNEL) || defined(STANDALONE)
	char	d_packname[16];			/* pack identifier */ 
#else
	union {
		char	un_d_packname[16];	/* pack identifier */ 
		char	*un_d_boot0;		/* primary bootstrap name */
	} d_un; 
#define d_packname	d_un.un_d_packname
#define d_boot0		d_un.un_d_boot0
#endif	/* ! KERNEL or STANDALONE */
			/* disk geometry: */
	uint16_t	d_secsize;		/* # of bytes per sector */
	uint16_t	d_nsectors;		/* # of data sectors per track */
	uint16_t	d_ntracks;		/* # of tracks per cylinder */
	uint16_t	d_ncylinders;		/* # of data cylinders per unit */
	uint16_t	d_secpercyl;		/* # of data sectors per cylinder */
	uint32_t	d_secperunit;		/* # of data sectors per unit */
	/*
	 * Spares (bad sector replacements) below
	 * are not counted in d_nsectors or d_secpercyl.
	 * Spare sectors are assumed to be physical sectors
	 * which occupy space at the end of each track and/or cylinder.
	 */
	uint16_t	d_sparespertrack;	/* # of spare sectors per track */
	uint16_t	d_sparespercyl;		/* # of spare sectors per cylinder */
	/*
	 * Alternate cylinders include maintenance, replacement,
	 * configuration description areas, etc.
	 */
	uint16_t	d_acylinders;		/* # of alt. cylinders per unit */

			/* hardware characteristics: */
	/*
	 * d_interleave, d_trackskew and d_cylskew describe perturbations
	 * in the media format used to compensate for a slow controller.
	 * Interleave is physical sector interleave, set up by the formatter
	 * or controller when formatting.  When interleaving is in use,
	 * logically adjacent sectors are not physically contiguous,
	 * but instead are separated by some number of sectors.
	 * It is specified as the ratio of physical sectors traversed
	 * per logical sector.  Thus an interleave of 1:1 implies contiguous
	 * layout, while 2:1 implies that logical sector 0 is separated
	 * by one sector from logical sector 1.
	 * d_trackskew is the offset of sector 0 on track N
	 * relative to sector 0 on track N-1 on the same cylinder.
	 * Finally, d_cylskew is the offset of sector 0 on cylinder N
	 * relative to sector 0 on cylinder N-1.
	 */
	uint16_t	d_rpm;			/* rotational speed */
	uint8_t	d_interleave;		/* hardware sector interleave */
	uint8_t	d_trackskew;		/* sector 0 skew, per track */
	uint8_t	d_cylskew;		/* sector 0 skew, per cylinder */
	uint8_t	d_headswitch;		/* head swith time, usec */
	uint16_t	d_trkseek;		/* track-to-track seek, msec */
	uint16_t	d_flags;		/* generic flags */
#define NDDATA 5
	uint32_t	d_drivedata[NDDATA];	/* drive-type specific information */
#define NSPARE 5
	uint32_t	d_spare[NSPARE];	/* reserved for future use */
	char	d_magic2[4];		/* the magic number (again) */
	uint16_t	d_checksum;		/* xor of data incl. partitions */

			/* filesystem and partition information: */
	uint16_t	d_npartitions;		/* number of partitions in following */
	uint16_t	d_bbsize;		/* size of boot area at sn0, bytes */
	uint16_t	d_sbsize;		/* max size of fs superblock, bytes */
	struct	partition {		/* the partition table */
		uint32_t	p_size;		/* number of sectors in partition */
		uint32_t	p_offset;	/* starting sector */
		uint16_t	p_fsize;	/* filesystem basic fragment size */
		uint8_t	p_fstype;	/* filesystem type, see below */
		uint8_t	p_frag;		/* filesystem fragments per block */
	} d_partitions[MAXPARTITIONS];	/* actually may be more */
}  __attribute__((packed));   /* Unclear if this will work  on architectures that require 32-bit aligned accesses. */

/* d_type values: */
#define	DTYPE_SMD		1		/* SMD, XSMD; VAX hp/up */
#define	DTYPE_MSCP		2		/* MSCP */
#define	DTYPE_DEC		3		/* other DEC (rk, rl) */
#define	DTYPE_SCSI		4		/* SCSI */
#define	DTYPE_ESDI		5		/* ESDI interface */
#define	DTYPE_ST506		6		/* ST506 etc. */
#define	DTYPE_FLOPPY		7		/* floppy */

#ifdef DKTYPENAMES
static const char *dktypenames[] = {
	"unknown",
	"SMD",
	"MSCP",
	"old DEC",
	"SCSI",
	"ESDI",
	"ST506",
	"floppy",
	0
};
#define DKMAXTYPES	(sizeof(dktypenames) / sizeof(dktypenames[0]) - 1)
#endif

/*
 * Filesystem type and version.
 * Used to interpret other filesystem-specific
 * per-partition information.
 */
#define	FS_UNUSED	0		/* unused */
#define	FS_SWAP		1		/* swap */
#define	FS_V6		2		/* Sixth Edition */
#define	FS_V7		3		/* Seventh Edition */
#define	FS_SYSV		4		/* System V */
/*
 * 2.11BSD uses type 5 filesystems even though block numbers are 4 bytes
 * (rather than the packed 3 byte format) and the directory structure is
 * that of 4.3BSD (long filenames).
*/
#define	FS_V71K		5		/* V7 with 1K blocks (4.1, 2.9, 2.11) */
#define	FS_V8		6		/* Eighth Edition, 4K blocks */
#define	FS_BSDFFS	7		/* 4.2BSD fast file system */
#define	FS_MSDOS	8		/* MSDOS file system */
#define	FS_BSDLFS	9		/* 4.4BSD log-structured file system */
#define	FS_OTHER	10		/* in use, but unknown/unsupported */
#define	FS_HPFS		11		/* OS/2 high-performance file system */
#define	FS_ISO9660	12		/* ISO 9660, normally CD-ROM */

#ifdef	DKTYPENAMES
static const char *fstypenames[] = {
	"unused",
	"swap",
	"Version 6",
	"Version 7",
	"System V",
	"2.11BSD",
	"Eighth Edition",
	"4.2BSD",
	"MSDOS",
	"4.4LFS",
	"unknown",
	"HPFS",
	"ISO9660",
	0
};
#define FSMAXTYPES	(sizeof(fstypenames) / sizeof(fstypenames[0]) - 1)
#endif

/*
 * flags shared by various drives:
 */
#define		D_REMOVABLE	0x01		/* removable media */
#define		D_ECC		0x02		/* supports ECC */
#define		D_BADSECT	0x04		/* supports bad sector forw. */
#define		D_RAMDISK	0x08		/* disk emulator */

/*
 * Structure used to perform a format
 * or other raw operation, returning data
 * and/or register values.
 * Register identification and format
 * are device- and driver-dependent.
 */
struct format_op {
	char	*df_buf;
	int	df_count;		/* value-result */
	daddr_t	df_startblk;
	int	df_reg[8];		/* result */
};

/*
 * Structure used internally to retrieve
 * information about a partition on a disk.
 */
struct partinfo {
	struct disklabel *disklab;
	struct partition *part;
};

/*
 * Disk-specific ioctls.
 */
		/* get and set disklabel; DIOCGPART used internally */
#define DIOCGDINFO	_IOR(d, 101, struct disklabel)	/* get */
#define DIOCSDINFO	_IOW(d, 102, struct disklabel)	/* set */
#define DIOCWDINFO	_IOW(d, 103, struct disklabel)	/* set, update disk */
#define DIOCGPART	_IOW(d, 104, struct partinfo)	/* get partition */

/* do format operation, read or write */
#define DIOCRFORMAT	_IOWR(d, 105, struct format_op)
#define DIOCWFORMAT	_IOWR(d, 106, struct format_op)

#define DIOCSSTEP	_IOW(d, 107, int)	/* set step rate */
#define DIOCSRETRIES	_IOW(d, 108, int)	/* set # of retries */
#define DIOCWLABEL	_IOW(d, 109, int)	/* write en/disable label */

#define DIOCSBAD	_IOW(d, 110, struct dkbad)	/* set kernel dkbad */

#ifndef	KERNEL
struct disklabel *getdiskbyname();
#endif

#if	defined(KERNEL) && !defined(SUPERVISOR)
memaddr	disklabelalloc();
#define	LABELDESC	(((btoc(sizeof (struct disklabel)) - 1) << 8) | RW)
#endif

#endif	/* !_SYS_DISKLABEL_H_ */
