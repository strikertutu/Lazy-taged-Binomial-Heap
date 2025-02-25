#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>

#define MAX_NODES 1000

// Node 
typedef struct BinomialNode {
    int id;             // identity
    int key;            // key value
    int degree;         // node degree
    int lazy;           // lazy tag
    struct BinomialNode *parent;
    struct BinomialNode *child;
    struct BinomialNode *sibling;
} BinomialNode;

// Binomial Heap
typedef struct BinomialHeap {
    BinomialNode *head;
} BinomialHeap;

// Adress for accessing node based on given ID direactly
BinomialNode* id_nodes[MAX_NODES];
//BinomialNode nodes[MAX_NODES];

// Create a new node
BinomialNode* createNode(int id, int key) {
    BinomialNode *node = (BinomialNode*)malloc(sizeof(BinomialNode));
    id_nodes[id] = node;
    node->id = id;
    node->key = key;
    node->degree = 0;
    node->lazy = 0;
    node->parent = NULL;
    node->child = NULL;
    node->sibling = NULL;
    return node;
}

// For debugging
void print_node(BinomialNode *node)
{
	printf("node[%d]: %d\n", id_nodes[node->id]->id, node->id);
	printf("\t key:%d\n", node->key);
	printf("\t degree:%d\n", node->degree);
	printf("\t lazy:%d\n", node->lazy);
	if(node->parent) printf("\t parent:%d\n", node->parent->id);
	if(node->child) printf("\t child:%d\n", node->child->id);
	if(node->sibling) printf("\t sibling:%d\n", node->sibling->id);
}

// For debugging
void print_heap_node(BinomialNode *node)
{
	print_node(node);
	if(node->child) print_heap_node(node->child);	
	if(node->sibling) print_heap_node(node->sibling);
}

// For debugging
void print_heap(BinomialHeap *H)
{
	if (H->head == NULL) return;
	print_heap_node(H->head);
	printf("Done!\n");
}



// Linking two binomial trees with the same degree
BinomialNode* binomialLink(BinomialNode *y, BinomialNode *z) {
    y->parent = z;
    y->lazy -= z->lazy;  // 调整懒标记
    y->sibling = z->child;
    z->child = y;
    z->degree++;
    return z;
}

// Merge the root lists of two binomial heaps
BinomialHeap* binomialHeapMerge(BinomialHeap *H1, BinomialHeap *H2) {
    BinomialHeap *H = (BinomialHeap*)malloc(sizeof(BinomialHeap));
    BinomialNode *p1 = H1->head, *p2 = H2->head;
    BinomialNode *prev = NULL, *next = NULL;
    if (p1 == NULL) {
        H->head = p2;
        return H;
    }
    if (p2 == NULL) {
        H->head = p1;
        return H;
    }
    if (p1->degree <= p2->degree) {
        H->head = p1;
        p1 = p1->sibling;
    } else {
        H->head = p2;
        p2 = p2->sibling;
    }
    next = H->head;
    while (p1 != NULL && p2 != NULL) {
        if (p1->degree <= p2->degree) {
            next->sibling = p1;
            p1 = p1->sibling;
        } else {
            next->sibling = p2;
            p2 = p2->sibling;
        }
        next = next->sibling;
    }
    if (p1 != NULL) {
        next->sibling = p1;
    } else {
        next->sibling = p2;
    }
    H1 = H; 
    return H;
}

// Merge two binomial heaps
BinomialHeap* binomialHeapUnion(BinomialHeap *H1, BinomialHeap *H2) {
    BinomialHeap *H = binomialHeapMerge(H1, H2);
    if (H->head == NULL) return H;
    BinomialNode *prev = NULL, *x = H->head, *next = x->sibling;
    while (next != NULL) {
        if (x->degree != next->degree || (next->sibling != NULL && next->sibling->degree == x->degree)) {
            prev = x;
            x = next;
        } else if (x->key+ x->lazy <= next->key+next->lazy) {
            x->sibling = next->sibling;
            binomialLink(next, x);
        } else {
            if (prev == NULL) {
                H->head = next;
            } else {
                prev->sibling = next;
            }
            binomialLink(x, next);
            x = next;
        }
        next = x->sibling;
    }
    return H;
}

// Insert a new node into the binomial heap
BinomialHeap* binomialHeapInsert(BinomialHeap *H, int id, int key) {
	if(id<=0){
		printf("Invalid id!");
		return NULL;
	} 
    BinomialNode *node = createNode(id, key);
    BinomialHeap *H1 = (BinomialHeap*)malloc(sizeof(BinomialHeap));
    H1->head = node;
    return binomialHeapUnion(H, H1);
}

// Get the key value of the node with the specified id
int getKey(int id) {
    BinomialNode *node = id_nodes[id];
    int key = node->key+node->lazy;

    BinomialNode *current = node->parent;
    while (current != NULL) {
        key += current->lazy;
        current = current->parent;
    }

    return key;
}

