#!/bin/bash

set -o errexit
set -o xtrace

DISK="2.11BSD_patch457-root.ra92"

if [ -e "${DISK}.post-06-snap" ]; then
    cp -p "${DISK}.post-06-snap" "${DISK}"
else
    rm -f "${DISK}"
    ./00_build_disk.expect
    ./01_build_root.expect
    ./02_bootstrap_disk.expect
    ./03_build_usr.expect
    ./04_fsck_root.expect
    ./05_usr_src.expect
    ./06_kernel.expect
    cp -p "${DISK}" "${DISK}.post-06-snap"
fi
./07_customize.expect
