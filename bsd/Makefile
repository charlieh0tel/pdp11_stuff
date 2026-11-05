# Builds a customized 2.11BSD disk image.  Each build step produces its own
# stage image from the previous one, so a change reruns only what it affects.

DIST := 2.11BSD_patch457
TAPE := $(DIST).tape0
DISK := $(DIST)-root.ra92

stage = $(DIST).stage$(1).ra92

BUILD_SOURCES := simh.ini bsdbuild/__main__.py bsdbuild/config.py \
	bsdbuild/console.py
BASE_SOURCES := $(BUILD_SOURCES) bsdbuild/steps.py
KERNEL_FILES := $(addprefix custom/,MURGATROID MURGATROID.Makefile \
	MAKEDEV.local dtab)
SITE_FILES := $(addprefix custom/,rc rc.local hosts networks netstart \
	ntp.conf resolv.conf ttys)

.PHONY: all run check clean clean-images

all: $(DISK)

$(TAPE):
	./fetch.sh

$(call stage,0): $(BASE_SOURCES)
$(call stage,1): $(call stage,0) $(BASE_SOURCES)
$(call stage,2): $(call stage,1) $(BASE_SOURCES)
$(call stage,3): $(call stage,2) $(BASE_SOURCES)
$(call stage,4): $(call stage,3) $(BASE_SOURCES)
$(call stage,5): $(call stage,4) $(BASE_SOURCES)
$(call stage,6): $(call stage,5) $(BASE_SOURCES) $(KERNEL_FILES)
$(call stage,7): $(call stage,6) $(BUILD_SOURCES) bsdbuild/customize.py \
	$(SITE_FILES)

$(call stage,%): | $(TAPE)
	rm -f $@.tmp
	$(if $(filter %.ra92,$^),cp --sparse=always $(filter %.ra92,$^) $@.tmp)
	uv run python -m bsdbuild $* $(TAPE) $@.tmp
	mv $@.tmp $@

$(DISK): $(call stage,7)
	cp --sparse=always $< $@

run: $(DISK)
	pdp11 simh.ini $(TAPE) $(DISK)

prlabel: prlabel.cc disklabel.h
	c++ -std=c++17 -Wall -Wpedantic -o prlabel prlabel.cc

check:
	uv run ruff format --check
	uv run ruff check
	uv run mypy
	uv run pytest
	shellcheck fetch.sh

clean:
	rm -f prlabel

clean-images:
	rm -f $(DISK) $(DIST).stage*.ra92 $(DIST).stage*.ra92.tmp
