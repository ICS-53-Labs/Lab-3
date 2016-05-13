/* Lab 3 */
/* Joseph Park, 24136956 */
/* Alex Dunn, 28474741 */


/* TODOS */
/* TESTING
 * Test Input Parsing and Tokenizing
 * Test Each Command (Except QUIT)
 * Test Packing and Unpacking
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

 /* User Input */
 /* Max arguments in a command */
#define MAX_ARGS 20
 /* Delimiter for Tokenizing */
#define TOKEN_DELIM " \t"
 /* Max command line characters */
#define MAX_CMD_LINE 80
 /* Commands */
#define ALLOCATE "allocate"
#define FREE "free"
#define BLOCKLIST "blocklist"
#define WRITEHEAP "writeheap"
#define PRINTHEAP "printheap"
#define QUIT "quit"
#define SET "set"
#define LIST "list"
 /* Arguments */
#define ARGS_ALLOCATE 1
#define ARGS_FREE 1
#define ARGS_BLOCKLIST 0
#define ARGS_WRITEHEAP 3
#define ARGS_PRINTHEAP 2
#define ARGS_QUIT 0
#define ARGS_SET 2
#define ARGS_LIST 0

 /* Constraints */
 /* Heap Size */
#define HEAP_SIZE 400
 /* Header Size */
#define HEADER_SIZE 2
 /* Smallest Heap Index */
#define MIN_INDEX 0
 /* Largest Heap Index */
#define MAX_INDEX HEAP_SIZE - 1
 /* Minimum Block Number */
#define MIN_BLOCK_NUMBER 1
 /* Maximum Block Number */
#define MAX_BLOCK_NUMBER 63 //6 bits - 1
 /* Unallocated Block Number */
#define UNA_BLOCK_NUMBER 0
 /* Minimum Block Size */
#define MIN_BLOCK_SIZE 1
 /* Maximum Block Size */
#define MAX_BLOCK_SIZE HEAP_SIZE
 /* Allocated Flag */
#define ALLOCATION_FLAG 1
 /* Free Flag */
#define FREE_FLAG 0

 /* Masking Values */
 /* Mask value to access the B 9 of a 2 byte value */
#define B9_TWO_BYTE 0x100
 /* Mask value to access the LS 6 bits of a 2 byte value */
#define LSB6_TWO_BYTE 0x3F
 /* Mask value to access the LS 8 bits of a 2 byte value */
#define LSB9_TWO_BYTE 0xFF
 /* Mask value to access the MSB of a 1 byte value */
#define MSB_ONE_BYTE 0x80
 /* Mask value to access the B 7 of a 1 byte value */
#define B7_ONE_BYTE 0x40
 /* Mask value to access the LS 6 bits of a 1 byte value */
#define LSB6_ONE_BYTE 0x3F
 /* Mask value to acces LS 7 bits of a 1 byte value */
#define LSB7_ONE_BYTE 0x40
 /* MS 2 bits of 1 byte */
#define MSB2_ONE_BYTE 0xC0

 /* Integer Error Codes */
 /* Success */
#define SUCCESS 1
 /* Non-Error Failure */
#define FAILURE 0
 /* Bad Position - Means Position is outside of the array*/
#define BAD_POS -1
 /* Unwritable Position - Means position is not writable in context. I.E. This may mean it is a header for writing into the payload */
#define UNW_POS -2
 /* Invalid Block Number */
#define BAD_BLOCK_NUM -3
 /* Invalid Block Size */
#define BAD_BLOCK_SIZE -4
 /* Heap is empty - Not allocated */
#define UNA_HEAP -5
 /* Not enough free space */
#define NO_SPACE -6
 /* Bad Command */
#define BAD_COMMAND -7
 /* Not Enough Arguments */
#define NEO_ARGS -8
/* Invalid value */
#define BAD_VALUE -9
/* Reading or Writing out of Heap Boundary */
#define OUT_OF_HEAP_CHAR_NUM -10
/* Payload Bigger than Block */
#define SPILLOVER_ERROR -11
/* Bad Option */
#define BAD_OPTION -12

