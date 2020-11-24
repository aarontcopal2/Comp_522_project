/*
 * File:
 *   test.c
 * Author(s):
 *   Vincent Gramoli <vincent.gramoli@epfl.ch>
 * Description:
 *   Concurrent accesses of a hashtable
 *
 * Copyright (c) 2009-2010.
 *
 * test.c is part of Synchrobench
 * 
 * Synchrobench is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

// #include "intset.h"

#include <stdio.h>
#include <pthread.h>
#include <stdint.h> //intptr_t
#include <stdlib.h> //RAND_MAX
#include <getopt.h> //no_argument, required_argument
#include <assert.h>
#include <limits.h> //INT_MIN, INT_MAX
#include <sys/time.h> //gettimeofday
#include <signal.h> //sigemptyset, sigsuspend
// #include "generalize.h" //AO_store_full

typedef intptr_t val_t;

typedef struct node {
	val_t val;
	struct node *next;
} node_t;

typedef struct intset {
	node_t *head;
} intset_t;

typedef struct ht_intset {
  intset_t **buckets;
} ht_intset_t;

#define DEFAULT_DURATION                10000
#define DEFAULT_INITIAL                 256
#define DEFAULT_NB_THREADS              1
#define DEFAULT_RANGE                   0x7FFFFFFF
#define DEFAULT_SEED                    0
#define DEFAULT_UPDATE                  20
#define DEFAULT_MOVE                    0
#define DEFAULT_SNAPSHOT                0
#define DEFAULT_LOAD                    1
#define DEFAULT_ELASTICITY							4
#define DEFAULT_ALTERNATE								0
#define DEFAULT_EFFECTIVE								1
#define MAXHTLENGTH                     65536

#define VAL_MIN                         INT_MIN
#define VAL_MAX                         INT_MAX

#define XSTR(s)                         STR(s)
#define STR(s)                          #s

#define AO_t size_t
static volatile AO_t stop;

#  define TM_THREAD_ENTER()              /* nothing */
#  define TM_THREAD_EXIT()               /* nothing */
#  define TM_STARTUP()                   /* nothing */
#  define TM_SHUTDOWN()                  /* nothing */

//#define MALLOC(size)                    stm_malloc(size)
#define MALLOC(size)                    malloc(size)

/* Hashtable length (# of buckets) */
unsigned int maxhtlength;

/* Hashtable seed */
#ifdef TLS
__thread unsigned int *rng_seed;
#else /* ! TLS */
pthread_key_t rng_seed_key;
#endif /* ! TLS */

typedef struct barrier {
	pthread_cond_t complete;
	pthread_mutex_t mutex;
	int count;
	int crossing;
} barrier_t;


void barrier_init(barrier_t *b, int n)
{
	pthread_cond_init(&b->complete, NULL);
	pthread_mutex_init(&b->mutex, NULL);
	b->count = n;
	b->crossing = 0;
}

void barrier_cross(barrier_t *b)
{
	pthread_mutex_lock(&b->mutex);
	/* One more thread through */
	b->crossing++;
	/* If not all here, wait */
	if (b->crossing < b->count) {
		pthread_cond_wait(&b->complete, &b->mutex);
	} else {
		pthread_cond_broadcast(&b->complete);
		/* Reset for next time */
		b->crossing = 0;
	}
	pthread_mutex_unlock(&b->mutex);
}

/* 
 * Returns a pseudo-random value in [1;range).
 * Depending on the symbolic constant RAND_MAX>=32767 defined in stdlib.h,
 * the granularity of rand() could be lower-bounded by the 32767^th which might 
 * be too high for given values of range and initial.
 *
 * Note: this is not thread-safe and will introduce futex locks
 */
inline long rand_range(long r) {
	int m = RAND_MAX;
	long d, v = 0;
	
	do {
		d = (m > r ? r : m);
		v += 1 + (long)(d * ((double)rand()/((double)(m)+1.0)));
		r -= m;
	} while (r > 0);
	return v;
}
long rand_range(long r);

/* Thread-safe, re-entrant version of rand_range(r) */
inline long rand_range_re(unsigned int *seed, long r) {
	int m = RAND_MAX;
	long d, v = 0;
	
	do {
		d = (m > r ? r : m);		
		v += 1 + (long)(d * ((double)rand_r(seed)/((double)(m)+1.0)));
		r -= m;
	} while (r > 0);
	return v;
}
long rand_range_re(unsigned int *seed, long r);

