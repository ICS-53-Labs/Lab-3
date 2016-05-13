/* Payload Bigger than Block */
#define SPILLOVER_ERROR -9

//Designates how to handle spill over for writeHeap and readHeap
#define SPILLOVER 1

/*Write the character 'c' to the block a user-input number of times*/
int writeHeap(char* heap, unsigned short blockNum, char c, unsigned short copies)
{
	size_t i = 0;
	unsigned short block_size = getBlockSize(heap, i);
	/* Find target block */
	while (getBlockNumber(heap, i) != blockNum)
	{
		i += HEADER_SIZE + block_size;
		block_size = getBlockSize(heap, i);
		/* Target block does not exist in the heap */
		if (i > MAX_INDEX) return UNW_POS;
	}
	/* Target block is unallocated */
	if (!getAllocFlag(heap, i)) return UNW_POS;
	return printToBlock(heap, c, i, copies);
}

int printToBlock(char* heap, char c, int i, unsigned short copies)
{
	unsigned short segment = copies;
	if (getBlockSize(heap, i) < copies)
	{
		segment = getBlockSize(heap, i);
		copies -= getBlockSize(heap, i);
	}
	i += HEADER_SIZE;
	unsigned short j = 0;
	/* Write to block */
	while (j < segment)
	{
		heap[i + j] = c;
		j++;
	}
	/* if payload size > than current block size */
	if (i + j < copies)
	{
		return SPILLOVER ? printToBlock(heap, c, i + j, copies) : SPILLOVER_ERROR;
	}
	return SUCCESS;
}

/*Prints a given ammount of characters from the target block*/
int printHeap(char* heap, int blockNum, int bytes)
{
	size_t i = 0;
	unsigned short block_size = getBlockSize(heap, i);
	/* Find target block */
	while (getBlockNumber(heap, i) != blockNum)
	{
		i += HEADER_SIZE + block_size;
		block_size = getBlockSize(heap, i);
	}
	/* If segment to be read is larger than size of the block */
	if (getBlockSize(heap, i) < bytes) return SPILLOVER_ERROR;
	i += HEADER_SIZE;
	int j = 0;
	/* Print block */
	while (j < bytes)
	{
		printf("%c", heap[i + j]);
		j++;
	}
	printf("\n");
	return SUCCESS;
}

else if (!strcmp(argv[0], WRITEHEAP)) {
	if (s < ARGS_WRITEHEAP) {
		return NEO_ARGS;
	}
	/* Write to Block */
	unsigned short block_num = atoi(argv[1]);
	char c = argv[2][0];	//first character of the string
	unsigned short copies = atoi(argv[3]);
	return writeHeap(*heap, block_num, c, copies);
}
else if (!strcmp(argv[0], PRINTHEAP)) {
	if (s < ARGS_PRINTHEAP) {
		return NEO_ARGS;
	}
	/* Print Contents of Block */
	unsigned short block_num = atoi(argv[1]);
	unsigned short bytes = atoi(argv[2]);
	return printHeap(*heap, block_num, bytes);
}