//Designates how to handle spill over for writeHeap and readHeap
#define SPILLOVER "spillover"
#define FILL_IN_WRITE "fill"
#define SKIP_HEADER "skip"
#define FREE_PRINT "freeprint"
//debug statements included
#define DEBUG "debug"
//Relevant information only
#define STAT "stat"
//Allow coalescing
#define JOIN "join"
#define ALL "all"

/* Global Counter for Block Number */
int block_counter = 0;
int spillover = 1;
int fill_in_write = 1;
int skip_header = 1;
int free_print = 1;
int debug = 1;
int stat = 1;
int join = 1;


/* Set Alloc Flag */
void setAllocFlag(char* heap, size_t pos, unsigned char alloc_flag) {
	unsigned char byte_first = heap[pos];
	/* Shift the alloc_flag to the MSB of the byte */
	unsigned char alloc_temp = alloc_flag << 7;
	/* Get the other 7 bits of the first byte */
	unsigned char byte_temp = byte_first & LSB7_ONE_BYTE;
	/* Combine the two */
	byte_first = byte_temp | alloc_temp;
	heap[pos] = byte_first;
}

/* Set Block Number */
void setBlockNumber(char* heap, size_t pos, unsigned short block_number) {
	unsigned char byte_first = heap[pos];
	/* Get the LSB 6 bits of block_number */
	unsigned char number_temp = block_number & LSB6_TWO_BYTE;
	/* Get the other 2 bits of the byte */
	unsigned char byte_temp = byte_first & MSB2_ONE_BYTE;
	/* Combine */
	byte_first = byte_temp | number_temp;
	heap[pos] = byte_first;
}

/* Set Block Size */
void setBlockSize(char* heap, size_t pos, unsigned short block_size) {
	unsigned char byte_first = heap[pos];
	/* Get the 9th bit of block_size */
	unsigned short size_msb = (block_size & B9_TWO_BYTE) >> 2;
	/* Get the rest of first byte */
	unsigned char byte_temp = (byte_first & MSB_ONE_BYTE) | (byte_first & LSB6_ONE_BYTE);
	/* Combine */
	byte_first = byte_temp | size_msb;
	/* Set the last 8 bits to second byte */
	unsigned char byte_second = block_size;
	heap[pos] = byte_first;
	heap[pos + 1] = byte_second;
}