typedef struct thread_data {
  val_t first;
	long range;
	int update;
	int move;
	int snapshot;
	int unit_tx;
	int alternate;
	int effective;
	unsigned long nb_add;
	unsigned long nb_added;
	unsigned long nb_remove;
	unsigned long nb_removed;
	unsigned long nb_contains;
	/* added for HashTables */
	unsigned long load_factor;
	unsigned long nb_move;
	unsigned long nb_moved;
	unsigned long nb_snapshot;
	unsigned long nb_snapshoted;
	/* end: added for HashTables */
	unsigned long nb_found;
	unsigned long nb_aborts;
	unsigned long nb_aborts_locked_read;
	unsigned long nb_aborts_locked_write;
	unsigned long nb_aborts_validate_read;
	unsigned long nb_aborts_validate_write;
	unsigned long nb_aborts_validate_commit;
	unsigned long nb_aborts_invalid_memory;
	unsigned long nb_aborts_double_write;
	unsigned long max_retries;
	unsigned int seed;
	ht_intset_t *set;
	barrier_t *barrier;
	unsigned long failures_because_contention;
} thread_data_t;


node_t *new_node(val_t val, node_t *next, int transactional)
{
  node_t *node;

  if (transactional) {
	node = (node_t *)MALLOC(sizeof(node_t));
  } else {
	node = (node_t *)malloc(sizeof(node_t));
  }
  if (node == NULL) {
	perror("malloc");
	exit(1);
  }

  node->val = val;
  node->next = next;

  return node;
}


intset_t *set_new()
{
  intset_t *set;
  node_t *min, *max;
	
  if ((set = (intset_t *)malloc(sizeof(intset_t))) == NULL) {
    perror("malloc");
    exit(1);
  }
  max = new_node(VAL_MAX, NULL, 0);
  min = new_node(VAL_MIN, max, 0);
  set->head = min;

  return set;
}


inline int set_seq_add(intset_t *set, val_t val)
{
	int result;
	node_t *prev, *next;
	
	prev = set->head;
	next = prev->next;
	while (next->val < val) {
		prev = next;
		next = prev->next;
	}
	result = (next->val != val);
	if (result) {
		prev->next = new_node(val, next, 0);
	}
	return result;
}


int set_add(intset_t *set, val_t val, int transactional)
{
	int result;
	
#ifdef DEBUG
	printf("++> set_add(%d)\n", (int)val);
	IO_FLUSH;
#endif

	if (!transactional) {
		
		result = set_seq_add(set, val);
		
	} else { 
	
#ifdef SEQUENTIAL /* Unprotected */
		
		result = set_seq_add(set, val);
		
#elif defined STM
	
		node_t *prev, *next;
		val_t v;	
	
		if (transactional > 2) {

		  TX_START(EL);
		  prev = set->head;
		  next = (node_t *)TX_LOAD(&prev->next);
		  while (1) {
		    v = TX_LOAD((uintptr_t *) &next->val);
		    if (v >= val)
		      break;
		    prev = next;
		    next = (node_t *)TX_LOAD(&prev->next);
		  }
		  result = (v != val);
		  if (result) {
		    TX_STORE(&prev->next, new_node(val, next, transactional));
		  }
		  TX_END;
		  
		} else {

		  TX_START(NL);
		  prev = set->head;
		  next = (node_t *)TX_LOAD(&prev->next);
		  while (1) {
		    v = TX_LOAD((uintptr_t *) &next->val);
		    if (v >= val)
		      break;
		    prev = next;
		    next = (node_t *)TX_LOAD(&prev->next);
		  }
		  result = (v != val);
		  if (result) {
		    TX_STORE(&prev->next, new_node(val, next, transactional));
		  }
		  TX_END;

		}

#elif defined LOCKFREE
		result = harris_insert(set, val);
#endif
		
	}
	
	return result;
}


ht_intset_t *ht_new() {
	ht_intset_t *set;
	int i;
	
	if ((set = (ht_intset_t *)malloc(sizeof(ht_intset_t))) == NULL) {
		perror("malloc");
		exit(1);
	}  
	for (i=0; i < maxhtlength; i++) {
		set->buckets[i] = set_new();
	}
	return set;
}


int ht_add(ht_intset_t *set, int val, int transactional) {
	int addr;
	
	addr = val % maxhtlength;
	if (transactional == 5)
		return set_add(set->buckets[addr], val, 4);
	else 
		return set_add(set->buckets[addr], val, transactional);
}


