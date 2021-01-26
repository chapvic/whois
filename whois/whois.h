#ifndef WHOIS_H
#define WHOIS_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#include <libxml/xmlreader.h>
#include <libxml/xpath.h>

#include "memory.h"

#undef _tputs
#ifdef _MBCS
#define _tputs puts
#else
#define _tputs _putws
#endif // UNICODE

// Maximum XML path length for XPath Query
#define XMLDB_PATH_LEN 512

extern char* error_str;
extern int error_val;

// ---------------
// array.c
// ---------------

typedef struct _array_t {
	size_t size;
	char** items;
} array_t, * array_ptr;

array_ptr __cdecl explode(char* str, char sep);
char* __cdecl implode(array_ptr arr, char sep);
array_ptr __cdecl array_new(size_t size);
void __cdecl array_free(array_ptr arr);
bool __cdecl array_set(array_ptr arr, size_t index, char* value);

// ---------------
// socks.c
// ---------------

#ifndef RECV_MAX_BUFFER
#define RECV_MAX_BUFFER 256
#endif // !RECV_MAX_BUFFER

#ifndef SOCK_WAIT_TIMEOUT
#define SOCK_WAIT_TIMEOUT 10
#endif // !SOCK_WAIT_TIMEOUT

#ifndef SOCK_RECV_BUFFER
#define SOCK_RECV_BUFFER 512
#endif // !SOCK_RECV_BUFFER

typedef void(__cdecl* on_sock_state)(void* opt);
typedef void(__cdecl* on_sock_error)(void* opt, int err_type, int err_code);
typedef void(__cdecl* on_sock_recv)(void* buffer, size_t size);

typedef struct _sockopt {
	SOCKET socket;			// socket number
	bool non_blocking;		// non-blocking mode
	long state;				// socket state (do not change !!!)
	char* server;			// FQDN server name
	char* port;				// server port
	int send_timeout;		// send timeout is seconds
	int recv_timeout;		// receive timeout is seconds
	char* query;			// pointer to a query string
	char recvbuf[SOCK_RECV_BUFFER + 1]; // receive buffer
	on_sock_state on_sock_open;		// socket open handler
	on_sock_state on_sock_opened;	// socket opened handler
	on_sock_recv on_data;			// data receive handler
	on_sock_error on_error;			// socket error handler
} sockopt_t, * sockopt_ptr;

#ifndef SOCK_STATE_DEFINED
#define SOCK_STATE_DEFINED
#define SOCK_STATE_INIT       0x00000000 // no operation (initial state)
#define SOCK_STATE_OPENING    0x00000001 // opening socket
#define SOCK_STATE_OPENED     0x0000000F // socket opened
#define SOCK_STATE_CONNECTING 0x00000010 // connecting socket
#define SOCK_STATE_CONNECTED  0x000000F0 // socket connected
#define SOCK_STATE_SENDING    0x00000100 // sending query
#define SOCK_STATE_SENT       0x00000F00 // query sent
#define SOCK_STATE_RECEIVING  0x00001000 // receiving data
#define SOCK_STATE_RECEIVED   0x0000F000 // data received
#define SOCK_STATE_CLOSED     0x000F0000 // socket closed
#define SOCK_STATE_ERROR      0xF0000000 // error flag

#define sock_check_flag(s, flag)		(((s)->state)&(flag))==(flag)
#define sock_if_error(s)				sock_check_flag(s, SOCK_STATE_ERROR)
#define sock_if_success(s)				!sock_check_flag(s, SOCK_STATE_ERROR)
#endif // !SOCK_STATE_DEFINED

bool sock_send(sockopt_ptr opt);
void sock_close(sockopt_ptr opt);

// ---------------
// xmldb.c
// ---------------

#ifndef xmldb_ready
#define xmldb_ready (xpath != NULL)
#endif

typedef struct _xmldb_zone {
	char* zone;
	array_ptr whois_servers;
} xmldb_zone, * xmldb_zone_ptr;

typedef struct _xmldb_record {
	char* domain;
	size_t zones;
	xmldb_zone_ptr* list;
} xmldb_record, * xmldb_record_ptr;

bool __cdecl xmldb_load(void);
void __cdecl xmldb_free(void);
xmldb_record_ptr __cdecl xmldb_get_record(char* domain);
void __cdecl xmldb_free_record(xmldb_record_ptr rec);

// ---------------
// client.c
// ---------------

#define SOCK_SEND_BUFFER 128
#define SOCK_SELECT_TIMEOUT 5000

typedef struct _whois_t {
	size_t req;
	size_t timeout;
	char* domain;
	array_ptr servers;
	FILE* file;
	char* response;
	char query[SOCK_SEND_BUFFER + 1];
} whois_t, *whois_ptr;

whois_ptr __cdecl whois_open(char* domain, FILE* file);
bool __cdecl whois_request(whois_ptr whois);
void __cdecl whois_close(whois_ptr* whois);

#endif // !WHOIS_H