/* Pack Arguments into a Header at Specified Position in the Heap */
/* Variable Assumptions and Information: */
/* Pos is assumed to be the start of the block. [Pos, Pos + 1] is the header. */
/* Alloc_Flag's LSB 1 is only checked - all other bits are ignored */
/* Block_Number's LSB 6 are only checked */
/* Block_Size's LSB 9 are only checked */
int pack(char* heap, size_t pos, unsigned short block_number, unsigned short block_size, unsigned char alloc_flag) {
	/* Check arguments for validity */
	if (pos < MIN_INDEX || pos > MAX_INDEX) {
		return BAD_POS;
	}
	if (block_number > MAX_BLOCK_NUMBER) {
		return BAD_BLOCK_NUM;
	}
	if (block_size < MIN_BLOCK_SIZE || block_size > MAX_BLOCK_SIZE) {
		return BAD_BLOCK_SIZE;
	}
	if (!heap) {
		return UNA_HEAP;
	}
	if (debug || STAT) printf("Packing - Alloc: %u, Number: %u, Size: %u\n", alloc_flag, block_number, block_size);
	/* For some reason, can't comment this block out; just leave it here so it never executes */
	if (0) {
		/* Pack the first byte: Alloc_status, MSB of Block_size, and Block_Number */
		unsigned char byte_first;
		/* Alloc_status - Turn into a byte with MSB as value and rest 0 */
		/* Shift alloc_flag to the left 7 times */
		if (debug) printf("Alloc_Flag: %u\n", alloc_flag);
		unsigned char alloc_temp = alloc_flag << 7;
		if (debug) printf("Shift Left 7 of Flag: %u\n", alloc_temp);
		/* Block_Size MSB - Turn into a byte with MSB at 0, Value, then 6 0's */
		/* Extract the MSB of the block_size */
		unsigned short size_msb = block_size & B9_TWO_BYTE; //Mask
		if (debug) printf("AND MSB Mask of Size: %u\n", size_msb);
		/* Shift the MSB twice to the right */
		size_msb = size_msb >> 2;
		if (debug) printf("Shift Right 2 of Size: %u\n", size_msb);
		/* Block_Number - Turn into a byte with MSB and LSB at 0, with Value as rest */
		/* Extract the least 6 bits of the short */
		unsigned char number_temp = block_number & LSB6_TWO_BYTE;
		if (debug) printf("And LSB6 Mask of Number: %u\n", number_temp);
		/* Combine the values together */
		byte_first = alloc_temp | size_msb | number_temp;
		if (debug) printf("First byte: %u\n", byte_first);

		/* Pack the second byte: Rest of Block_size */
		/* Extract the least significant 8 bits of block_size */
		unsigned char byte_second = block_size & LSB9_TWO_BYTE;

		/* Place the two bytes into the pos and pos + 1, respectively; this creates the header. */
		heap[pos] = byte_first;
		heap[pos + 1] = byte_second;
	}

	setAllocFlag(heap, pos, alloc_flag);
	setBlockNumber(heap, pos, block_number);
	setBlockSize(heap, pos, block_size);

	return SUCCESS; 
}

/* Gets the allocation flag of the header at pos */
/* Assumes pos is at the first byte of the header */
unsigned char getAllocFlag(char* heap, size_t pos) {
	//Can just unpack, but that requires performing unnecessary stuff
	/* Unpack first byte */
	unsigned char byte_first = heap[pos];
	/* CGet MSB and shift to LSB */
	unsigned char alloc_flag = (byte_first & MSB_ONE_BYTE) >> 7;
	return alloc_flag;
}

/* Gets the block number of the header as pos */
unsigned short getBlockNumber(char* heap, size_t pos) {
	unsigned char byte_first = heap[pos];
	/* Mask to get the 6 LSB which is the block number */
	unsigned short block_number = (byte_first & LSB6_ONE_BYTE);
	return block_number;
}

/* Gets the block size of the header at pos */
unsigned short getBlockSize(char* heap, size_t pos) {
	unsigned char byte_first = heap[pos];
	/* Get 7th B of First, which is the MSB of size */
	/* Shift right by 2 so that it is the 9th B */
	unsigned short number_temp = (byte_first & B7_ONE_BYTE) << 2;

	unsigned char byte_second = heap[pos + 1];
	/* Combine the two */
	unsigned short block_size = byte_second | number_temp;
	return block_size;
}

/* Unpacks the header at pos, and at pos + 1 */
/* Pos assumes that it is a header, and not a payload. */
int unpack(char* heap, size_t pos, unsigned short* block_number, unsigned short* block_size, unsigned char* alloc_flag) {
	/* Check arguments for validity */
	if (pos < MIN_INDEX || pos > MAX_INDEX) {
		return BAD_POS;
	}
	if (!heap) {
		return UNA_HEAP;
	}
	/* This should essentially do the reverse of the pack function */
	*block_number = getBlockNumber(heap, pos);
	*block_size = getBlockSize(heap, pos);
	*alloc_flag = getAllocFlag(heap, pos);

	return SUCCESS;
}

/* Initialize Heap */
char* initHeap(int bytes) {
	/* Create and Allocate Heap */
	char* heap = (char*)malloc(bytes * sizeof(char));
	/* Create an initial Block */
	/* Pack initial Information into the Block Header */
	unsigned short block_number = UNA_BLOCK_NUMBER;
	unsigned short block_size = bytes - HEADER_SIZE;
	unsigned char alloc_flag = FREE_FLAG;
	pack(heap, 0, block_number, block_size, alloc_flag);
	/* Return Heap */
	return heap;
}

