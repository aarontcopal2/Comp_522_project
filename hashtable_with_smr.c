#include <stdio.h>
#include <stdbool.h>
#include <stdatomic.h>

#include "hashtable_with_smr.h"


// HP is the shared array of hazard pointers
// j is thread id for SMR purposes
// hp0=&HP[3*j]
// hp1=&HP[3*j+1]
// hp2=&HP[3*j+2]

// this is incorrect
NodeType **hp0, **hp1, **hp2, *cur, *next;
MarkPtrType *prev;

static void DeleteNode(NodeType *node) {
    // to be completed
}


static bool Find(MarkPtrType *head, KeyType key) {
try_again: ;
    prev = head;
    // <pmark, cur> = *prev; What is the C equivalent for this?
    bool pmark = prev->Mark;
    cur = prev->Next;

    *hp1 = cur;
    //if (*prev != <0, cur>)
    if (prev->Mark != 0 && prev->Next != cur) {
        goto try_again;
    }
    while (true) {
        if (cur == NULL) {
            return false;
        }
        // <cmark, next>←curˆ.<Mark, Next>;
        bool cmark = cur->Mark;
        next = cur->Next;
        *hp0 = next;
        //if curˆ<Mark, Next> != <cmark, next> {
        if (cur->Mark != cmark && cur->Next != next) {
            goto try_again;
        }
        KeyType ckey = cur->Key;
        //if *prev != <0, cur>{
        if (prev->Mark != 0 && prev->Next != cur) {
            goto try_again;
        }
        if (!cmark) {
            if (ckey >= key) {
                return ckey == key;
            }
            //prev= &curˆ.<Mark, Next> ;
            prev->Mark = cur->Mark;
            prev->Next = cur->Next;
            *hp2 = cur;
        } else {
            //if CAS(prev,<0,cur>,<0,next>)
            MarkPtrType expected = {0, cur};
            MarkPtrType desired = {0, next};

            if (atomic_compare_exchange_strong(prev, &expected, desired)) {
                DeleteNode(cur);
            } else {
                goto try_again;
            }
        }
        cur = next;
        *hp1 = next;
    }
}


static bool Insert(MarkPtrType *head, NodeType *node)
{
    bool result;
    KeyType key = node->Key;
    while (true) {
        if (Find(head, key)) {
            result = false;
            break;
        }
        //nodeˆ.<Mark,Next> = <0,cur>;
        node->Mark = 0;
        node->Next = cur;
        MarkPtrType expected = {0, cur};
        MarkPtrType desired = {0, node};
        //if CAS(prev,<0,cur>,<0,node>) {
        if (atomic_compare_exchange_strong(prev, &expected, desired)) {
            result = true;
            break;
        }
    }
    *hp0 = NULL;
    *hp1 = NULL;
    *hp2 = NULL;
    return result;
}


static bool Delete(MarkPtrType *head, KeyType key) {
    bool result;
    while (true) {
        if (!Find(head, key)) {
            result = false;
            break;
        }
        //if (!CAS(&curˆ.<Mark,Next>, <0,next>, <1,next>)) {
        MarkPtrType cur_markType = {cur->Mark, cur->Next};
        MarkPtrType expected = {0, next};
        MarkPtrType desired = {1, next};
        if (!atomic_compare_exchange_strong(&cur_markType, &expected, desired)) {
            continue;
        }
        MarkPtrType expected_d = {0, cur};
        MarkPtrType desired_d = {0, next};
        //if (CAS(prev,<0,cur>,<0,next>)) {
        if (atomic_compare_exchange_strong(prev, &expected_d, desired_d)) {
            DeleteNode(cur);
        } else {
            Find(head, key);
        }
        result = true;
        break;
    }
    *hp0 = NULL;
    *hp1 = NULL;
    *hp2 = NULL;
    return result;
}


bool Search(MarkPtrType *head, KeyType key) {
    bool result = Find(head, key);
    *hp0 = NULL;
    *hp1 = NULL;
    *hp2 = NULL;
    return result;
}
