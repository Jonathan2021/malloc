#include <stddef.h>
#include <sys/mman.h>
#include <pthread.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#define MEM 1000000000000
/*
__attribute__((visibility("default")))
void *malloc(size_t __attribute__((unused)) size)
{
	return NULL;
}

__attribute__((visibility("default")))
void free(void __attribute__((unused)) *ptr)
{
}


__attribute__((visibility("default")))
void *realloc(void __attribute__((unused)) *ptr,
             size_t __attribute__((unused)) size)
{
	return NULL;
}

__attribute__((visibility("default")))
void *calloc(size_t __attribute__((unused)) nmemb,
             size_t __attribute__((unused)) size)
{
	return NULL;
}



static size_t nb_block(size_t size)
{
    if(size == 0)
        return 1;
    return ((size - 1)/BLOCK) + 1;
}
*/
struct chunk
{
  struct chunk *next, *prev;
  size_t        size;
  char          free;
};

static void *allocate(void)
{
    return mmap(NULL, MEM, PROT_READ | PROT_WRITE |\
    PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
}

static size_t word_align(size_t n)
{
	return ((n-1)|(16-1))+1;
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
    //assert(c->size >= s + sizeof(struct chunk));
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


static struct chunk *base = NULL;

static struct chunk* get_base(void) 
{
  if (base == NULL) 
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

/*
 * extend_heap(last, size) extends the heap with a new chunk containing a data block of size bytes.
 * Return 1 in case of success, and return 0 if sbrk(2) fails.
 */
/*
static int extend_heap(struct chunk *last, size_t size)
{
        size_t real_size = get_block(size + sizeof(struct chunk)) * BLOCK;
	struct chunk *new_chunk = allocate(real_size); 
	if ((void*)new_chunk == (void*)(-1))
		return 0;
	last->next = new_chunk;
        new_chunk->prev = last;
	new_chunk->size = real_size - sizeof(struct chunk);
	new_chunk->free = 1;
	new_chunk->data = new_chunk + 1;
	new_chunk->next = NULL;
	return 1;
	
}
*/

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
			if (heap_ptr->size >= size)
				return heap_ptr;
		}
		heap_ptr = heap_ptr->next;
	}
	return heap_ptr;		  
}

static struct chunk* get_chunk(void *p)
{
	struct chunk *chunk;

	if (p == NULL)
		return NULL;
	if ((intptr_t)(p) & (sizeof(void*)-1))
		return NULL;
	if (p < (void*)(get_base() + 1))
		return NULL;
	//if (p >= sbrk(0))
	//	return NULL;

	chunk = (struct chunk *)(p) - 1;
	if (p != chunk + 1)
		return NULL;

	return chunk;
}

__attribute__((visibility("default")))
void *my_malloc(size_t __attribute__((unused)) size)
{
	struct chunk *c;
	size_t s;
	s = word_align(size);
	c = find_chunk(s);
	if (!c)
		return NULL;
	//if(!c->free || (c->size) < size + sizeof(struct chunk))
	//{
	//	if (!(extend_heap(c, s)))
	//		return NULL;
        //        c = c->next;
	//}
        add_alloc(c, s);
        assert(c >= base);
        return c + 1;
}

__attribute__((visibility("default")))
void *my_calloc(size_t __attribute__((unused)) nb,
             size_t __attribute__((unused)) size)
{
	void *p;
	p = my_malloc(nb * size);
	if (!p)
		return NULL;
	zerofill(p, word_align(size*nb));	
	return p;
}

__attribute__((visibility("default")))
void my_free(void __attribute__((unused)) *p)
{
	struct chunk *c = get_chunk(p);
        assert(c >= base);
	if (c)
	{
		c->free = 1;
                //printf("\nfree succeeded\n\n");
	}
}

__attribute__((visibility("default")))
void *my_realloc(void __attribute__((unused)) *p,
             size_t __attribute__((unused)) size)
{
        size_t s = word_align(size);
        //printf("size should be %lu (add_alloc)\n", s);
	struct chunk *c = get_chunk(p);
        struct chunk *tmp = c->next;
        if(!c)
            return NULL;
	if (s <= c->size)
        {
            if(s + sizeof(struct chunk) < c->size)
	    {
                add_alloc(c, s);
                tmp->prev = c->next;
            }
            return p;
        }
        size_t opti = c->size;
        while(tmp && tmp->free && opti < s)
        {
            opti += sizeof(struct chunk) + tmp->size;
            tmp = tmp->next;
        }
        if(opti >= s)
        {
            if(opti > s + sizeof(struct chunk))
            {
                add_alloc(c, s);
                c->next->size = (char *)tmp - (char *)(c->next + 1);
                c = c->next;
            }
            else
                c->size = opti;
            c->next = tmp;
            tmp->prev = c;
            return p;
        }
        void *new_p = my_malloc(size);		
	wordcpy(new_p, p, c->size);
	my_free(p);
	return new_p;
}

void sanity_check(void) {
  struct chunk *this, *prev = 0;

  for (this = base; this; prev = this, this = this->next) {
    assert(this->prev == prev);
    if (prev) {
      if(((char*)prev) + (prev->size) + sizeof(struct chunk) != (char *)this)
       {
           printf("previous = %p + prev->size = %lu + sizeof(chunk) = %lu == this = %p\nFAILED\n", (void *)prev, prev->size, sizeof(struct chunk), (void *)this);
           assert(0);
        }
    }
  }
}

int main(void)
{
    printf("sizeof struct chunk %ld\n", sizeof(struct chunk));
    int *int_p = my_malloc(5000 * sizeof(int));
    sanity_check();
    struct chunk *c = get_chunk(int_p);
    int_p[3] = 224;
    printf("free is %d\n", c->free);
    printf("size is %lu\n", c->size);
    printf("int_p[3] is %d\n", int_p[3]);
    printf("next->size is %lu\n", c->next->size);
    printf("next->free is %d\n", c->next->free);
    char *str = my_malloc(10);
    sanity_check();
    memcpy(str, "123456789", 10);
    printf("str is %s\n", str);
    printf("c->next->size: %lu\n", c->next->size);
    printf("c->next->next->size: %lu\n", c->next->next->size);
    printf("adress p_int before realloc %p\n", (void *)int_p);
    printf("size is %lu\n", c->size);
    int_p = my_realloc(int_p, 150 * sizeof(int));
    sanity_check();
    printf("adress p_int after 150 realloc %p\n", (void *)int_p);
    printf("size is %lu\n", c->size);
    int_p = my_realloc(int_p, 500 * sizeof(int));
    sanity_check();
    printf("adress p_int after 500 realloc %p\n", (void *)int_p);
    printf("size is %lu\n", c->size);
    int_p = my_realloc(int_p, 8000 * sizeof(int));
    sanity_check();
    printf("adress p_int after 8000 realloc %p\n", (void *)int_p);
    printf("size is %lu\n", c->size);
    printf("c->free %d\n", c->free);
    c = get_chunk(str);
    printf("size after  str %lu\n", c->next->size);
    printf("all done\n");
}
