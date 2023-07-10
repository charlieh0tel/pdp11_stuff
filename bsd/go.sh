#!/bin/bash

set -o errexit
set -o xtrace

DISK=2.11BSD_patch457-root.ra92

if [ -e "${DISK}.post-03-snap" ]; then
    cp -p "${DISK}.post-03-snap" "${DISK}"
else
    ./00_build_disk.expect
    ./01_build_root.expect
    ./02_bootstrap_disk.expect
    ./03_build_usr.expect
    cp -p "${DISK}" "${DISK}.post-03-snap"
fi
./04_customize.expect