/* Finds the size_t pos of the block number */
int findPos(char* heap, unsigned short block_number, size_t* pos) {
	if (block_number < MIN_BLOCK_NUMBER || block_number > MAX_BLOCK_NUMBER) {
		return BAD_BLOCK_NUM;
	}
	size_t s = 0;
	while (s < MAX_INDEX) {
		if (debug) printf("Block Number of Pos %u: %u\n", s, getBlockNumber(heap, s));
		if (getBlockNumber(heap, s) == block_number) {
			*pos = s;
			return SUCCESS;
		}
		s += (HEADER_SIZE + getBlockSize(heap, s));
	}
	return FAILURE;
}

/* Allocate Block to Heap */
int allocate(char* heap, int size) {
	/* Check arguments for validity */
	if (!heap) {
		return UNA_HEAP;
	}
	if (size < MIN_BLOCK_SIZE || size > MAX_BLOCK_SIZE) {
		return BAD_BLOCK_SIZE;
	}
	/* Find first unallocated block that has enough room in payload for size */
	size_t i = 0;
	unsigned short block_size = getBlockSize(heap, i);
	if (debug) printf("Initial Block Size @ Index 0: %u\n", block_size);
	while (i < MAX_INDEX && (getAllocFlag(heap, i) != FREE_FLAG
		|| block_size < size)) {
		i += (HEADER_SIZE + block_size);
		if (debug) printf("In While New Index: %u\n", i);
		block_size = getBlockSize(heap, i);
		if (debug) printf("In While New Block Size: %u\n", block_size);
	}
	if (debug) printf("While Result Index: %u\n", i);
	if (debug) printf("While Result Block Size: %u\n", block_size);
	/* Variable i is at either >= MAX_INDEX or at the header of a valid block */
	/* block_size should also be the size of the valid block*/
	if (i < MAX_INDEX) { /* Overwrite the header */
		int result = pack(heap, i, ++block_counter, size, ALLOCATION_FLAG);
		if (result != SUCCESS) {
			if (debug) printf("Problem packing new information into allocated block. Error Code: %d\n", result);
			return result;
			if (debug) printf("No problem packing new infromation into allocated block.\n");
		}
		/* Determine if splitting is needed */
		unsigned short actual_new_size = HEADER_SIZE + size;
		if (debug) printf("Actual New Size: %u\n", actual_new_size);
		unsigned short actual_old_size = HEADER_SIZE + block_size;
		if (debug) printf("Actual Old Size: %u\n", actual_old_size);
		int leftovers = 0;
		if ((actual_old_size - actual_new_size) > HEADER_SIZE) {
			leftovers = 1;
		}
		if (leftovers) { /* Leftovers */
			/* Create new header */
			size_t split_pos = i + actual_new_size;
			if (debug) printf("Split Pos: %u\n", split_pos);
			unsigned short split_block_size = actual_old_size - actual_new_size - HEADER_SIZE;
			if (debug) printf("Split Block Size: %u\n", split_block_size);
			unsigned short split_block_number = UNA_BLOCK_NUMBER;
			if (debug) printf("Split Block Number: %u\n", split_block_number);
			unsigned char split_alloc_flag = FREE_FLAG;
			if (debug) printf("Split Alloc Flag: %u\n", split_alloc_flag);
			result = pack(heap, split_pos, split_block_number, split_block_size, split_alloc_flag);
			if (result != SUCCESS) {
				if (debug) printf("Problem packing new information into split block. Error Code: %d\n", result);
				return result;
			}
			if (debug) printf("No problem packing new infromation into split block.\n");
		}
		else {
			setBlockSize(heap, i, block_size);
		}
		if (STAT) {
			unsigned short bn, bs;
			unsigned char af;
			printf("\nAllocated Block Information\n");
			unpack(heap, i, &bn, &bs, &af);
			printf("Position: %u\nBlock Number: %u\nBlock Size: %u\nAlloc FLag: %u\n\n", i, bn, bs, af);
			if (leftovers) {
				size_t p = i + actual_new_size;
				printf("Split Block Information\n");
				unpack(heap, p, &bn, &bs, &af);
				printf("Position: %u\nBlock Number: %u\nBlock Size: %u\nAlloc FLag: %u\n\n", p, bn, bs, af);
			}
			
		}
		return SUCCESS;
	}
	else {
		return NO_SPACE;
	}
}

