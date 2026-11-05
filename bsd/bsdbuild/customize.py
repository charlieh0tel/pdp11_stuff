"""The last build step: site configuration."""

from bsdbuild import config
from bsdbuild.console import Console


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

    console.shell("sync")
    console.halt()