void ht_delete(ht_intset_t *set) {
	node_t *node, *next;
	int i;
	
	for (i=0; i < maxhtlength; i++) {
		node = set->buckets[i]->head;
		while (node != NULL) {
			next = node->next;
			free(node);
			node = next;
		}
	}
	free(set);
}


int ht_size(ht_intset_t *set) {
	int size = 0;
	node_t *node;
	int i;
	
	for (i=0; i < maxhtlength; i++) {
		//size += set_size(set->buckets[i]);
		
		node = set->buckets[i]->head->next;
		while (node->next) {
			size++;
			node = node->next;
		}
		
	}
	return size;
}


void *test(void *data) {
	int val2, numtx, r, last = -1;
	val_t val = 0;
	int unext, mnext, cnext;
	
	thread_data_t *d = (thread_data_t *)data;
	
	/* Create transaction */
	TM_THREAD_ENTER();
	/* Wait on barrier */
	barrier_cross(d->barrier);

	/* Free transaction */
	TM_THREAD_EXIT();
	
	return NULL;
}


int main(int argc, char **argv)
{
	struct option long_options[] = {
		// These options don't set a flag
		{"help",                      no_argument,       NULL, 'h'},
		{"duration",                  required_argument, NULL, 'd'},
		{"initial-size",              required_argument, NULL, 'i'},
		{"thread-num",                required_argument, NULL, 't'},
		{"range",                     required_argument, NULL, 'r'},
		{"seed",                      required_argument, NULL, 'S'},
		{"update-rate",               required_argument, NULL, 'u'},
		{"move-rate",                 required_argument, NULL, 'a'},
		{"snapshot-rate",             required_argument, NULL, 's'},
		{"elasticity",                required_argument, NULL, 'x'},
		{NULL, 0, NULL, 0}
	};
	
	ht_intset_t *set;
	int i, c, size;
	val_t last = 0; 
	val_t val = 0;
	unsigned long reads, effreads, updates, effupds, moves, moved, snapshots, 
	snapshoted, aborts, aborts_locked_read, aborts_locked_write, 
	aborts_validate_read, aborts_validate_write, aborts_validate_commit, 
	aborts_invalid_memory, aborts_double_write,
	max_retries, failures_because_contention;
	thread_data_t *data;
	pthread_t *threads;
	pthread_attr_t attr;
	barrier_t barrier;
	struct timeval start, end;
	struct timespec timeout;
	int duration = DEFAULT_DURATION;
	int initial = DEFAULT_INITIAL;
	int nb_threads = DEFAULT_NB_THREADS;
	long range = DEFAULT_RANGE;
	int seed = DEFAULT_SEED;
	int update = DEFAULT_UPDATE;
	int load_factor = DEFAULT_LOAD;
	int move = DEFAULT_MOVE;
	int snapshot = DEFAULT_SNAPSHOT;
	int unit_tx = DEFAULT_ELASTICITY;
	int alternate = DEFAULT_ALTERNATE;
	int effective = DEFAULT_EFFECTIVE;
	sigset_t block_set;
	
	while(1) {
		i = 0;
		c = getopt_long(argc, argv, "hAf:d:i:t:r:S:u:a:s:l:x:", long_options, &i);
		
		if(c == -1)
			break;
		
		if(c == 0 && long_options[i].flag == 0)
			c = long_options[i].val;
		
		switch(c) {
				case 0:
					// Flag is automatically set 
					break;
				case 'h':
					printf("intset -- STM stress test "
								 "(hash table)\n"
								 "\n"
								 "Usage:\n"
								 "  intset [options...]\n"
								 "\n"
								 "Options:\n"
								 "  -h, --help\n"
								 "        Print this message\n"
								 "  -A, --Alternate\n"
								 "        Consecutive insert/remove target the same value\n"
								 "  -f, --effective <int>\n"
								 "        update txs must effectively write (0=trial, 1=effective, default=" XSTR(DEFAULT_EFFECTIVE) ")\n"
								 "  -d, --duration <int>\n"
								 "        Test duration in milliseconds (0=infinite, default=" XSTR(DEFAULT_DURATION) ")\n"
								 "  -i, --initial-size <int>\n"
								 "        Number of elements to insert before test (default=" XSTR(DEFAULT_INITIAL) ")\n"
								 "  -t, --thread-num <int>\n"
								 "        Number of threads (default=" XSTR(DEFAULT_NB_THREADS) ")\n"
								 "  -r, --range <int>\n"
								 "        Range of integer values inserted in set (default=" XSTR(DEFAULT_RANGE) ")\n"
								 "  -S, --seed <int>\n"
								 "        RNG seed (0=time-based, default=" XSTR(DEFAULT_SEED) ")\n"
								 "  -u, --update-rate <int>\n"
								 "        Percentage of update transactions (default=" XSTR(DEFAULT_UPDATE) ")\n"
								 "  -a , --move-rate <int>\n"
								 "        Percentage of move transactions (default=" XSTR(DEFAULT_MOVE) ")\n"
								 "  -s , --snapshot-rate <int>\n"
								 "        Percentage of snapshot transactions (default=" XSTR(DEFAULT_SNAPSHOT) ")\n"
								 "  -l , --load-factor <int>\n"
								 "        Ratio of keys over buckets (default=" XSTR(DEFAULT_LOAD) ")\n"
								 "  -x, --elasticity (default=4)\n"
								 "        Use elastic transactions\n"
								 "        0 = non-protected,\n"
								 "        1 = normal transaction,\n"
								 "        2 = read elastic-tx,\n"
								 "        3 = read/add elastic-tx,\n"
								 "        4 = read/add/rem elastic-tx,\n"
								 "        5 = elastic-tx w/ optimized move.\n"
								 );
					exit(0);
				case 'A':
					alternate = 1;
					break;
				case 'f':
					effective = atoi(optarg);
					break;
				case 'd':
					duration = atoi(optarg);
					break;
				case 'i':
					initial = atoi(optarg);
					break;
				case 't':
					nb_threads = atoi(optarg);
					break;
				case 'r':
					range = atol(optarg);
					break;
				case 'S':
					seed = atoi(optarg);
					break;
				case 'u':
					update = atoi(optarg);
					break;
				case 'a':
					move = atoi(optarg);
					break;
				case 's':
					snapshot = atoi(optarg);
					break;
				case 'l':
					load_factor = atoi(optarg);
					break;
				case 'x':
					unit_tx = atoi(optarg);
					break;
				case '?':
					printf("Use -h or --help for help\n");
					exit(0);
				default:
					exit(1);
		}
	}
	
	assert(duration >= 0);
	assert(initial >= 0);
	assert(nb_threads > 0);
	assert(range > 0 && range >= initial);
	assert(update >= 0 && update <= 100);
	assert(move >= 0 && move <= update);
	assert(snapshot >= 0 && snapshot <= (100-update));
	assert(initial < MAXHTLENGTH);
	assert(initial >= load_factor);
	
	printf("Set type     : lock-free hash table\n");
	printf("Duration     : %d\n", duration);
	printf("Initial size : %d\n", initial);
	printf("Nb threads   : %d\n", nb_threads);
	printf("Value range  : %ld\n", range);
	printf("Seed         : %d\n", seed);
	printf("Update rate  : %d\n", update);
	printf("Load factor  : %d\n", load_factor);
	printf("Move rate    : %d\n", move);
	printf("Snapshot rate: %d\n", snapshot);
	printf("Elasticity   : %d\n", unit_tx);
	printf("Alternate    : %d\n", alternate);	
	printf("Effective    : %d\n", effective);
	printf("Type sizes   : int=%d/long=%d/ptr=%d/word=%d\n",
				 (int)sizeof(int),
				 (int)sizeof(long),
				 (int)sizeof(void *),
				 (int)sizeof(uintptr_t));
	
	timeout.tv_sec = duration / 1000;
	timeout.tv_nsec = (duration % 1000) * 1000000;
	
	if ((data = (thread_data_t *)malloc(nb_threads * sizeof(thread_data_t))) == NULL) {
		perror("malloc");
		exit(1);
	}
	if ((threads = (pthread_t *)malloc(nb_threads * sizeof(pthread_t))) == NULL) {
		perror("malloc");
		exit(1);
	}
	
	if (seed == 0)
		srand((int)time(0));
	else
		srand(seed);
	
	maxhtlength = (unsigned int) initial / load_factor;
	set = ht_new();
	
	stop = 0;
	
	// Init STM 
	printf("Initializing STM\n");
	
	TM_STARTUP();
	
	// Populate set 
	printf("Adding %d entries to set\n", initial);
	i = 0;
	maxhtlength = (int) (initial / load_factor);
	while (i < initial) {
		val = rand_range(range);
		if (ht_add(set, val, 0)) {
		  last = val;
		  i++;			
		}
	}
	size = ht_size(set);
	printf("Set size     : %d\n", size);
	printf("Bucket amount: %d\n", maxhtlength);
	printf("Load         : %d\n", load_factor);
	
	// Access set from all threads 
	barrier_init(&barrier, nb_threads + 1);
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	for (i = 0; i < nb_threads; i++) {
		printf("Creating thread %d\n", i);
		data[i].first = last;
		data[i].range = range;
		data[i].update = update;
		data[i].load_factor = load_factor;
		data[i].move = move;
		data[i].snapshot = snapshot;
		data[i].unit_tx = unit_tx;
		data[i].alternate = alternate;
		data[i].effective = effective;
		data[i].nb_add = 0;
		data[i].nb_added = 0;
		data[i].nb_remove = 0;
		data[i].nb_removed = 0;
		data[i].nb_move = 0;
		data[i].nb_moved = 0;
		data[i].nb_snapshot = 0;
		data[i].nb_snapshoted = 0;
		data[i].nb_contains = 0;
		data[i].nb_found = 0;
		data[i].nb_aborts = 0;
		data[i].nb_aborts_locked_read = 0;
		data[i].nb_aborts_locked_write = 0;
		data[i].nb_aborts_validate_read = 0;
		data[i].nb_aborts_validate_write = 0;
		data[i].nb_aborts_validate_commit = 0;
		data[i].nb_aborts_invalid_memory = 0;
		data[i].nb_aborts_double_write = 0;
		data[i].max_retries = 0;
		data[i].seed = rand();
		data[i].set = set;
		data[i].barrier = &barrier;
		data[i].failures_because_contention = 0;
		if (pthread_create(&threads[i], &attr, test, (void *)(&data[i])) != 0) {
			fprintf(stderr, "Error creating thread\n");
			exit(1);
		}
	}
	pthread_attr_destroy(&attr);
	
	// Start threads 
	barrier_cross(&barrier);
	
	printf("STARTING...\n");
	gettimeofday(&start, NULL);
	if (duration > 0) {
		nanosleep(&timeout, NULL);
	} else {
		sigemptyset(&block_set);
		sigsuspend(&block_set);
	}
	//AO_store_full(&stop, 1);
	gettimeofday(&end, NULL);
	printf("STOPPING...\n");

	//print_ht(set);
	
	// Wait for thread completion 
	for (i = 0; i < nb_threads; i++) {
		if (pthread_join(threads[i], NULL) != 0) {
			fprintf(stderr, "Error waiting for thread completion\n");
			exit(1);
		}
	}
	duration = (end.tv_sec * 1000 + end.tv_usec / 1000) - (start.tv_sec * 1000 + start.tv_usec / 1000);
	aborts = 0;
	aborts_locked_read = 0;
	aborts_locked_write = 0;
	aborts_validate_read = 0;
	aborts_validate_write = 0;
	aborts_validate_commit = 0;
	aborts_invalid_memory = 0;
	aborts_double_write = 0;
	failures_because_contention = 0;
	reads = 0;
	effreads = 0;
	updates = 0;
	effupds = 0;
	moves = 0;
	moved = 0;
	snapshots = 0;
	snapshoted = 0;
	max_retries = 0;
	for (i = 0; i < nb_threads; i++) {
		printf("Thread %d\n", i);
		printf("  #add        : %lu\n", data[i].nb_add);
		printf("    #added    : %lu\n", data[i].nb_added);
		printf("  #remove     : %lu\n", data[i].nb_remove);
		printf("    #removed  : %lu\n", data[i].nb_removed);
		printf("  #contains   : %lu\n", data[i].nb_contains);
		printf("    #found    : %lu\n", data[i].nb_found);
		printf("  #move       : %lu\n", data[i].nb_move);
		printf("  #moved      : %lu\n", data[i].nb_moved);
		printf("  #snapshot   : %lu\n", data[i].nb_snapshot);
		printf("  #snapshoted : %lu\n", data[i].nb_snapshoted);
		printf("  #aborts     : %lu\n", data[i].nb_aborts);
		printf("    #lock-r   : %lu\n", data[i].nb_aborts_locked_read);
		printf("    #lock-w   : %lu\n", data[i].nb_aborts_locked_write);
		printf("    #val-r    : %lu\n", data[i].nb_aborts_validate_read);
		printf("    #val-w    : %lu\n", data[i].nb_aborts_validate_write);
		printf("    #val-c    : %lu\n", data[i].nb_aborts_validate_commit);
		printf("    #inv-mem  : %lu\n", data[i].nb_aborts_invalid_memory);
		printf("    #dup-w  : %lu\n", data[i].nb_aborts_double_write);
		printf("    #failures : %lu\n", data[i].failures_because_contention);
		printf("  Max retries : %lu\n", data[i].max_retries);
		aborts += data[i].nb_aborts;
		aborts_locked_read += data[i].nb_aborts_locked_read;
		aborts_locked_write += data[i].nb_aborts_locked_write;
		aborts_validate_read += data[i].nb_aborts_validate_read;
		aborts_validate_write += data[i].nb_aborts_validate_write;
		aborts_validate_commit += data[i].nb_aborts_validate_commit;
		aborts_invalid_memory += data[i].nb_aborts_invalid_memory;
		aborts_double_write += data[i].nb_aborts_double_write;
		failures_because_contention += data[i].failures_because_contention;
		reads += data[i].nb_contains;
		effreads += data[i].nb_contains + 
		(data[i].nb_add - data[i].nb_added) + 
		(data[i].nb_remove - data[i].nb_removed) + 
		(data[i].nb_move - data[i].nb_moved) +
		data[i].nb_snapshoted;
		updates += (data[i].nb_add + data[i].nb_remove + data[i].nb_move);
		effupds += data[i].nb_removed + data[i].nb_added + data[i].nb_moved; 
		moves += data[i].nb_move;
		moved += data[i].nb_moved;
		snapshots += data[i].nb_snapshot;
		snapshoted += data[i].nb_snapshoted;
		size += data[i].nb_added - data[i].nb_removed;
		if (max_retries < data[i].max_retries)
			max_retries = data[i].max_retries;
	}
	printf("Set size      : %d (expected: %d)\n", ht_size(set), size);
	printf("Duration      : %d (ms)\n", duration);
	printf("#txs          : %lu (%f / s)\n", reads + updates + snapshots, (reads + updates + snapshots) * 1000.0 / duration);
	
	printf("#read txs     : ");
	if (effective) {
		printf("%lu (%f / s)\n", effreads, effreads * 1000.0 / duration);
		printf("  #cont/snpsht: %lu (%f / s)\n", reads, reads * 1000.0 / duration);
	} else printf("%lu (%f / s)\n", reads, reads * 1000.0 / duration);
	
	printf("#eff. upd rate: %f \n", 100.0 * effupds / (effupds + effreads));
	
	printf("#update txs   : ");
	if (effective) {
		printf("%lu (%f / s)\n", effupds, effupds * 1000.0 / duration);
		printf("  #upd trials : %lu (%f / s)\n", updates, updates * 1000.0 / 
					 duration);
	} else printf("%lu (%f / s)\n", updates, updates * 1000.0 / duration);
	
	printf("#move txs     : %lu (%f / s)\n", moves, moves * 1000.0 / duration);
	printf("  #moved      : %lu (%f / s)\n", moved, moved * 1000.0 / duration);
	printf("#snapshot txs : %lu (%f / s)\n", snapshots, snapshots * 1000.0 / duration);
	printf("  #snapshoted : %lu (%f / s)\n", snapshoted, snapshoted * 1000.0 / duration);
	printf("#aborts       : %lu (%f / s)\n", aborts, aborts * 1000.0 / duration);
	printf("  #lock-r     : %lu (%f / s)\n", aborts_locked_read, aborts_locked_read * 1000.0 / duration);
	printf("  #lock-w     : %lu (%f / s)\n", aborts_locked_write, aborts_locked_write * 1000.0 / duration);
	printf("  #val-r      : %lu (%f / s)\n", aborts_validate_read, aborts_validate_read * 1000.0 / duration);
	printf("  #val-w      : %lu (%f / s)\n", aborts_validate_write, aborts_validate_write * 1000.0 / duration);
	printf("  #val-c      : %lu (%f / s)\n", aborts_validate_commit, aborts_validate_commit * 1000.0 / duration);
	printf("  #inv-mem    : %lu (%f / s)\n", aborts_invalid_memory, aborts_invalid_memory * 1000.0 / duration);
	printf("  #dup-w      : %lu (%f / s)\n", aborts_double_write, aborts_double_write * 1000.0 / duration);
	printf("  #failures   : %lu\n",  failures_because_contention);
	printf("Max retries   : %lu\n", max_retries);
	
	// Delete set 
	ht_delete(set);
	
	// Cleanup STM 
	TM_SHUTDOWN();
	
	free(threads);
	free(data);
	
	return 0;
}