/* Free an allocated block by setting the allocation flag */
int freeBlock(char* heap, size_t pos) {
	/* Check arguments for validity */
	if (pos < MIN_INDEX || pos > MAX_INDEX) {
		return BAD_POS;
	}
	if (!heap) {
		return UNA_HEAP;
	}
	/* Set the allocation flag at pos */
	setAllocFlag(heap, pos, FREE_FLAG);
	/* Set Block Number */
	if (!free_print) {
		setBlockNumber(heap, pos, UNA_BLOCK_NUMBER);
	}
	return SUCCESS;
}

/* List all of the Blocks*/
void blocklist(char* heap) {
	size_t s = 0;
	/* Find initial start address */
	size_t start = (size_t)&heap, end;
	/* Print the column headings */
	printf("Size\tAllocated\tStart     \tEnd\n");
	if (debug) printf("MAX_INDEX: %u\n", MAX_INDEX);
	while (s < MAX_INDEX) {
		if (debug) printf("Current Pos: %u\n", s);
		/* Get actual block size */
		unsigned short size = getBlockSize(heap, s) + HEADER_SIZE;
		/* Get allocation status */
		char* alloc_flag = (getAllocFlag(heap, s) ? "yes" : "no");
		/* Find the ending address */
		end = (size_t)(start + size - 1);
		/* Print: -# means to right align with a min of # char. */
		printf("%-4u\t%-9s\t0x%-08x\t0x%-08x\n", size, alloc_flag, start, end);
		s += size;
		start += s;
	}
}

/* Writes characters into a block */
/* Behavior changes slightly based on global defines: SPILLOVER, FILL_IN_WRITE, SKIP_HEADER */
/* This function is like this because the specifications for how write should behave are unclear */
int writeHeap(char* heap, size_t pos, char ch, int num) {
	if (!heap) {
		return UNA_HEAP;
	}
	if (pos < MIN_INDEX || pos > MAX_INDEX) {
		return BAD_POS;
	}
	if (debug) printf("Num: %u\nPos: %u\n", num,pos);
	if (num < 1 || num > HEAP_SIZE - HEADER_SIZE) { /* For making sure it does not write OUTSIDE of the heap*/
		return OUT_OF_HEAP_CHAR_NUM;
	}

	unsigned short block_size = getBlockSize(heap, pos);
	unsigned short segment = num; /* Amount to write, if there is enough 
								  room, this is num */
	if (block_size < num) { /* If there is not enough room for the chars */
		if (spillover) {
			segment = block_size;
		}
		else {
			if (fill_in_write) {
				segment = block_size;
			}
			else {
				return SPILLOVER_ERROR;
			}
		}
	}
	size_t s = pos + HEADER_SIZE; /* First payload index */
	if (debug) printf("S: %u\nSegment: %u\n", s,segment);
	size_t i = 0;
	/* Write to block */
	while (i < segment) {
		heap[i + s] = ch;
		++i;
	}
	if (debug) printf("i+s: %u\n", i + s);
	/* i is equal to segment, which is block_size or num */
	if (block_size < num) { /* There is a spillover */
		unsigned short diff = num - segment;
		if (debug) printf("Diff: %u\n", diff);
		if (skip_header) { /* Skip the next block's header? */
			/* Recursive call to write the rest of the chars if Spillover is allowed, Error if not */
			return spillover ? writeHeap(heap, i + s, ch, diff) : SPILLOVER_ERROR;
		} 
		else { /* Do not skip next block's header; instead, overwrite it */
			if (spillover) { /* If spillover is allowed */
				while (i < diff + segment) { /* Write without skipping */
					heap[i + s] = ch;
					++i;
				}
			}
			else { /* Error since spillover is not allowed */
				return SPILLOVER_ERROR;
			}
		}
	}/* If there is no spillover, the job is done */
	return SUCCESS;

	/* Original Version */
	/* 	unsigned short segment = copies;
	if (getBlockSize(heap, i) < copies)
	{
	segment = getBlockSize(heap, i);
	copies -= getBlockSize(heap, i);
	}
	i += HEADER_SIZE;
	unsigned short j = 0;
	while (j < segment)
	{
	heap[i + j] = c;
	j++;
	}
	if (i + j < copies)
	{
	return SPILLOVER ? printToBlock(heap, c, i + j, copies) : SPILLOVER_ERROR;
	}
	return SUCCESS; */
}

