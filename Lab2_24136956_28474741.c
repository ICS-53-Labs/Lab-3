//Park, Joseph: 24136956
//Dunn, Alex: 29474741
 
/* TODO: 
 * Functions:
 * -Parseline
 * -EvaluateCommand
 * --Allocate (number of bytes in the allocated block) = Allocate a block of memory from heap. Print out a unique block number which is associated with the block of memory which has just been allocated. The block numbers should increment each time a new block is allocated. Ex: First block is number 1, second is 2, etc. Only allocated blocks receive block numbers.
 *		What happens when there is not enough space in the heap? Fragmentation? 
 *		Running out of block numbers? 
 * --Free (block number to free) = Free a block of memory. It must be an allocated block. When a block is freed, its block number is no longer valid. The block number should not be reused to number any new block.
 *		Block Number does not exist?
 *		Block Number associated with unallocated block, or previously allocated block? Ex: Free 1, Free 1
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
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Debug Set
#define DEBUG 0

int main () {
	return 0;
}