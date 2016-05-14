//Park, Joseph: 24136956
//Dunn, Alex: 29474741
//http://cs.nyu.edu/~acase/fa14/CS201/labs/malloclab-recitation.pdf
/* TODO: 
 * Functions:
 * -Parseline
 * -EvaluateCommand
 * --Allocate (number of bytes in the allocated block) = Allocate a block of memory from heap. Print out a unique block number which is associated with the block of memory which has just been allocated. The block numbers should increment each time a new block is allocated. Ex: First block is number 1, second is 2, etc. Only allocated blocks receive block numbers.
 *		What happens when there is not enough space in the heap? Fragmentation? 
 *		Running out of block numbers? 
 *		Function should return a pointer to the allocated memory block. If size is 0, then NULL returns.
 *		Allocation should look for the first available space. What about combining free spaces?
 * --Free (block number to free) = Free a block of memory. It must be an allocated block. When a block is freed, its block number is no longer valid. The block number should not be reused to number any new block.
 *		Block Number does not exist?
 *		Block Number associated with unallocated block, or previously allocated block? Ex: Free 1, Free 1
 *		Function returns the block
 * --Blocklist () = Print out information about all of the blocks in your heap. Each block per row. First row is Column Headers: Size, Allocated, Start, End. The columns are size (number of bytes in the block), allocated (yer or no), start (starting address), end (ending address)
 *		How to find starting and ending address?
 *		How to find size?
 *		How to determine if block is allocated?
 *		How to iterate through the heap?
 * --Writeheap (block number, character, number of char) = Writes the character, number of char times into block. The character is written into number successive locations in the block. Do not overwrite the header of the block, only the payload.
 *		How to separate block into header and payload?
 *		What to do when number of chars exceeds block size, or available bytes?
 * --Printheap (block number, number of bytes to print) = Print the contents of the block. Do not print the header, only the payload. If the number of bytes exceeds the block size, print all of the bytes anyways, even if they extend into the other blocks
 * --Quit () = Quit the program.
 * Other Stuff:
 * Initialize Heap when starting
 * Delete Heap when done
 * Prompt for/Get Input
 * Have Block Number/Increment/Never reuse
 * Determine if a block is allocated or not/or figure out the size in between allocated blocks
 * Find out the starting and ending addresses
 * Do not use malloc and free, only for the heap.
 * Free list, dynamic allocator
 * Heap consistency checker, call after every change to the heap
		Is every chunk of memory in the free list actually free?
		Are there any consecutive free chunks of memory that escaped coalescing?
		Is every chunk of memory that is free actually in the free list?
		Do any allocated chunks of memory overlap?
 * HEAP:
 * Implemented as a collection of variable sized blocks, which can be allocated or free
 * Is Heap word-addressed (4 bytes per address)? or is it byte addressed.
 * From Power Point:
 * Knowing How Much to Free:
 *		Keep the length of a block in the word preceding the block, AKA the header. This requires one extra word per allocation block
 * Keeping Track of Free Blocks:
 * 		Free List - Implicit: Using length, link all blocks (linked list) - Explicit: Using pointers, only free blocks; 
 * Implicit List:
		Store allocation flag in the last bit of the size header word. Mask the size word to determine
		To find a free block: search list from beginning, choose first free block that fits OR choose best free block (that leaves the fewest bytes left over)
		Allocating in Free block: Split the block?
		Freeing a block: clear the allocated flag, clear the memory- How?
		Coalescing: Joining free adjacent blocks. Bidirectional Coalescing requires a footer - an additional word per block
 * Explicit List - Requires a linked list, pref. double linked list
 * Fragmentation - When a size n block is allocated in a free block of size m > n, we can split the free block into n and m - n blocks, having the m - n block free, or assume the m - n portion is allocated as well
 * Useful functions
 * mem_sbrk (n) = grows heap by positive n bytes
 * mem_heap_lo () = pointer to the lowest byte in the heap
 * mem_heap_hi () = pointer to the first byte above the heap
 * mem_heapsize () = size of heap
 */
 /*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* heap;
//Initializing and allocating heap: heap= (char*)malloc(MAX_HEAP_SIZE);
static char* heap_last;
//Initializing last heap: heap_last = (char*) heap;
static char* heap_max;
//Initializing heap max address: heap_max = (char*) (heap + MAX_HEAP_SIZE);
*/
//From Text Book
//Extend heap by n bytes and retuns the start address of the new area.
/* Heap extender void* mem_sbrk (int n) {
	char* start_addr = heap_last;
	if (n < 0 || heap_last + n > heap_max) {
		//Error
		return (void*) -1;
	}
	heap_last += n;
	return (void*) start_addr;
}
//Create heap with initial free block
int mm_init(void) {
	if ((heap_listp = mem_sbrk(4*WSIZE)) == (void*)-1) return -1;
	PUT(head_listp,0); //Alignment padding
	PUT(head_listp+(1*WSIZE),PACK(DSIZE,1)); //Prologue header
	PUT(head_listp+(2*WSIZE),PACK(DSIZE,1)); //Prologue footer
	PUT(head_listp+(3*WSIZE),PACK(0,1)); //Epilogue header
	head_listp += (2*WSIZE);
	//Extend the empty heap with a free block
	if (extend_heap(CHUNKSIZE/WSIZE) == NULL) return -1;
	return 0;
}
//Extend the heap with a new free block
static void* extend_heap(size_t words) {
	char* bp;
	size_t size;
	//Allocate an even number of words to maintain alignment
	size = (words%2)?(words+1)*WSIZE : words*WSIZE;
	if ((long)(bp=mem_sbrk(size))==-1) return NULL;
	//Initialize free block header/footer and the epilogue header
	PUT(HDRP(bp),PACK(size,0)); //new free block header
	PUT(FTRP(bp),PACK(size,0)); //new free block footer
	PUT(HDRP(NEXT_BLKP(bp)),PACK(0,1)); //new epilogue header
	//Coalesce if the previous block was free
	return coalesce(bp);
}
//Free
void mm_free(void* bp) {
	size_t size = GET_SIZE(HDRP(bp));
	PUT(HDRP(bp),PACK(size,0));
	PUT(FTRP(bp),PACK(size,0));
	caolesce(bp);
}
//Coalesce
static void* coalesce (void* bp) {
	size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
	size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
	size_t size = GET_SIZE(HDRP(bp));
	if(prev_alloc && next_alloc) return bp; //Case 1
	else if (prev_alloc && !next_alloc) { //Case 2
		size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
		PUT(HDRP(bp),PACK(size,0));
		PUT(FTRP(bp),PACK(size,0));
	}
	else if (!prev_alloc && next_alloc) { //Case 3
		size += GET_SIZE(HDRP(PREV_BLKP(bp)));
		PUT(FTRP(bp),PACK(size,0));
		PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
		bp=PREV_BLKP(bp);
	}
	else { //Case 4
		size += GET_SIZE(HDRP(PREV_BLKP(bp)))+GET_SIZE(FTRP(NEXT_BLKP(bp)));
		PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
		PUT(FTRP(MEXT_BLKP(bp)),PACK(size,0));
		bp=PREV_BLKP(bp);
	}
	return bp;
}
//Allocated
void* mm_malloc (size_t size) {
	size_t asize; //Adjusted block size
	size_t extendsize; //Amount to extend heap if no fit
	char* bp;
	//Ignore spurious requests
	if (size==0) return NULL;
	//Adjust block size to include overhead and alignment reqs
	if (size <= DSIZE) asize =2*DSIZE;
	else asize=DSIZE*((size+DSIZE+DSIZE-1)/DSIZE);
	//Search the free list for a fit
	if ((bp=find_fit(asize)) != NULL) {
		place(bp,asize);
		return bp;
	}
	//No fit found. Get more memory and place the block
	extendsize=MAX(asize,CHUNKSIZE);
	if((bp=extend_heap(extendsize/WSIZE))==NULL) return NULL;
	place(bp,asize);
	return bp;
}
*/

