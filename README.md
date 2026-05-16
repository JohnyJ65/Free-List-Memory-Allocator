# Simple Aligned Free List Memory Allocator
Overview
This project implements a small, self‑contained memory allocator using a statically allocated heap buffer. 

It provides basic heap‑management features including aligned allocation, block splitting, freeing, and coalescing of adjacent free blocks.

The allocator manages memory inside a fixed‑size buffer and maintains a doubly linked free list to track available blocks.

# Features
8‑Byte Alignment
All allocations are rounded up to the nearest multiple of 8 bytes.
The heap itself is also 8‑byte aligned using:

alignas(8) static char heap[HEAP_SIZE];
This ensures that all returned payload pointers meet typical alignment requirements for common data types.

Block Header Structure
Each block begins with a header:

struct Header {
    int size;        // size of the payload in bytes
    bool allocated;  // allocation status
    Header* prev;    // previous block in free list
    Header* next;    // next block in free list
};

The header is followed immediately by the payload.

Free List
The allocator maintains a doubly linked list of free blocks.
When a block is allocated, it is removed from the free list.
When a block is freed, it is inserted at the head of the free list.

Block Splitting
If a free block is larger than the requested size, it is split:

The first part becomes the allocated block.

The remainder becomes a new free block and is inserted into the free list.

Coalescing
When freeing a block, the allocator checks:

The next physical block

The previous physical block

If either is free, the blocks are merged into a single larger block.
This reduces fragmentation and restores large contiguous free regions.

How Allocation Works
Find the first free block large enough.

Align the requested size to 8 bytes.

If the block is larger than needed, split it.

Mark the block as allocated.

Return a pointer to the payload (header + sizeof(Header)).

How Freeing Works

Convert the payload pointer back to its header.

Insert the block into the free list.

Check the next physical block; merge if free.

Check the previous physical block; merge if free.

# Test Cases (from main())
1. Initial State
Code
Free list:
Block at <addr> size=1048552 allocated=0
The heap starts as one large free block.

Size = HEAP_SIZE - sizeof(Header).

2. Allocate 100 Bytes
Code
After allocating 100:
Free list:
Block at <addr> size=1048424 allocated=0
100 bytes is rounded up to 104.

The block is split.

The remainder becomes the new free block.

3. Allocate 200 Bytes
Code
After allocating 200:
Free list:
Block at <addr> size=1048200 allocated=0
200 bytes is already aligned.

Another split occurs.

Free list now contains the remaining large block.

4. Free the First Block
Code
After freeing first block:
Block at <addr> size=104 allocated=0
Block at <addr> size=1048200 allocated=0
The 104‑byte block is returned to the free list.

No coalescing yet because the next block is still allocated.

5. Free the Second Block (Coalescing)
Code
After freeing second block (coalesced):
Block at <addr> size=1048552 allocated=0
The 200‑byte block is freed.

It is physically adjacent to the 104‑byte block and the large remainder block.

All three blocks merge back into one large free block.

The heap returns to its initial state.

# Build & Run
Compile with any C++ compiler:

g++ -std=c++17 allocator.cpp -o allocator
./allocator

# Summary
This allocator demonstrates:

aligned memory allocation

free list management

block splitting

block coalescing

pointer‑based heap traversal

It provides a compact example of how dynamic memory can be managed inside a fixed buffer.# Free-List-Memory-Allocator
