#  we need gcc version that contains stdatomics library shipped internally. Tested with gcc version 8.3.0 (Spack GCC) [on llnl.cs]

# hardcoding the gcc path for now, to show that gcc version 8.3.1 20191121 (Red Hat 8.3.1-5) (GCC) doesnt work
CC=/home/atc8/software/spack/opt/spack/linux-centos8-skylake/gcc-8.3.1/gcc-8.3.0-vodp4hkmzrwqjb5m45xsb7syhgwzgv57/bin/gcc
OPT=-O0 -g
CXX_VERSION=
LIBS=-latomic -lpthread
MKDIR_P = mkdir -p

all: benchmark.c splay-tree/splay-tree-benchmark.c Split-Ordered-Lists/Split-Ordered-Lists-benchmark.c
	$(MKDIR_P) build
	$(CC) $(OPT) $(CXX_VERSION) $(LIBS) -o build/benchmark.o benchmark.c -c
	$(CC) $(OPT) $(CXX_VERSION) $(LIBS) -o build/splay-tree-benchmark.o splay-tree/splay-tree-benchmark.c -c
	$(CC) $(OPT) $(CXX_VERSION) $(LIBS) -o build/Split-Ordered-Lists-benchmark.o Split-Ordered-Lists/Split-Ordered-Lists-benchmark.c -c
	

clean:
	rm -r build/