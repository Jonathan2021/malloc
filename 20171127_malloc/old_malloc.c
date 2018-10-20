#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static size_t word_align(size_t n)
{
	return ((n-1)|(sizeof(size_t)-1))+1;
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

struct chunk {
  struct chunk *next, *prev;
  size_t        size;
  long          free;
  void         *data;
};

static struct chunk* get_base(void) 
{
  static struct chunk *base = NULL;

  if (base == NULL) 
  {
	void *new_mem = sbrk(sizeof(struct chunk));
	if (new_mem == (void*)(-1))
		return NULL;
	base = new_mem;
	base->size = 0;
	base->free = 0;
	base->data = base + 1;
	base->next = NULL;
	base->prev = NULL;	    		    
  }
  return base;
}

/*
 * extend_heap(last, size) extends the heap with a new chunk containing a data block of size bytes.
 * Return 1 in case of success, and return 0 if sbrk(2) fails.
 */

static int extend_heap(struct chunk *last, size_t size)
{
	struct chunk *new_chunk = sbrk(size + sizeof(struct chunk));
	if ((void*)new_chunk == (void*)(-1))
		return 0;
	last->next = new_chunk;
	new_chunk->size = size;
	new_chunk->free = 1;
	new_chunk->data = new_chunk + 1;
	new_chunk->next = NULL;
	return 1;
	
}

static struct chunk* find_chunk(size_t size)
{
	struct chunk *heap_ptr = get_base();
	if (heap_ptr == NULL)
	{
		return NULL;
	}
	while (heap_ptr->next)
	{
		if (heap_ptr->next->free)
		{
			if (heap_ptr->next->size >= size)
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
	if (p < (void*)(get_base() + 2))
		return NULL;
	if (p >= sbrk(0))
		return NULL;

	chunk = p - sizeof(struct chunk);
	if (p != chunk->data)
		return NULL;

	return chunk;
}

void *malloc (size_t size)
{
	struct chunk *c;
	size_t s;
	s = word_align(size);
	c = find_chunk(s);
	if (!c)
		return NULL;
	if(!(c->next))
	{
		if (!(extend_heap(c, s)))
			return NULL;
	}
	c->next->free = 0;			
	return c->next->data;
}

void *calloc (size_t nb, size_t size)
{
	void *p;
	p = malloc(nb * size);
	if (!p)
		return NULL;
	zerofill(p, word_align(size*nb));	
	return p;
}

void free(void *p)
{
	struct chunk *c = get_chunk(p);
	if (c)
	{
		assert(!c->free);
		c->free = 1;
	}
}

void *realloc(void *p, size_t size)
{
	struct chunk *c = get_chunk(p);
	if (size <= c->size)
		return p;

	void *new_p = malloc(size);		
	wordcpy(new_p, p, c->size);
	free(p);
	return new_p;
}
