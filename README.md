# Micheal_lock_free_hashtable
Adopted from paper: High Performance Dynamic Lock-Free Hash Tables and List-Based Sets
author = {Michael, Maged M.},
title = {High Performance Dynamic Lock-Free Hash Tables and List-Based Sets},
year = {2002},
url = {https://doi.org/10.1145/564870.564881}

This work is part of my Comp522 project

**Current plan**: 
* Create end-to-end implementation mem−mng-new−smr. We can think of replacing wait-free reuse by SMR if we find that strategy better
* Check scaling with benchmark
* Integrate into hpctoolkit implementation

**To-do/pending items**
1. Memory management: alternative to SMR: reusing objects using wait-free DS
2. Benchmark: insert and delete with random-keys, Scaling on 100+ threads

	
**PlanB: Evaluating multilock strategy(if SMR doesnt work)**
* Create end-to-end implementation with multi-locks(multi-rw-locks) and larger bucket size(1000) and see if lock-contention is reduced
* Check scaling with benchmark 
* Integrate into hpctoolkit implementation
Note: PlanB may not be fruitful; the paper says the experiments were conducted for varying paramters and gave similar results i.e adding buckets may not improve the relative performance of multi-locks

**Questions:**
1. More Than You Ever Wanted to Know about Synchronization says they implement Micheal's hashtable buckets using Harris LL(Harris LL: https://timharris.uk/papers/2001-disc.pdf). But Micheal's paper says that its algo performs 2.5X better than harris LL(with mem-management). Which approach should we take?

<ins>Answer</ins>: Need to try both approaches. Here is what Vincent Gramoli said about implementing the algorithm in his paper
> You will probably be interested in the lock-free hashtable located here: https://github.com/gramoli/synchrobench/tree/master/c-cpp/src/hashtables/lockfree-ht
It reuses the lock-free linked list by Harris. Although the Synchrobench referenced Maged Michael's paper as the closest, the code differs from Maged' specification of the hash table that appeared in SPAA 2002 for which we did not have the source code.
A key difference is that Harris did not specify a memory reclamation algorithm while Maged discussed a few. Note that ThreadScan from ACM TOPC 2018 has been used to automatically add memory reclamation to Synchrobench Harris linked list and there is a tutorial from MIT linked from https://sites.google.com/site/synchrobench/teaching that indicates how to do this.

2. Memory-management techniques: IBM freelist and SMR. Which should be used? SMR may be better, but core algo is slightly different for both techniques. SMR code method is present in the paper, is freelist code simpler?


**Issues that may happen on integration with hpctoolkit:**
1. Memory reclamation- do we use SMR, IBM freelist, ThreadScan or wait-free channels(current approach of hpctoolkit)?
2. Memory consistency- picking appropriate memory ordering for the synchronization operations(CAS). Read https://en.cppreference.com/w/c/atomic/memory_order to understand each category of mem.ordering