//Macros for free list manipulation
/*
wsize = 4, dsize = 8, chunksize = 1<<12
MAX(x,y) x > y ? x : y
//Pack a size and allocated bit into a word
PACK(size,alloc) size | alloc
//Read and write a word at address parsfnm
GET(p) *(unsigned int *)parsfnm
PUT(p,val) *(unsigned int *) p = val
//Read the size and allocated fields from address parsfnm
GET_SIZE(p) GET(p) & ~0x7
GET_ALLOC(p) GET(p) & 0x1
//Given block ptr bp, compute address of its header and footer
HDRP(bp) (char*)bp-wsize
FTRP(bp) (char*)bp+get_size(HDRP(bp)-dsize)
//Given block ptr bp, compute addess of next and previous blocks
NEXT_BLKP(bp) (char*)bp+get_size(char*)bp-wsize
PREV_BLKP(bp) (char*)bp-get_size(char*)bp-dsize
*/
/* Functions
-Allocate - Allocate memory in heap. Used for allocate command
-Free - Free memory. Used for free command
-Print
-Write
-Read
-Join
-Check heap
-Get addresses of next and previous blocks
-Get addresses of header and footer
-Get size, allocation flag
-initialize heap
-extend heap
-find free block
*/

//Debug Set
#define DEBUG 0

int main () {
	return 0;
}