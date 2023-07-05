#!/bin/bash

set -o errexit
set -o xtrace

rm -f 2.11BSD_patch457-root.ra92

./00_build_disk.expect
./01_build_root.expect
./02_bootstrap_disk.expect
cp -p 2.11BSD_patch457-root.ra92  temp_bootable.ra92
