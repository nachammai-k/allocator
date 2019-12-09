#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "allocate.h"

struct context* initialize_context(void *addr, int n_cpu, int n_pages, int total_buf, struct free_list** percpu_freelist)
{
	struct context *ctx = malloc(sizeof(struct context));
	ctx->start_address = addr;
	ctx->n_cpu = n_cpu;
	ctx->n_pages = n_pages;
	ctx->end_address = addr + total_buf;
	ctx->percpu_freelist = percpu_freelist;
}


void init_percpu_freelist(void *start_mem, struct free_list **percpu_freelist, int n_cpu, int n_pages)
{
	int i, n;
	void *cpu_buffer = start_mem + (n_cpu * n_pages * PAGE_SIZE);
	struct free_list *prev = NULL;
	for( n = 0; n < n_cpu; n++)
	{
		prev = NULL;
	for ( i = 0; i < n_pages; i++)
	{
		struct free_list *temp = malloc(sizeof(struct free_list));
		temp->start_address = start_mem + (i * PAGE_SIZE) + (n * n_pages * PAGE_SIZE);
		if (i == 0)
		{
			memset(temp->start_address, 0, PAGE_SIZE);
			continue;
		}
		if (prev)
			prev->next = temp;
		else
			percpu_freelist[n] = temp;
		prev = temp;
	}
	prev->next = NULL;
	}
}

struct context* register_address(void *addr, int n_cpu, int n_pages, int total_buf)
{
	int i;
	struct free_list** percpu_freelist;
	if ((n_cpu * n_pages) > total_buf / (PAGE_SIZE))
	     return 0;	
        percpu_freelist = malloc(sizeof(struct free_list *) * n_cpu);
	init_percpu_freelist(addr, percpu_freelist, n_cpu, n_pages);	
	return initialize_context(addr, n_cpu, n_pages, total_buf, percpu_freelist);
}

void* compute_first_page(struct context *ctx, int cpu)
{
	return ctx->start_address + (cpu * ctx->n_pages * PAGE_SIZE);
}

void set_bit_map(uint8_t* bitmap_address, void *page_address)
{
	int page_number = (page_address - (void *)bitmap_address) / (PAGE_SIZE);
        int offset = page_number / 8;
	uint8_t bit;
	if (offset)
		bit = 1 << (offset * 8 - page_number);
	else
		bit = 1 << page_number;
        bitmap_address[offset] = bitmap_address[offset] | bit ;
	printf("%d set %x\n" , offset, bit);
}

void dump(struct context *ctx)
{
	int i, j;
	for (i = 0; i < ctx->n_cpu; i++)
	{
		printf("CPU %d\n", i);
		void *start_page = compute_first_page(ctx, i);
		int count = 0;
		for (j = 0; j < ctx->n_pages; j++)
		{
			unsigned long long bitmap = ((unsigned long long *) start_page)[j];
			int k, qword_read = 0;
			for (k = 0; k < sizeof(long long); k++){
				if (bitmap & (1 << k))
				{
					printf("page %d is allocated\n", qword_read * sizeof(long long) + k);
					count++;
				}
			}
			qword_read++;
			j = j + sizeof(long long);			
		}

		printf(" Total pages allocated %d CPU %d\n", count, i); 

	}


}




void* allocate_page(struct context *ctx, int cpu)
{
	void *address = NULL;
	if (ctx->percpu_freelist[cpu])
	{
		struct free_list *current = ctx->percpu_freelist[cpu];
		void *start_address = compute_first_page(ctx, cpu);
		ctx->percpu_freelist[cpu] = current->next;
                set_bit_map((uint8_t *)start_address, current->start_address); 
		address = current->start_address;
		free(current);
	}
	return address;

}

int find_cpu(struct context *ctx, void *address)
{
	return (address - ctx->start_address) / (PAGE_SIZE * ctx->n_pages);
         

}


int isAllocated(struct context *ctx, void *address, int cpu)
{
	void *bitmap_address = compute_first_page(ctx, cpu);
	int page_number = (address - (void *)bitmap_address) / (PAGE_SIZE) ;
        int offset = page_number / 8;
	uint8_t bit;
	if (offset)
		bit = 1 << (offset * 8 - page_number);
	else
		bit = 1 << page_number;

	printf("%d check %x\n" , offset, bit);
        if (((uint8_t *) bitmap_address)[offset] & bit)
		return 1;
	return 0;
}

int add_to_free_list(struct context *ctx, void *address)
{
	int cpu = find_cpu(ctx, address);
	if(isAllocated(ctx, address, cpu))
	{
		struct free_list *temp = malloc(sizeof(struct free_list));
		temp->start_address = address;
		temp->next = ctx->percpu_freelist[cpu];
		ctx->percpu_freelist[cpu] = temp;
		return 1;
	}
	return -1;	
}

int isValid(struct context *ctx, void * address)
{
	if (address != NULL)
		if (address >= ctx->start_address && address <= ctx->end_address)
			return 1;
	return 0;
}


int free_page(struct context *ctx, void *address)
{
	if (isValid(ctx, address))
		return add_to_free_list(ctx, address);	

}


