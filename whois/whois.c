#include "whois.h"

static bool __cdecl whois_query(whois_ptr whois) {
	static char recvbuf[SOCK_RECV_BUFFER + 1];
	int status;
	WSADATA wsaData;
	SOCKET ConnectSocket;
	struct addrinfo addr, * result = NULL, * ptr = NULL;
	bool retval = false;
	fd_set readSet;

	struct timeval sendTimeout;
	sendTimeout.tv_sec = 3;
	sendTimeout.tv_usec = (whois->timeout % 1000) * 1000;

	struct timeval selTimeout;
	selTimeout.tv_sec = whois->timeout / 1000;
	selTimeout.tv_usec = (whois->timeout % 1000) * 1000;

	if (whois->req < whois->servers->size) {

		// Initialize Winsock
		status = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (status != 0) {
			printf("WSAStartup failed with error: %d\n", status);
			return retval;
		}

		memset(&addr, 0, sizeof(addr));
		addr.ai_family = AF_INET;
		addr.ai_socktype = SOCK_STREAM;
		addr.ai_protocol = IPPROTO_TCP;

		// Resolve the server address and port
		status = getaddrinfo(whois->servers->items[whois->req], "43", &addr, &result);
//		status = getaddrinfo("whois.net.ru", "43", &addr, &result);
		if (status != 0) {
			printf("getaddrinfo failed with error: %d\n", status);
			goto Exit;
		}

		// Attempt to connect to an address until one succeeds
		ConnectSocket = INVALID_SOCKET;
		for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
			// Create a SOCKET for connecting to server
			ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
			if (ConnectSocket == INVALID_SOCKET) {
				printf("socket failed with error: %ld\n", WSAGetLastError());
				goto Exit;
			}

			unsigned long iMode = 1;	//(0 = blocking mode, 1 = non-blocking mode)
			status = ioctlsocket(ConnectSocket, FIONBIO, &iMode);
			if (status != NO_ERROR) {
				printf("ioctlsocket failed with error: %ld\n", status);
				closesocket(ConnectSocket);
				goto Exit;
			}

			// Connect to server.
			status = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
			if (status == EWOULDBLOCK) {
				closesocket(ConnectSocket);
				ConnectSocket = INVALID_SOCKET;
				continue;
			}
			break;

		}
		freeaddrinfo(result);

		// Wait for socket ready
		FD_ZERO(&readSet);
		FD_SET(ConnectSocket, &readSet);
		int ready = select(ConnectSocket, NULL, &readSet, NULL, &sendTimeout);
		if (ready < 1) {
			printf("select failed with error: %d\n", WSAGetLastError());
			closesocket(ConnectSocket);
			goto Exit;
		}

		if (ConnectSocket == INVALID_SOCKET) {
			printf("Unable to connect to server!\n");
			goto Exit;
		}

		// Send an initial buffer
		status = send(ConnectSocket, whois->query, (int)strlen(whois->query), 0);
		if (status == SOCKET_ERROR) {
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(ConnectSocket);
			goto Exit;
		}

		FD_ZERO(&readSet);
		FD_SET(ConnectSocket, &readSet);
		ready = select(ConnectSocket, &readSet, NULL, NULL, &selTimeout);
		if (ready < 1) {
			printf("select failed with error: %d\n", WSAGetLastError());
			closesocket(ConnectSocket);
			goto Exit;
		}

		// shutdown the connection since no more data will be sent
		/*
		status = shutdown(ConnectSocket, SD_SEND);
		if (status == SOCKET_ERROR) {
			printf("shutdown failed with error: %d\n", WSAGetLastError());
			closesocket(ConnectSocket);
			goto Exit;
		}
		*/

		// Receive until the peer closes the connection
		do {
			memset(recvbuf, 0, SOCK_RECV_BUFFER + 1);
			status = recv(ConnectSocket, recvbuf, SOCK_RECV_BUFFER, 0);
			if (status > 0) {
				fprintf_s(whois->file, "%s", recvbuf);
			}
			else if (status < 0) {
				printf("recv failed with error: %d\n", WSAGetLastError());
			}
		} while (status > 0);
		closesocket(ConnectSocket);
		retval = true;

	Exit:
		WSACleanup();
		whois->req++;
	}
	return retval;
}


whois_ptr __cdecl whois_open(char* domain, FILE* file) {
	size_t i;
	whois_ptr whois;
	array_ptr srv;
	xmldb_record_ptr rec = xmldb_get_record(domain);
	if (!rec) return NULL;
	whois = mallocz(sizeof(whois_t));
	if (whois) {
		i = strlen(domain) + 1;
		whois->domain = malloc(i);
		memcpy(whois->domain, domain, i);
		memset(whois->query, 0, SOCK_SEND_BUFFER + 1);
		sprintf_s(whois->query, SOCK_SEND_BUFFER, "%s\r\n", domain);
		srv = (rec->zones > 1) ? srv = rec->list[1]->whois_servers : rec->list[0]->whois_servers;
		whois->servers = array_new(srv->size);
		whois->file = file ? file : stdout;
		whois->timeout = SOCK_SELECT_TIMEOUT;
		if (!whois->servers) goto Error;
		for (i = 0; i < srv->size; i++)
			array_set(whois->servers, i, srv->items[i]);
	}
	return whois;
Error:
	free(whois);
	return NULL;
}

bool __cdecl whois_request(whois_ptr whois) {
	if (whois && whois->req < whois->servers->size) {
		do {
			printf("WHOIS '%s' (server: %s)\n\n",
				whois->domain, whois->servers->items[whois->req]);
			whois_query(whois);
			puts("");
		} while (whois->req < whois->servers->size);
		return true;
	}
	else return false;
}

void __cdecl whois_close(whois_ptr* whois) {
	if (whois && *whois) {
		if ((*whois)->domain) free((*whois)->domain);
		if ((*whois)->response) free((*whois)->response);
		array_free((*whois)->servers);
		free(*whois);
		*whois = NULL;
	}
}

array_ptr __cdecl whois_root_servers(char *domain) {
	static char std_root_template[] = "%s.whois-servers.net";
	static char def_root_template[] = "whois.%s.%s";
	array_ptr root = NULL;
	if (domain) {

	}
	return root;
}
