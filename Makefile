#  we need gcc version that contains stdatomics library shipped internally. Tested with gcc version 8.3.0 (Spack GCC) [on llnl.cs]

# hardcoding the gcc path for now, to show that gcc version 8.3.1 20191121 (Red Hat 8.3.1-5) (GCC) doesnt work
CC=/home/atc8/software/spack/opt/spack/linux-centos8-skylake/gcc-8.3.1/gcc-8.3.0-vodp4hkmzrwqjb5m45xsb7syhgwzgv57/bin/gcc
OPT=-g
CXX_VERSION=
LIBS=-latomic -lpthread -fopenmp
BUILD_DIR=hashtable/build

all:
	$(MAKE) -C "hashtable" all
	$(CC) $(OPT) $(CXX_VERSION) $(LIBS) -o main main.c $(BUILD_DIR)/Split-Ordered-Lists.o $(BUILD_DIR)/Micheal-Lock-Free-List.o

clean:
	$(MAKE) -C "hashtable" clean
	rm main