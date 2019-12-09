#define PAGE_SIZE 4 * 1024
struct free_list
{
	struct free_list *next;
	void *start_address;
};

struct context
{
	void *start_address;
	void *end_address;
	struct free_list **percpu_freelist;
	int n_cpu;
	int n_pages;
};


struct context* register_address(void *addr, int n_cpu, int n_pages, int total_buf);
void* allocate_page(struct context *ctx, int cpu);
int free_page(struct context *ctx, void *address);
void dump(struct context *ctx);
