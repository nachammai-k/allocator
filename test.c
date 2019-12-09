#include <stdio.h>
#include <stdlib.h>
#include "allocate.h"

void main()
{
	void *region = aligned_alloc(4 * 1024, 10 * 10 * 4*1024);
	struct context *ctx = register_address(region, 10, 10, 100*4*1024);
	void *addr = allocate_page(ctx, 1);
	void *addr1 = allocate_page(ctx, 1);
	void *addr2 = allocate_page(ctx, 1);
	int f = free_page(ctx, addr2);
	void *addr3 = allocate_page(ctx, 1);
	printf("%p %p %p %d %p", addr, addr1, addr2, f, addr3);

	addr = allocate_page(ctx, 1);
	addr1 = allocate_page(ctx, 1);
	addr2 = allocate_page(ctx, 1);
	f = free_page(ctx, addr2);
	addr3 = allocate_page(ctx, 2);
	f = free_page(ctx, addr3);
	printf("%p %p %p %d %p", addr, addr1, addr2, f, addr3);
	dump(ctx);

}
