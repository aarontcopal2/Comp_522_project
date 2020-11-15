#include <stdint.h>


// types and structures
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