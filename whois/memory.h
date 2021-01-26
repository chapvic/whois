#ifndef MEMORY_H
#define MEMORY_H

#include <windows.h>

#undef malloc
#define malloc(s) HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(s))
#undef realloc
#define realloc(p,s) HeapReAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(p),(s))
#undef free
#define free(p) HeapFree(GetProcessHeap(),0,(p))
#undef msize
#define msize(p) HeapSize(GetProcessHeap(),0,(p))

void* __cdecl mallocz(size_t size);
void* __cdecl bcopy(void* src, size_t size);
char* __cdecl concat_s(char* str1, char* str2, char sep);
char* __cdecl concat(char* str1, char* str2);

#endif // !MEMORY_H

