#!/bin/sh

URL_BASE="https://www.tuhs.org/Archive/Distributions/UCB/"
DIR="2.11BSD_patch457"
OUT="${DIR}.tape0"

curl "${URL_BASE}/${DIR}/tape0.bz2" | bunzip2 > "${OUT}"
