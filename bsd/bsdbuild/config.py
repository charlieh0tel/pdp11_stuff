"""Site settings and the layout of the RA92 disk."""

from dataclasses import dataclass

LABEL = "MURGATROID"
HOSTNAME = "pdp1153.murgatroid.com"

CYLINDERS = 3099
SECTORS_PER_CYLINDER = 73 * 13
SECTOR_BYTES = 512
DISK_SECTORS = CYLINDERS * SECTORS_PER_CYLINDER
DISK_BYTES = DISK_SECTORS * SECTOR_BYTES

ROOT_DISK = "ra0"
ROOT_CYLINDERS = 12
SWAP_CYLINDERS = 9


@dataclass(frozen=True)
class Partition:
    """One disk label entry; start and size are in sectors."""

    letter: str
    start: int
    size: int
    fstype: str

    @property
    def device(self) -> str:
        return f"{ROOT_DISK}{self.letter}"


_root_sectors = ROOT_CYLINDERS * SECTORS_PER_CYLINDER
_swap_sectors = SWAP_CYLINDERS * SECTORS_PER_CYLINDER
_usr_start = _root_sectors + _swap_sectors

ROOT = Partition("a", 0, _root_sectors, "2.11BSD")
SWAP = Partition("b", _root_sectors, _swap_sectors, "swap")
WHOLE = Partition("c", 0, DISK_SECTORS, "unused")
USR = Partition("d", _usr_start, DISK_SECTORS - _usr_start, "2.11BSD")
