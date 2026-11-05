"""The steps through the kernel; each starts at the simulator's first prompt."""

from bsdbuild import config
from bsdbuild.console import BOOT_PROMPT, LONG_TIMEOUT_SECONDS, Console

TOP_MENU = "d(isplay) D(efault) m(odify) w(rite) q(uit)? "
MODIFY_MENU = "d(isplay) g(eometry) m(isc) p(artitions) q(uit)? "
MISC_MENU = "d(isplay) t(ype) n(ame) l(abel) f(lags) r(pm) D(rivedata) q(uit)? "
PARTITIONS_MENU = "d(isplay) n(umber) s(elect) q(uit)? "
PARTITION_MENU = "d(isplay) z(ero) t(ype) o(ffset) s(ize) f(rag) F(size) q(uit)? "

TAPE_DISKLABEL = "ts(0,1)"
TAPE_MKFS = "ts(0,2)"
TAPE_RESTOR = "ts(0,3)"
TAPE_ROOT_DUMP = "ts(0,5)"
STANDALONE_DISK = "ra(0,0)"
DISK_KERNEL = "ra(0,0,0)unix"
TAPE_DEVICE = "/dev/rmt12"

# Tape file numbers, counted from the rewound tape.
TAPE_FILE_USR = 6
TAPE_FILE_USR_SRC = 8

MKFS_PROMPTS = 4


def _set_partition(console: Console, partition: config.Partition) -> None:
    console.respond(MODIFY_MENU, "p")
    console.respond(PARTITIONS_MENU, "s")
    console.send(partition.letter)
    console.respond(PARTITION_MENU, "o")
    console.answer(BOOT_PROMPT, str(partition.start))
    console.respond(PARTITION_MENU, "s")
    console.answer(BOOT_PROMPT, str(partition.size))
    console.respond(PARTITION_MENU, "t")
    console.answer(BOOT_PROMPT, partition.fstype)
    console.respond(PARTITION_MENU, "q")
    console.respond(PARTITIONS_MENU, "q")


def write_label(console: Console) -> None:
    console.boot("ts0", TAPE_DISKLABEL)
    console.answer("Disk? ", STANDALONE_DISK)
    console.respond("Proceed? [y/n] ", "y")
    for key in "dDd":
        console.respond(TOP_MENU, key)

    console.respond(TOP_MENU, "m")
    console.respond(MODIFY_MENU, "m")
    console.respond(MISC_MENU, "l")
    console.answer("label [DEFAULT]: ", config.LABEL)
    console.respond(MISC_MENU, "q")
    console.respond(MODIFY_MENU, "d")

    for partition in (config.ROOT, config.SWAP, config.WHOLE, config.USR):
        _set_partition(console, partition)

    console.respond(MODIFY_MENU, "q")
    for key in "wdq":
        console.respond(TOP_MENU, key)
    console.expect(BOOT_PROMPT)
    console.interrupt()


def restore_root(console: Console) -> None:
    console.boot("ts0", TAPE_MKFS)
    console.answer("file system: ", STANDALONE_DISK)
    for _ in range(MKFS_PROMPTS):
        console.answer(BOOT_PROMPT, "")
    console.expect("Exit called")

    console.answer(BOOT_PROMPT, TAPE_RESTOR)
    console.answer("Tape? ", TAPE_ROOT_DUMP)
    console.answer("Disk? ", STANDALONE_DISK)
    console.answer("Last chance before scribbling on disk.", "")
    console.expect(BOOT_PROMPT, LONG_TIMEOUT_SECONDS)
    console.interrupt()


def install_boot_block(console: Console) -> None:
    console.boot_to_single_user("ts0", DISK_KERNEL)
    console.shell(f"dd if=/mdec/rauboot of=/dev/r{config.ROOT.device} count=1")
    console.shell("sync")
    console.halt()


def _extract_tape_file(console: Console, file_number: int) -> None:
    console.shell("mt rew", timeout=LONG_TIMEOUT_SECONDS)
    console.shell(f"mt fsf {file_number}", timeout=LONG_TIMEOUT_SECONDS)
    console.shell(f"tar xpbf 20 {TAPE_DEVICE}", timeout=LONG_TIMEOUT_SECONDS)


