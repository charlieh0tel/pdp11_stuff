# 2.11BSD disk image

Builds a customized 2.11BSD RA92 disk image from the distribution tape
by driving the SIMH console.

Requires `make`, `uv`, `pdp11` (SIMH), `curl`, `bunzip2`.  `prlabel` also
requires a C++17 compiler and Boost.

## Targets

| Target | Result |
|---|---|
| `make` | Fetches the tape if needed and builds the disk image |
| `make run` | Boots the disk image in SIMH |
| `make check` | Runs ruff, mypy, pytest and shellcheck |
| `make prlabel` | Builds `prlabel`, which prints a disk image's label |
| `make clean` | Removes `prlabel` |
| `make clean-images` | Removes the disk and stage images |

## Build steps

Each step produces a stage image from the previous one.  Make reruns a
step when its image is older than its prerequisites.

| Stage | Step | Code | Files in `custom/` |
|---|---|---|---|
| 0 | Writes the disk label | `steps.py` | |
| 1 | Restores root from tape | `steps.py` | |
| 2 | Installs the boot block | `steps.py` | |
| 3 | Creates `/usr` and extracts it and the kernel source | `steps.py` | |
| 4 | Checks the root filesystem | `steps.py` | |
| 5 | Extracts `/usr/src` | `steps.py` | |
| 6 | Builds the `MURGATROID` kernel and device nodes | `steps.py` | `MURGATROID`, `MURGATROID.Makefile`, `MAKEDEV.local`, `dtab` |
| 7 | Applies the site configuration | `customize.py` | `rc`, `rc.local`, `hosts`, `networks`, `netstart`, `ntp.conf`, `resolv.conf`, `ttys` |

`custom/orig/` holds the distribution's versions of the files in `custom/`.

## Source

Tape: <https://www.tuhs.org/Archive/Distributions/UCB/2BSD/2.11BSD_patch457/>
