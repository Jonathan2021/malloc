#include <stddef.h>
#include <sys/mman.h>
#include <pthread.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#define MEM 0x100000000000
#define ALIGN 16

struct chunk
{
  struct chunk *next;
  struct chunk *prev;
  size_t size;
  char free;
};


static void *allocate(void)
{
    return mmap(NULL, MEM, PROT_READ | PROT_WRITE |\
    PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
}

static size_t word_align(size_t n)
{
	return ((n - 1) | (ALIGN - 1)) + 1;
}

static void add_alloc(struct chunk *c, size_t size)
{
    size_t s = word_align(size);
    struct chunk *new = (struct chunk *)((char *)(c + 1) + s);
    c->free = 0;
    new->next = c->next;
    if(c->next)
        c->next->prev = new;
    c->next = new;
    new->prev = c;
    new->free = 1;
    new->size = c->size - (sizeof(struct chunk) + s);
    c->size = s;
}

/*zerofill(ptr, len): write len 0 bytes at the address ptr */
static void zerofill(void *ptr, size_t len)
{
	memset(ptr, 0, len);
}

/* wordcpy(dst, src, len) copy len bytes from src to dst */

static void wordcpy(void *dst, void *src, size_t len)
{
	memcpy(dst, src, len);
}

//kind of global variable
static struct chunk *base = NULL;

static struct chunk* get_base(void) 
{
  if (base == NULL) //only at first call of get_base
  {
	void *new_mem = allocate();
	if (new_mem == (void*)(-1))
		return NULL;
	base = new_mem;
	base->size =  MEM - sizeof(struct chunk);
	base->free = 1;
	base->next = NULL;
	base->prev = NULL;
  }
  return base;
}

//Sanity checks could be usefull later so I commented them out

void my_assert(int pass, int code) {
  if (!pass)
    _exit(code);
}

void sanity_check(void) {
  struct chunk *this, *prev = 0;

  for (this = base; this; prev = this, this = this->next) {
    my_assert(!((intptr_t)this & (ALIGN - 1)), 1);
    my_assert(!(this->size & (ALIGN - 1)), 2);
    my_assert(this->prev == prev, 3);
    if (this->next) {
      my_assert((char *)this->next == ((char*)this) + \
      (this->size) + sizeof(struct chunk), 4);
      my_assert(this->next > this, 5);
    }
  }
}

//Find the first free chunk big enough
static struct chunk* find_chunk(size_t size)
{
	struct chunk *heap_ptr = get_base();
	if (heap_ptr == NULL)
	{
		return NULL;
	}
	while (heap_ptr->next)
	{
		if (heap_ptr->free)
		{
			if (heap_ptr->size >= size + sizeof(struct chunk))
				return heap_ptr;
		}
		heap_ptr = heap_ptr->next;
	}
	return heap_ptr;		  
}

//Get the chunk (containing metadata) from void *
static struct chunk* get_chunk(void *p)
{
	struct chunk *chunk;

	if (p == NULL)
		return NULL;
	if ((intptr_t)(p) & (ALIGN - 1))
		return NULL;
	if ((uintptr_t)p < sizeof(struct chunk))
		return NULL;
	chunk = (struct chunk *)(p) - 1;
	if (chunk < get_base())
		return NULL;
	return chunk;
}

//malloc at first big enough space and splits it
__attribute__((visibility("default")))
void *malloc(size_t __attribute__((unused)) size)
{
        if(size == 0)
            return NULL;
	struct chunk *c;
	size_t s;
	s = word_align(size);
	c = find_chunk(s);
	if (!c)
		return NULL;
        add_alloc(c, s);
        sanity_check();
        return c + 1;
}

//same as malloc and initializes at 0
__attribute__((visibility("default")))
void *calloc(size_t __attribute__((unused)) nb,
             size_t __attribute__((unused)) size)
{
	void *p;
	p = malloc(nb * size);
	if (!p)
		return NULL;
	zerofill(p, nb * size);	
        sanity_check();
	return p;
}

//free void *
__attribute__((visibility("default")))
void free(void __attribute__((unused)) *p)
{
        if(p==0)
            return;
	struct chunk *c = get_chunk(p);
	if (!c)
                return;

	    c->free = 1;
            if(c->next && c->next->free)
            {
                c->size += c->next->size + sizeof(struct chunk);
                c->next = c->next->next;
                if (c->next)
                        c->next->prev = c;
            }
            if(c->prev && c->prev->free)
            {
                c->prev->size += c->size + sizeof(struct chunk);
                c->prev->next = c->next;
                if (c->next)
                        c->next->prev = c->prev;
            }
        sanity_check();
}

//realloc void *
__attribute__((visibility("default")))
void *realloc(void __attribute__((unused)) *p,
             size_t __attribute__((unused)) size)
{
        if(p == NULL)
            return malloc(size);
        if(size == 0)
        {
            free(p);
            return NULL;
        }
        size_t s = word_align(size);
	struct chunk *c = get_chunk(p);
        if(!c)
            return NULL;
	if (s <= c->size) //fits in same box
        {
            if(s + sizeof(struct chunk) < c->size) {
                add_alloc(c, s);
                sanity_check();
            }
            return p;
        } //had an opti but didn't work out well so just malloc it
        void *new_p = malloc(size);		
	wordcpy(new_p, p, c->size);
	free(p); //free the old void *
	return new_p;
}
