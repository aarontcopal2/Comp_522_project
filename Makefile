#  we need gcc version that contains stdatomics library shipped internally. Tested with gcc version 8.3.0 (Spack GCC) [on llnl.cs]

# hardcoding the gcc path for now, to show that gcc version 8.3.1 20191121 (Red Hat 8.3.1-5) (GCC) doesnt work
CC=gcc
# CC=/home/atc8/software/spack/opt/spack/linux-centos8-skylake/gcc-8.3.1/gcc-8.3.0-vodp4hkmzrwqjb5m45xsb7syhgwzgv57/bin/gcc
OPT=-O2 -g
CXX_VERSION=-std=gnu99
LIBS=-latomic -lpthread
BUILD_DIR=hashtable/build
# BUILD_OBJS = $(wildcard $(BUILD_DIR)/*.o) make needs to run twice for this to work
HASHTABLE_BUILD_OBJS=hashtable/build/spinlock.o hashtable/build/gpu-splay-allocator.o hashtable/build/stacks.o hashtable/build/bistack.o hashtable/build/bichannel.o hashtable/build/gpu-channel-item-allocator.o hashtable/build/hashtable-memory-manager.o hashtable/build/splay-uint64.o hashtable/build/Micheal-splay-tree.o hashtable/build/Micheal-Lock-Free-List.o hashtable/build/Split-Ordered-Lists.o
BENCHMARK_BUILD_OBJS=benchmark/build/benchmark.o benchmark/build/test-splay-tree.o benchmark/build/splay-tree-benchmark.o benchmark/build/Split-Ordered-Lists-benchmark.o
HELGRIND_PARAMS=--free-is-write=yes --history-level=full --show-error-list=yes --suppressions=$(shell pwd)/helgrind_suppression.supp
MEMCHECK_PARAMS=--leak-check=full --track-origins=yes


all:
	$(MAKE) -C "hashtable" all
	$(MAKE) -C "benchmark" all
	$(CC) $(OPT) $(CXX_VERSION) $(LIBS) -o main main.c $(HASHTABLE_BUILD_OBJS) $(BENCHMARK_BUILD_OBJS)


helgrind: all
	valgrind --tool=helgrind $(HELGRIND_PARAMS) ./main > helgrind_output 2>&1


memcheck: all
	valgrind --tool=memcheck $(MEMCHECK_PARAMS) ./main > memcheck_output 2>&1


clean:
	$(MAKE) -C "hashtable" clean
	$(MAKE) -C "benchmark" clean
	rm main
