#!/bin/bash

set -o errexit
set -o nounset
set -o pipefail

URL_BASE="https://www.tuhs.org/Archive/Distributions/UCB/2BSD"
DIR="2.11BSD_patch457"
OUT="${DIR}.tape0"
TMP="${OUT}.tmp"

if [ -s "${OUT}" ]; then
    exit 0
fi

trap 'rm -f "${TMP}"' EXIT

curl --fail --location --silent --show-error "${URL_BASE}/${DIR}/tape0.bz2" | bunzip2 > "${TMP}"
mv "${TMP}" "${OUT}"
