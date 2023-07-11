#!/bin/bash

set -o errexit
set -o xtrace

DISK="2.11BSD_patch457-root.ra92"

if [ -e "${DISK}.post-05-snap" ]; then
    cp -p "${DISK}.post-05-snap" "${DISK}"
else
    rm -f "${DISK}"
    ./00_build_disk.expect
    ./01_build_root.expect
    ./02_bootstrap_disk.expect
    ./03_build_usr.expect
    ./04_usr_src.expect
    ./05_kernel.expect
   cp -p "${DISK}" "${DISK}.post-05-snap"
fi
./06_customize.expect
