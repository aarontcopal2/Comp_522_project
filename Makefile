#  we need gcc version that contains stdatomics library shipped internally. Tested with gcc version 8.3.0 (Spack GCC) [on llnl.cs]

CC=gcc
OPT=-g
CXX_VERSION=
LIBS=

all: hashtable_with_smr.c hashtable_with_smr.h
	$(CC) $(OPT) $(CXX_VERSION) $(LIBS) -o hashtable_with_smr hashtable_with_smr.c

clean:
	rm hashtable_with_smr