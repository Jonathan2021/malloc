#include <sys/mman.h>
#include <stdio.h>

int main(void)
{
    void * map = mmap(NULL, 1000000000000, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
    if(map != (void *)(-1))
        puts("ok!");
    else
        perror("");
    return 0;
}
