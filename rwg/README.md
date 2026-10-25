# RWG — Reticon Width Gauge

These EEPROM images are from an MRV11-C/M8048 card I purchased off of
eBay.  The EEPROMs were labeled RWG.  It appears to have something to
do with cameras.

The strings in the ROM identify it as a **RETICON WIDTH GAUGE**: a
non-contact gauge that measures the width of hot metal (probably strip
or plate in a steel or aluminum mill) using two Reticon line-scan
photodiode array cameras.

## What it does (speculation)

- Two cameras, each probably viewing one edge of the hot product.
  Each reports the diode position of the edge (in hex counts).
- Width is computed from the two edge positions, a per-camera
  resolution (counts per 10 inches), and an offset.
- Several readings are averaged per displayed width; readings outside
  the configured maximum/minimum widths are rejected.
- Calibration uses marks a known distance apart, checked against
  upper/lower tolerances.
- Optional hot-to-cold width conversion using a shrinkage factor and
  an average temperature (degrees F).
- Output goes to a remote display unit at a configurable update rate
  (ms), and probably to parallel ports ("Port_C", "Port_D") in a
  selectable format ("Binry2/3/4").
- Operator interface is a VT100 menu-driven calibration program.
  Errors say "call CPC", possibly the vendor/integrator.

### Calibration menu

```
RETICON WIDTH GAUGE
CALIBRATION PARAMETERS - SETUP/DISPLAY

 0  EXIT THE PROGRAM
 1  Display All Current Parameter Values
 2  Display Raw Camera Counts
 3  Change diode counts per camera
 4  Change offset
 5  Change resolution
 6  Change calibration mark tolerance
 7  Change readings per displayed width
 8  Change maximum/minimum widths
 9  Change remote display unit update rate
10  Select/deselect hot to cold width conversion
11  Change width shrinkage factor
12  Change average temperature for shrinkage
13  Change width output format
```

## Hardware notes

- Interrupt vectors occupy 0–0776; code starts at 01000.  Every vector
  sets the CPU priority to 7.  Unused vectors all go to one catch-all
  handler.
- Vectors with real handlers: 100 (line clock), 244 (FP exception),
  250 (MMU fault), 264, 300/304 (probably a second serial line), and
  a block of 16 at 400–474 (likely the camera or parallel I/O
  interface).
- Frequent references to 172344/172346 (KISAR2/KISAR3) suggest an
  LSI-11/23 with memory management mapping ROM pages in and out.
- Strings like "TRUE"/"FALSE", "$TTA" and real-number parsing suggest
  a high-level language runtime (Fortran or Pascal?) rather than pure
  assembly.
