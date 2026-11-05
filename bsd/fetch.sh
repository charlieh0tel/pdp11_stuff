#!/bin/bash
#
# Downloads the 2.11BSD distribution tape unless it is already present.

set -o errexit
set -o nounset
set -o pipefail

readonly URL_BASE="https://www.tuhs.org/Archive/Distributions/UCB/2BSD"
readonly DIR="2.11BSD_patch457"
readonly OUT="${DIR}.tape0"
readonly TMP="${OUT}.tmp"

main() {
  if [[ -s "${OUT}" ]]; then
    exit 0
  fi

  trap 'rm -f "${TMP}"' EXIT

  curl --fail --location --silent --show-error \
    "${URL_BASE}/${DIR}/tape0.bz2" | bunzip2 > "${TMP}"
  mv "${TMP}" "${OUT}"
}

main "$@"
