/* Lab 3 */
/* Joseph Park, 24136956 */
/* Alex Dunn, 28474741 */


/* TODOS */
/* Implement Writeheap
 * Implement Printheap
 * Implement Lowercasing arguments
 * Implement Error Checks for Bad Errors
 * Maybe Coalescing 
 */

#include <stdio.h>
#include <string.h>


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
 /* Arguments */
#define ARGS_ALLOCATE 1
#define ARGS_FREE 1
#define ARGS_BLOCKLIST 0
#define ARGS_WRITEHEAP 3
#define ARGS_PRINTHEAP 2
#define ARGS_QUIT 0

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
#define MIN_BLOCK_SIZE HEADER_SIZE + 1
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




/* Global Counter for Block Number */
int block_counter = 0;
/* Toggle for many things: if errors should occur when write spills over, read spills over, printing unalloced*/
char clean = 0;

//DEBUG statements included
#define DEBUG 0
//Relevant information only
#define STAT 1

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
	if (DEBUG || STAT) printf("Packing - Alloc: %u, Number: %u, Size: %u\n", alloc_flag, block_number, block_size);
	/* For some reason, can't comment this block out; just leave it here so it never executes */
	if (0) {
		/* Pack the first byte: Alloc_status, MSB of Block_size, and Block_Number */
		unsigned char byte_first;
		/* Alloc_status - Turn into a byte with MSB as value and rest 0 */
		/* Shift alloc_flag to the left 7 times */
		if (DEBUG) printf("Alloc_Flag: %u\n", alloc_flag);
		unsigned char alloc_temp = alloc_flag << 7;
		if (DEBUG) printf("Shift Left 7 of Flag: %u\n", alloc_temp);
		/* Block_Size MSB - Turn into a byte with MSB at 0, Value, then 6 0's */
		/* Extract the MSB of the block_size */
		unsigned short size_msb = block_size & B9_TWO_BYTE; //Mask
		if (DEBUG) printf("AND MSB Mask of Size: %u\n", size_msb);
		/* Shift the MSB twice to the right */
		size_msb = size_msb >> 2;
		if (DEBUG) printf("Shift Right 2 of Size: %u\n", size_msb);
		/* Block_Number - Turn into a byte with MSB and LSB at 0, with Value as rest */
		/* Extract the least 6 bits of the short */
		unsigned char number_temp = block_number & LSB6_TWO_BYTE;
		if (DEBUG) printf("And LSB6 Mask of Number: %u\n", number_temp);
		/* Combine the values together */
		byte_first = alloc_temp | size_msb | number_temp;
		if (DEBUG) printf("First byte: %u\n", byte_first);

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
		if (DEBUG) printf("Block Number of Pos %u: %u\n", s, getBlockNumber(heap, s));
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
	if (DEBUG) printf("Initial Block Size @ Index 0: %u\n", block_size);
	while (i < MAX_INDEX && (getAllocFlag(heap, i) != FREE_FLAG
		|| block_size < size)) {
		i += (HEADER_SIZE + block_size);
		if (DEBUG) printf("In While New Index: %u\n", i);
		block_size = getBlockSize(heap, i);
		if (DEBUG) printf("In While New Block Size: %u\n", block_size);
	}
	if (DEBUG) printf("While Result Index: %u\n", i);
	if (DEBUG) printf("While Result Block Size: %u\n", block_size);
	/* Variable i is at either >= MAX_INDEX or at the header of a valid block */
	/* block_size should also be the size of the valid block*/
	if (i < MAX_INDEX) { /* Overwrite the header */
		int result = pack(heap, i, ++block_counter, size, ALLOCATION_FLAG);
		if (result != SUCCESS) {
			if (DEBUG) printf("Problem packing new information into allocated block. Error Code: %d\n", result);
			return result;
			if (DEBUG) printf("No problem packing new infromation into allocated block.\n");
		}
		/* Determine if splitting is needed */
		unsigned short actual_new_size = HEADER_SIZE + size;
		unsigned short actual_old_size = HEADER_SIZE + block_size;
		if (actual_old_size > actual_new_size) { /* Leftovers */
			/* Create new header */
			size_t split_pos = i + actual_new_size;
			if (DEBUG) printf("Split Pos: %u\n", split_pos);
			unsigned short split_block_size = block_size - size - HEADER_SIZE;
			if (DEBUG) printf("Split Block Size: %u\n", split_block_size);
			unsigned short split_block_number = UNA_BLOCK_NUMBER;
			if (DEBUG) printf("Split Block Number: %u\n", split_block_number);
			unsigned char split_alloc_flag = FREE_FLAG;
			if (DEBUG) printf("Split Alloc Flag: %u\n", split_alloc_flag);
			result = pack(heap, split_pos, split_block_number, split_block_size, split_alloc_flag);
			if (result != SUCCESS) {
				if (DEBUG) printf("Problem packing new information into split block. Error Code: %d\n", result);
				return result;
			}
			if (DEBUG) printf("No problem packing new infromation into split block.\n");
		}
		if (STAT) {
			unsigned short bn, bs;
			unsigned char af;
			printf("\nAllocated Block Information\n");
			unpack(heap, i, &bn, &bs, &af);
			printf("Position: %u\nBlock Number: %u\nBlock Size: %u\nAlloc FLag: %u\n\n", i, bn, bs, af);
			if (actual_old_size > actual_new_size) {
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
	return SUCCESS;
}

/* List all of the Blocks*/
void blocklist(char* heap) {
	size_t s = 0;
	/* Find initial start address */
	size_t start = (size_t)&heap, end;
	/* Print the column headings */
	printf("Size\t\Allocated\tStart     \tEnd\n");
	while (s < MAX_INDEX) {
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
}

/* Count the number of arguments */
size_t countArgs(char* argv[MAX_ARGS]) {
	size_t s = 1;
	while (argv[s] != NULL) {
		++s;
	}
	return s - 1;
}

/* Execute command */
int executeCommand(char* argv[MAX_ARGS], char** heap) {
	/* Get number of arguments */
	size_t s = countArgs(argv);
	if (DEBUG) printf("Command: %s\n", argv[0]);
	if (!strcmp(argv[0], ALLOCATE)) {
		if (s < ARGS_ALLOCATE) {
			return NEO_ARGS;
		}
		/* Allocate Block */
		unsigned short block_size = atoi(argv[1]);
		int result = allocate(*heap, block_size);
		if (result != SUCCESS) {
			return result;
		}
		printf("%u\n", block_counter);
	}
	else if (!strcmp(argv[0], FREE)) {
		if (s < ARGS_FREE) {
			return NEO_ARGS;
		}
		unsigned short block_number = atoi(argv[1]);
		size_t pos;
		int result = findPos(*heap, block_number, &pos);
		if (result != SUCCESS) {
			return result;
		}
		/* Free Block */
		result = freeBlock(*heap, pos);
		if (result != SUCCESS) {
			return result;
		}
	}
	else if (!strcmp(argv[0], BLOCKLIST)) {
		if (s < ARGS_BLOCKLIST) {
			return NEO_ARGS;
		}
		/* Print all Blocks */
		blocklist(*heap);
	}
	else if (!strcmp(argv[0], WRITEHEAP)) {
		if (s < ARGS_WRITEHEAP) {
			return NEO_ARGS;
		}
		/* Write to Block */
	}
	else if (!strcmp(argv[0], PRINTHEAP)) {
		if (s < ARGS_PRINTHEAP) {
			return NEO_ARGS;
		}
		/* Print Contents of Block */
	}
	else if (!strcmp(argv[0], QUIT)) {
		if (s < ARGS_QUIT) {
			return NEO_ARGS;
		}
		/* Free Heap */
		free(*heap);
		*heap = NULL;
		exit(0);
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
		int result = executeCommand(argv, &heap); /* Execute Command */
		printf("Result: %s\n", lookUp(result));
	}
	return 0;
}