// Add or subtract a value to all nodes in the binomial heap
void binomialHeapAddValue(BinomialHeap *H, int value) {
    BinomialNode *current = H->head;
    while (current != NULL) {
        current->lazy += value;
        current = current->sibling;
    }
}

// Get the minimum node id in the heap
int binomialHeapMinimum(BinomialHeap *H) {
    if (H->head == NULL) return -1;
    BinomialNode *min = H->head;
    BinomialNode *current = min->sibling;
    while (current != NULL) {
        if (getKey(current->id) < getKey(min->id)) {
            min = current;
        }
        current = current->sibling;
    }
    return min->id;
}

// Swap the positions of two nodes
void swapNodes(BinomialNode *a, BinomialNode *b) {
	
    int temp = a->key;
    a->key = b->key;
    b->key = temp;
    
    a->key -= b->lazy;
    
    temp = a->id;
    a->id = b->id;
    b->id = temp;
	
	id_nodes[a->id] = a;
	id_nodes[b->id] = b;
}

// Adjust the node upward
void bubbleUp(BinomialNode *node) {
    while (node->parent != NULL) {
        swapNodes(node, node->parent);
        node = node->parent;
    }
}

// Delete the node with the specified id
BinomialHeap* binomialHeapDelete(BinomialHeap *H, int id) {
    BinomialNode *node = id_nodes[id];
    node->key = INT_MIN;
    bubbleUp(node);
	

    // Remove the smallest node from the root linked list
    BinomialNode *prev = NULL, *current = H->head;
    while (current != NULL && current->id != id) {
        prev = current;
        current = current->sibling;
    }
    if (prev == NULL) {
        H->head = current->sibling;
    } else {
        prev->sibling = current->sibling;
    }

    // Reverse the child node list of the deleted node
    BinomialNode *child = current->child;
    BinomialNode *prevChild = NULL, *nextChild;
    
    while (child != NULL) {
    	child->lazy += current->lazy;
        nextChild = child->sibling;
        child->sibling = prevChild;
        child->parent = NULL;
        prevChild = child;
        child = nextChild;
    }
    
    // Create a new binomial heap containing the children of the deleted node
    BinomialHeap *H1 = (BinomialHeap*)malloc(sizeof(BinomialHeap));
    H1->head = prevChild;

    // Merge the original and new piles
    return binomialHeapUnion(H, H1);
}

BinomialHeap* pouring_out(BinomialHeap* H)
{
	int x;
	int pre = INT_MIN;
    x = binomialHeapMinimum(H);
    while(x>=0)
    {
    	int a = getKey(x);
    	if(a<pre) printf("Error!");
    	pre = a;
    	printf("Node %d, key: %d\n", x, a);
    	printf("Deleting %d\n", x);
    	H = binomialHeapDelete(H, x);
    	x = binomialHeapMinimum(H);
	}
	return H;
}

int main() {
	//Small test
    BinomialHeap *H = (BinomialHeap*)malloc(sizeof(BinomialHeap));
    BinomialHeap *H1 = (BinomialHeap*)malloc(sizeof(BinomialHeap));
    H->head = NULL;
    H1->head = NULL;
    srand((unsigned)time(NULL));
    
    for(int i = 1;i<100;i++)
    {
    	H = binomialHeapInsert(H, i, rand()%1000-500);
    	binomialHeapAddValue(H, rand()%1000-500);
	}
	for(int i = 100;i<200;i++)
    {
    	H1 = binomialHeapInsert(H1, i, rand()%1000-500);
    	binomialHeapAddValue(H1, rand()%1000-500);
	}
	H = binomialHeapUnion(H,H1);
    H = pouring_out(H);
    return 0;
}

/*
    H = binomialHeapInsert(H, 1, 10);
    H = binomialHeapInsert(H, 2, 20);
    H = binomialHeapInsert(H, 3, 50);
    H = binomialHeapInsert(H, 4, 7);
    H = binomialHeapInsert(H, 5, 9);

	BinomialHeap *H2 = (BinomialHeap*)malloc(sizeof(BinomialHeap));
    H2->head = NULL;
    
    H2 = binomialHeapInsert(H2, 6, 10);
    H2 = binomialHeapInsert(H2, 7, 20);
    H2 = binomialHeapInsert(H2, 8, 50);
    H2 = binomialHeapInsert(H2, 9, 7);
    H2 = binomialHeapInsert(H2, 10, 9);

    printf("Minimum node id: %d, key: %d\n", binomialHeapMinimum(H), getKey(binomialHeapMinimum(H)));

    printf("Key of node with id 2: %d\n", getKey(2));

    binomialHeapAddValue(H, 25);
    printf("Key of node with id 2 after adding 25: %d\n", getKey(2));
    
    H = binomialHeapUnion(H, H2);
    print_heap(H);
*/
