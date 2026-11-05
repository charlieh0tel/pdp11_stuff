"""The last build step: site configuration."""

from bsdbuild import config
from bsdbuild.console import Console

USER_NAME = "ch"
USER_HOME = f"/usr/{USER_NAME}"
HASH_SOURCE = "/tmp/hash.c"
HASH_PROGRAM = "/tmp/hash"


def user_entry(password_hash: str) -> str:
    """Returns the user's line in master.passwd format."""
    return (
        f"{USER_NAME}:{password_hash}:100:10::0:0:"
        f"Christopher Hoover,,,:{USER_HOME}:/bin/sh"
    )


def _add_user(console: Console) -> None:
    # FTP needs a password, and passwd(1) rejects one this short, so the guest
    # hashes it.  The login name is both the password and the 2-character salt.
    console.shell(
        f"""echo 'main(){{puts(crypt("{USER_NAME}","{USER_NAME}"));}}' """
        f"> {HASH_SOURCE}"
    )
    console.shell(f"cc -o {HASH_PROGRAM} {HASH_SOURCE}")
    # chpass updates master.passwd, passwd and the hashed database together.
    console.shell(f'chpass -a "{user_entry(f"`{HASH_PROGRAM}`")}"')
    console.shell(f"rm {HASH_PROGRAM} {HASH_SOURCE}")
    console.shell(f"mkdir {USER_HOME}")
    console.shell(f"chown {USER_NAME} {USER_HOME}")


def customize(console: Console) -> None:
    console.boot_to_multiuser()
    console.shell(f"hostname {config.HOSTNAME}")
    console.set_clock()

    console.shell("rm -f /*.core")
    console.shell("rm /disklabel")
    console.shell("rm /toyset")
    console.shell("rm /etc/dtab.save")

    console.copy_custom_file("rc", "/etc/rc", "root", "wheel", "0644")
    console.copy_custom_file("rc.local", "/etc/rc.local", "root", "wheel", "0755")

    for name in ("hosts", "networks", "netstart", "ntp.conf", "resolv.conf"):
        console.copy_custom_file(name, f"/etc/{name}", "root", "wheel", "0644")
    console.shell("mv /etc/inetd.conf /etc/inetd.conf.wrappers")
    console.shell("mv /etc/inetd.conf.nowrappers /etc/inetd.conf")
    console.write_file("/etc/hosts.equiv", b"", "root", "wheel", "0600")

    console.copy_custom_file("ttys", "/etc/ttys", "root", "wheel", "0644")

    _add_user(console)

    console.shell("sync")
    console.halt()