def build_usr(console: Console) -> None:
    usr = config.USR.device
    console.boot_to_single_user("rq0")
    console.shell(f"hostname {config.HOSTNAME}")
    # Exits 2 because it cannot write wtmp yet.
    console.set_clock(check=False)

    console.shell(f"newfs {usr}", timeout=LONG_TIMEOUT_SECONDS)
    console.shell(f"mount /dev/{usr} /usr")

    console.shell("cd /dev")
    console.shell("rm *mt*")
    console.shell("./MAKEDEV ts0")
    console.shell("rm -f swap")
    console.shell(f"ln /dev/{config.SWAP.device} swap")

    console.shell("cd /usr")
    _extract_tape_file(console, TAPE_FILE_USR)

    # The kernel sources are the tape file after /usr.
    console.shell("mkdir src")
    console.shell("cd src")
    console.shell(f"mt -f {TAPE_DEVICE} fsf", timeout=LONG_TIMEOUT_SECONDS)
    console.shell(f"tar xpbf 20 {TAPE_DEVICE}", timeout=LONG_TIMEOUT_SECONDS)

    console.shell("cd /")
    console.shell("chmod 755 / /usr /usr/src /usr/src/sys")
    console.shell("rm -f sys")
    console.shell("ln -s /usr/src/sys sys")

    console.shell(f"umount /dev/{usr}")
    console.shell(
        f"fsck /dev/r{usr}",
        replies=[("ENTER FILENAME: ", "/tmp/usrfsck")],
        timeout=LONG_TIMEOUT_SECONDS,
    )
    console.shell(f"mount /dev/{usr} /usr")

    fstab = (
        f"/dev/{config.ROOT.device}\t\t/\tufs\trw\t1\t1\n"
        f"/dev/{config.SWAP.device}\t\tnone\tswap\tsw\t0\t0\n"
        f"/dev/{usr}\t\t/usr\tufs\trw\t1\t2\n"
    )
    console.write_file("/etc/fstab", fstab.encode("ascii"), "root", "wheel", "0644")
    console.shell("sync")
    console.halt()


def check_root(console: Console) -> None:
    console.boot_to_single_user("rq0")
    console.shell(f"hostname {config.HOSTNAME}")
    # Exits 2 because it cannot write wtmp yet.
    console.set_clock(check=False)
    # Exits nonzero when it finds anything to report.
    console.shell(f"fsck /dev/{config.ROOT.device}", check=False)
    # A sync would write the old in-memory superblock over fsck's repairs.
    console.halt(sync=False)


def extract_usr_src(console: Console) -> None:
    console.boot_to_multiuser()
    console.shell(f"hostname {config.HOSTNAME}")
    console.set_clock()
    console.shell("cd /usr/src")
    _extract_tape_file(console, TAPE_FILE_USR_SRC)
    console.shell("sync")
    console.halt()


def build_kernel(console: Console) -> None:
    console.boot_to_multiuser()
    console.shell(f"hostname {config.HOSTNAME}")
    console.set_clock()

    console.copy_custom_file(
        "MURGATROID", "/sys/conf/MURGATROID", "root", "wheel", "0644"
    )
    console.shell("cd /sys/conf")
    console.shell("./config MURGATROID")
    console.shell("cd ../MURGATROID")
    console.copy_custom_file("MURGATROID.Makefile", "Makefile", "root", "wheel", "0644")
    console.shell("make", timeout=LONG_TIMEOUT_SECONDS)

    # Keep the GENERIC kernel around.
    console.shell("cp -p /unix /genunix")
    console.shell("make install")

    console.copy_custom_file(
        "MAKEDEV.local", "/dev/MAKEDEV.local", "root", "wheel", "0555"
    )
    console.shell("cd /dev")
    for target in ("std", "pty", "local", "lp0"):
        console.shell(f"./MAKEDEV {target}")

    console.copy_custom_file("dtab", "/etc/dtab", "root", "wheel", "0644")
    console.shell("sync")
    console.halt()
