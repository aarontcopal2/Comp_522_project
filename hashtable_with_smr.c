#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdatomic.h>

typedef uint64_t KeyType;

typedef struct NodeType {
    KeyType Key;
    bool Mark;
    struct NodeType *Next;
} NodeType;

typedef struct MarkPtrType {
    bool Mark;
    NodeType *Next;
} MarkPtrType;

// HP is the shared array of hazard pointers
// j is thread id for SMR purposes
// hp0=&HP[3*j]
// hp1=&HP[3*j+1]
// hp2=&HP[3*j+2]

// this is incorrect
NodeType **hp0, **hp1, **hp2;


static void DeleteNode(NodeType *node) {
    // to be completed
}


static bool Find(MarkPtrType *head, KeyType key) {
try_again: ;
    MarkPtrType * prev = head;
    // <pmark, cur> = *prev; What is the C equivalent for this?
    bool pmark = prev->Mark;
    NodeType *cur = prev->Next;

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
        NodeType *next = cur->Next;
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

int main() {
    
}