/* Print characters from a block */
int printHeap(char* heap, size_t pos, int num) {
	if (!heap) {
		return UNA_HEAP;
	}
	if (pos < MIN_INDEX || pos > MAX_INDEX) {
		return BAD_POS;
	}
	if (num < 1 || num > HEAP_SIZE - HEADER_SIZE) { /* For making sure it does not read OUTSIDE of the heap*/
		return OUT_OF_HEAP_CHAR_NUM;
	}
	if (!spillover && num > getBlockSize(heap, pos)) {
		return SPILLOVER_ERROR;
	}

	size_t i = 0;
	i += HEADER_SIZE;
	int j = 0;
	/* Print block */
	while (j < num)
	{
		printf("%c", heap[i + j]);
		++j;
	}
	printf("\n");
	return SUCCESS;
	/* I just changed the variable names and switched the check to the beginning and added parameter checks */
	/* Also for both print and write, I already had a findpos function that I probably forgot to commit so I just moved that out of the function */
	/* Original Alex's Verion */
	/*
	size_t i = 0;
	unsigned short block_size = getBlockSize(heap, i);
	while (getBlockNumber(heap, i) != blockNum)
	{
		i += HEADER_SIZE + block_size;
		block_size = getBlockSize(heap, i);
	}
	if (getBlockSize(heap, i) < bytes) return SPILLOVER_ERROR;
	i += HEADER_SIZE;
	int j = 0;
	while (j < bytes)
	{
		printf("%c", heap[i + j]);
		j++;
	}
	printf("\n");
	return SUCCESS;
	*/
}

/* Join two adjacent unallocated blocks */
int coalesce(char* heap) {
	if (!heap) {
		return UNA_HEAP;
	}
	size_t current = 0;
	size_t next = current + HEADER_SIZE + getBlockSize(heap, current);
	/* Current = Free, Next = Free*/
	while (current < MAX_INDEX && next < MAX_INDEX) {
		if (0) { /* This should be faster since it skips unnecessary checks, but may cause problems that I don't really want to deal with right now. */
			if (!getAllocFlag(heap, current) && !getAllocFlag(heap, next)) {
				/* Add the size to the first header, essentially combining the blocks */
				unsigned short current_size = getBlockSize(heap, current);
				unsigned short next_size = getBlockSize(heap, next);
				unsigned short new_size = current_size + HEADER_SIZE + next_size;
				setBlockSize(heap, current, new_size);
				heap[next] = 0, heap[next + 1] = 0; /* Wipe the second header */
				next += HEADER_SIZE + next_size;
			}
			else if (getAllocFlag(heap, current) && !getAllocFlag(heap, next)) {
				current = next;
				next += HEADER_SIZE + getBlockSize(heap, next);
			}
			else if (!getAllocFlag(heap, current) && getAllocFlag(heap, next)) {
				current = next + HEADER_SIZE + getBlockSize(heap, next);
				next = current + HEADER_SIZE + getBlockSize(heap, next);
			}
			else {
				current = next + HEADER_SIZE + getBlockSize(heap, next);
				next = current + HEADER_SIZE + getBlockSize(heap, next);
			}
		}
		else { /* Simpler method */
			/* Join possible */
			if (!getAllocFlag(heap, current) && !getAllocFlag(heap, next)) {
				/* Add the size to the first header, essentially combining the blocks */
				unsigned short current_size = getBlockSize(heap, current);
				unsigned short next_size = getBlockSize(heap, next);
				unsigned short new_size = current_size + HEADER_SIZE + next_size;
				setBlockSize(heap, current, new_size);
				heap[next] = 0, heap[next + 1] = 0; /* Wipe the second header */
				next += HEADER_SIZE + next_size;
			}
			/* No join possible */
			else {
				current = next;
				next += HEADER_SIZE + getBlockSize(heap, next);
			}
		}
	}
	return SUCCESS;
}

