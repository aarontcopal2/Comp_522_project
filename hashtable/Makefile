#  we need gcc version that contains stdatomics library shipped internally. Tested with gcc version 8.3.0 (Spack GCC) [on llnl.cs]

# hardcoding the gcc path for now, to show that gcc version 8.3.1 20191121 (Red Hat 8.3.1-5) (GCC) doesnt work
CC=/home/atc8/software/spack/opt/spack/linux-centos8-skylake/gcc-8.3.1/gcc-8.3.0-vodp4hkmzrwqjb5m45xsb7syhgwzgv57/bin/gcc
OPT=-O2 -g
CXX_VERSION=
LIBS=-latomic -lpthread
MKDIR_P = mkdir -p

all: memory-manager splay-tree/splay-uint64.c Micheal-splay-tree.c Micheal-Lock-Free-List.c Split-Ordered-Lists.c
	$(MKDIR_P) build
	$(CC) $(OPT) $(CXX_VERSION) $(LIBS) -o build/splay-uint64.o splay-tree/splay-uint64.c -c
	$(CC) $(OPT) $(CXX_VERSION) $(LIBS) -o build/Micheal-splay-tree.o Micheal-splay-tree.c -c
	$(CC) $(OPT) $(CXX_VERSION) $(LIBS) -o build/Micheal-Lock-Free-List.o Micheal-Lock-Free-List.c -c
	$(CC) $(OPT) $(CXX_VERSION) $(LIBS) -o build/Split-Ordered-Lists.o Split-Ordered-Lists.c -c


memory-manager:	channel/hashtable-memory-manager.c
	$(MKDIR_P) build
	$(CC) $(OPT) $(CXX_VERSION) $(LIBS) -o build/spinlock.o channel/lib/prof-lean/spinlock.c -c
	$(CC) $(OPT) $(CXX_VERSION) $(LIBS) -o build/gpu-splay-allocator.o channel/hpcrun/gpu/gpu-splay-allocator.c -c
	$(CC) $(OPT) $(CXX_VERSION) $(LIBS) -o build/stacks.o channel/lib/prof-lean/stacks.c -c
	$(CC) $(OPT) $(CXX_VERSION) $(LIBS) -o build/bistack.o channel/lib/prof-lean/bistack.c -c
	$(CC) $(OPT) $(CXX_VERSION) $(LIBS) -o build/bichannel.o channel/lib/prof-lean/bichannel.c -c
	$(CC) $(OPT) $(CXX_VERSION) $(LIBS) -o build/gpu-channel-item-allocator.o channel/hpcrun/gpu/gpu-channel-item-allocator.c -c
	$(CC) $(OPT) $(CXX_VERSION) $(LIBS) -o build/hashtable-memory-manager.o channel/hashtable-memory-manager.c -c
	
	

clean:
	rm -r build/