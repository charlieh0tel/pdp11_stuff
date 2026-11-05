"""Runs one build step: python -m bsdbuild STEP TAPE DISK."""

import argparse
import os
import sys
from pathlib import Path

from bsdbuild import config, steps
from bsdbuild.console import ConsoleError, simulator
from bsdbuild.customize import customize

# Build steps in order; a step's number is its index.
STEPS = (
    steps.write_label,
    steps.restore_root,
    steps.install_boot_block,
    steps.build_usr,
    steps.check_root,
    steps.extract_usr_src,
    steps.build_kernel,
    customize,
)
FIRST_STEP = 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("step", type=int, choices=range(len(STEPS)))
    parser.add_argument("tape", type=Path)
    parser.add_argument("disk", type=Path)
    arguments = parser.parse_args()

    if arguments.step == FIRST_STEP:
        arguments.disk.touch()
        os.truncate(arguments.disk, config.DISK_BYTES)

    try:
        with simulator(arguments.tape, arguments.disk) as console:
            STEPS[arguments.step](console)
            console.quit()
    except ConsoleError as error:
        print(f"step {arguments.step} failed: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