/* Convert string to all lower-case */
void lower(char* string) {
	size_t i = 0;
	while (string[i] != '\0') {
		string[i] = tolower(string[i]);
		++i;
	}
}

/* Parses and tokenizes user input */
void parseLine(char* cmdline, char* argv[MAX_ARGS]) {
	const char delim[3] = TOKEN_DELIM;
	size_t s = strlen(cmdline);
	/* Get rid of \n */
	--s;
	cmdline[s] = '\0';
	size_t i = 0;
	/* Tokenize */
	argv[i] = strtok(cmdline, delim);
	while (argv[i] != NULL) {
		if (i >= MAX_ARGS - 1) {
			argv[i] = NULL;
			break;
		}
		argv[++i] = strtok(NULL, delim);
	}
	/* lower case the command */
	lower(argv[0]);
}

/* Count the number of arguments */
size_t countArgs(char* argv[MAX_ARGS]) {
	size_t s = 1;
	while (argv[s] != NULL) {
		++s;
	}
	return s - 1;
}

/* Toggle options */
int toggle(char* option, int val) {
	if (val != 0 && val != 1) {
		return BAD_VALUE;
	}
	if (!strcmp(option, DEBUG)) debug = val;
	else if (!strcmp(option, STAT)) stat = val;
	else if (!strcmp(option, JOIN)) join = val;
	else if (!strcmp(option, SPILLOVER)) spillover = val;
	else if (!strcmp(option, FILL_IN_WRITE)) fill_in_write = val;
	else if (!strcmp(option, SKIP_HEADER)) skip_header = val;
	else if (!strcmp(option, FREE_PRINT)) free_print = val;
	else if (!strcmp(option, ALL)) {
		debug = val;
		stat = val;
		join = val;
		spillover = val;
		fill_in_write = val;
		skip_header = val;
		free_print = val;
	}
	else return BAD_OPTION;
	return SUCCESS;
}

void listOptions() {
	printf("debug=%d\nstat=%d\njoin=%d\nspillover=%d\nfill=%d\nskip=%d\nfreeprint=%d\n", debug, stat, join, spillover, fill_in_write, skip_header, free_print);
}

