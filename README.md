**ENTRY TEST NOTES**
----

1) Implementation parser for EDID is incomplete.
   Single EDID file contains Additional Timing Data Block.
   But it is no format description on WIKI.
   The problem can be solved on additional time.
2) Used functional testing can be progressed.
   But now only 1 test was implemented.

BUILD and RUN test

git clone https://github.com/vzhirov/elvis_entry_test.git
cd elvis_entry_test
autoreconf --install
./configure --prefix `pwd`/out
make
make check

USAGE

make install
out/bin/elvis_vzhirov_bin tests/acer-xf290c.edid


EXTERNAL INFO:

* https://en.wikipedia.org/wiki/Extended_Display_Identification_Data
  Wiki - Extended Display Identification Data

