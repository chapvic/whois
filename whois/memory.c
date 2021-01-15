#include "whois.h"

void* __cdecl mallocz(size_t size) {
	void* ptr = NULL;
	if (size) {
		ptr = malloc(size);
		if (ptr) memset(ptr, 0, size);
	}
	return ptr;
}

void* __cdecl bcopy(void* src, size_t size) {
	void* ptr = NULL;
	if (src && size) {
		ptr = malloc(size);
		if (ptr) memcpy(ptr, src, size);
	}
	return ptr;
}

char* __cdecl concat_s(char* str1, char* str2, char sep) {
	char* str = NULL, * p = NULL;
	size_t len1 = 0, len2 = 0, len = 0;
	if (str1) len1 = strlen(str1);
	if (str2) len2 = strlen(str2);
	len = len1 + len2;
	if (len) {
		len += sep ? 2 : 1;
		str = mallocz(len);
		if (str) {
			p = str;
			if (len1) {
				memcpy(p, str1, len1);
				p += len1;
			}
			if (sep) *p++ = sep;
			if (len2) {
				memcpy(p, str2, len2);
			}
		}
	}
	return str;
}

char* __cdecl concat(char* str1, char* str2) {
	return concat_s(str1, str2, 0);
}