/* Execute command */
/* Change char** heap to char* heap */
int executeCommand(char* argv[MAX_ARGS], char* heap) {
	/* Get number of arguments */
	size_t s = countArgs(argv);
	if (debug) printf("Command: %s\n", argv[0]);
	if (!strcmp(argv[0], ALLOCATE)) { /* allocate command */
		if (s < ARGS_ALLOCATE) {
			return NEO_ARGS;
		}
		/* Allocate Block */
		unsigned short block_size = atoi(argv[1]);
		int result = allocate(heap, block_size);
		if (result != SUCCESS) {
			return result;
		}
		printf("%u\n", block_counter);
		if (join) {
			return coalesce(heap);
		}
	}
	else if (!strcmp(argv[0], FREE)) { /* free command */
		if (s < ARGS_FREE) {
			return NEO_ARGS;
		}
		unsigned short block_number = atoi(argv[1]);
		size_t pos;
		int result = findPos(heap, block_number, &pos);
		if (result != SUCCESS) {
			return result;
		}
		/* Free Block */
		result = freeBlock(heap, pos);
		if (result != SUCCESS) {
			return result;
		}
		if (join) {
			return coalesce(heap);
		}
	}
	else if (!strcmp(argv[0], BLOCKLIST)) { /* blocklist command */
		if (s < ARGS_BLOCKLIST) {
			return NEO_ARGS;
		}
		/* Print all Blocks */
		blocklist(heap);
	}
	else if (!strcmp(argv[0], WRITEHEAP)) { /* writeheap command */
		if (s < ARGS_WRITEHEAP) {
			return NEO_ARGS;
		}
		/* Write to Block */
		unsigned short block_number = atoi(argv[1]);
		size_t pos; 
		int result = findPos(heap, block_number, &pos);
		if (result != SUCCESS) {
			return result;
		}
		char ch = argv[2][0];
		int copies = atoi(argv[3]);
		return writeHeap(heap, pos, ch, copies);
	}
	else if (!strcmp(argv[0], PRINTHEAP)) { /* printheap command */
		if (s < ARGS_PRINTHEAP) {
			return NEO_ARGS;
		}
		/* Print Contents of Block */
		unsigned short block_number = atoi(argv[1]);
		size_t pos;
		int result = findPos(heap, block_number, &pos);
		if (result != SUCCESS) {
			return result;
		}
		int copies = atoi(argv[2]);
		return printHeap(heap, pos, copies);
	}
	else if (!strcmp(argv[0], QUIT)) { /* quit command */
		if (s < ARGS_QUIT) {
			return NEO_ARGS;
		}
		/* Free Heap */
		free(heap);
		//*heap = NULL;
		exit(0);
	}
	else if (!strcmp(argv[0], SET)) {
		if (s < ARGS_SET) {
			return NEO_ARGS;
		}
		char* tog = argv[1];
		int val = atoi(argv[2]);
		return toggle(tog, val);
	}
	else if (!strcmp(argv[0], LIST)) {
		if (s < ARGS_LIST) {
			return NEO_ARGS;
		}
		listOptions();
	}
	else {
		return BAD_COMMAND;
	}
	return SUCCESS;
}

/* Result Code Look Up */
char* lookUp(int code) {
	switch (code) {
		case SUCCESS: return "Success";
		case FAILURE: return "General Failure";
		case BAD_POS: return "Invalid Position";
		case UNW_POS: return "Unwritable Position";
		case BAD_BLOCK_NUM: return "Invalid Block Number";
		case BAD_BLOCK_SIZE: return "Invalid Block Size";
		case UNA_HEAP: return "Unallocated Heap";
		case NO_SPACE: return "Not Enough Free Space";
		case BAD_COMMAND: return "Invalid Command";
		case NEO_ARGS: return "Not Enough Arguments";
		case BAD_VALUE: return "Invalid Value";
		case OUT_OF_HEAP_CHAR_NUM: return "Out of Heap Boundaries";
		case SPILLOVER_ERROR: return "Spillover Error";
		case BAD_OPTION: return "Bad Option. Options are debug, stat, join, spillover, fill, skip, freeprint\n";
		default: return "No such code";
	}
}

int main() {
	char* heap = initHeap(HEAP_SIZE);
	char cmdline[MAX_CMD_LINE];
	char* argv[MAX_ARGS];
	while (1) {
		printf("/>"); /* Prompt */
		if (!fgets(cmdline, MAX_CMD_LINE, stdin)) {
			printf("Fgets error\n");
		}
		if (feof(stdin)) {
			exit(0);
		}
		parseLine(cmdline, argv); /* Parse Line */
		int result = executeCommand(argv, heap); /* Execute Command */
		if (result != SUCCESS) {
			printf("Error: %s\n", lookUp(result));
		}
	}
	return 0;
}
