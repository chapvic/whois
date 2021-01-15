#include "whois.h"

array_ptr __cdecl explode(char* str, char sep) {
	array_ptr arr;
	size_t len, cnt;
	char* start, * end;
	if (!str || !sep) return NULL;
	len = strlen(str);
	cnt = 1;
	for (size_t i = 0; i < len; i++) {
		if (str[i] == sep) ++cnt;
	}
	arr = malloc(sizeof(array_t));
	arr->items = mallocz(sizeof(char*) * cnt);
	arr->size = cnt;
	start = str;
	end = str;
	cnt = 0;
	while (true) {
		if (*end == sep || !*end) {
			len = end - start + 1;
			arr->items[cnt] = mallocz(len);
			if (arr->items[cnt]) memcpy(arr->items[cnt], start, len - 1);
			start += len;
			++cnt;
		}
		if (!*end) break;
		++end;
	}
	return arr;
}

char* __cdecl implode(array_ptr arr, char sep) {
	char* retval, * ptr, * end;
	size_t i, len;
	if (!arr) return NULL;
	len = arr->size;
	for (i = 0; i < arr->size; i++)
		len += strlen(arr->items[i]);
	retval = mallocz(len);
	if (retval) {
		ptr = retval;
		end = retval + len - 1;
		for (i = 0; i < arr->size; i++) {
			len = strlen(arr->items[i]);
			ptr = (char *)memcpy(ptr, arr->items[i], len) + len;
			if (ptr < end) *ptr++ = sep;
		}
	}
	return retval;
}

array_ptr __cdecl array_new(size_t size) {
	if (!size) return NULL;
	array_ptr arr = malloc(sizeof(array_t));
	if (arr) {
		arr->size = size;
		arr->items = malloc(sizeof(char*) * size);
		for (size_t i = 0; i < size; i++) {
			arr->items[i] = mallocz(1);
		}
	}
	return arr;
}

void __cdecl array_free(array_ptr arr) {
	if (arr && arr->size) {
		for (size_t i = 0; i < arr->size; i++) {
			free(arr->items[i]);
		}
		free(arr->items);
		free(arr);
	}
}

bool __cdecl array_set(array_ptr arr, size_t index, char* value) {
	if (arr && index < arr->size) {
		size_t len = strlen(value);
		arr->items[index] = realloc(arr->items[index], len + 1);
		if (arr->items[index]) {
			memcpy(arr->items[index], value, len + 1);
			return true;
		}
	}
	return false;
}
