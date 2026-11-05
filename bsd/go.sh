#!/bin/bash
#
# Builds the disk image, reusing the snapshot taken after the kernel step.

set -o errexit
set -o nounset
set -o xtrace

readonly PATCHLEVEL=457
readonly TAPE="2.11BSD_patch${PATCHLEVEL}.tape0"
readonly DISK="2.11BSD_patch${PATCHLEVEL}-root.ra92"
readonly SNAPSHOT="${DISK}.post-06-snap"
readonly LAST_SNAPSHOT_STEP=6
readonly LAST_STEP=7

run_step() {
  uv run python -m bsdbuild "$1" "${TAPE}" "${DISK}"
}

main() {
  local step
  if [[ -e "${SNAPSHOT}" ]]; then
    cp --preserve=timestamps --sparse=always "${SNAPSHOT}" "${DISK}"
  else
    rm -f "${DISK}"
    for ((step = 0; step <= LAST_SNAPSHOT_STEP; step++)); do
      run_step "${step}"
    done
    cp --preserve=timestamps --sparse=always "${DISK}" "${SNAPSHOT}"
  fi
  run_step "${LAST_STEP}"
}

main "$@"
