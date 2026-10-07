# Generic Data Structures Library
Three reusable templated containers, written from scratch, with a built-in test suite.

Demonstrates: class templates, iterators, move semantics, hashing, recursion, std::optional, unit testing with assert.
What's included
Container
Implementation notes
LinkedList<T>
Singly linked using unique_ptr nodes. pushFront, pushBack, reverse, range-for iteration. Teardown is iterative to avoid stack overflow on long lists.
HashMap<K, V, Hash>
Separate chaining. Grows (rehashes) when load factor would exceed 0.75. put, get (returns std::optional), erase.
BST<T, Compare>
Binary search tree with a custom comparator. insert (rejects duplicates), contains, inOrder, height.

# Build and run
g++ -std=c++17 -O2 -Wall -Wextra main.cpp -o ds

./ds

Expected output:

LinkedList OK

HashMap OK

BST OK

All tests passed

The tests use assert, so do not compile with -DNDEBUG.
# Known limitations
BST is unbalanced, so sorted input degrades it to a linked list. BST has no erase.
LinkedList::pushBack is O(n) because there is no tail pointer.
LinkedList iterators are forward-only.
