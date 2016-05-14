#include <stdio.h>

/* Create Heap - Heap contains all of the information in block headers - 400 bytes - Fixed, no need to increase */
/* Blocks are not word aligned; Make them word aligned */
/* Must Split Blocks into allocated and free sections, no need to coalesce */
/* Six Commands */
//Allocate - argument of number of bytes for the new block
//Free
//Write
//Print
//Block List - start and end addresses include header and footer
//Quit
/* Implicit Block List */

#define HEAP_SIZE = 400;

void printBlock(struct header* h);

int block_count = 0;

struct header {
	short size;
	char block_num;
	char flag;
};

void init(struct header** h) //have to pass as pointer to pointer if you want to call by reference
{
	(*h) = (struct header*) malloc(sizeof(char) * 400);
	(*h)->size = 400 - sizeof(struct header);
	(*h)->flag = 0;
	(*h)->block_num = 0;
}

void printBlock(struct header *h)
{
	printf("Size: %i\nBlock: %i\n", h->size, h->block_num);
}

void allocate(struct header **head, int size)
{
	struct header *conductor = *head;
	int i = 0;
	short actual_size = size + sizeof(struct header);
	
	while (i < 400 && conductor->flag && conductor->size < size)
	{
		conductor += actual_size;
		i += actual_size;
	}

	if (i >= 400)
	{
		printf("NO FREE BLOCKS OF REQUESTED SIZE\n");
		//no valid blocks
	}

	conductor->flag = 1;
	conductor->block_num = ++block_count;

	
	if (conductor->size > actual_size)
	{
		struct header *temp = conductor + actual_size;
		temp->block_num = -1;
		temp->size = conductor->size - actual_size;
		temp->flag = 0;
	}
	conductor->size = size;
}

void freeMem(struct header* h)
{
	free(h);
}

int main()
{
	struct header* h = NULL;
	init(&h);
	printBlock(h);
	allocate(&h, 100);
	printBlock(h);
	freeMem(h);
}