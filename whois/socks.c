#include "whois.h"

void sock_close(sockopt_ptr so) {
	if (so
		&&sock_check_flag(so, SOCK_STATE_OPENED)
		&& !sock_check_flag(so, SOCK_STATE_CLOSED)) {
		closesocket(so->socket);
		so->socket = INVALID_SOCKET;
		so->state |= SOCK_STATE_CLOSED;
	}
}

bool sock_send(sockopt_ptr opt) {
	WSADATA wsaData;
	int status;
	fd_set fd;
	struct timeval send_tv, recv_tv;
	struct addrinfo addr, * result = NULL, * ptr = NULL;
	bool retval = false;

	if (!opt || opt->state != SOCK_STATE_INIT) return false;
	opt->socket = INVALID_SOCKET;
	opt->state = SOCK_STATE_OPENING;

	// Call socket opening handler
	if (opt->on_sock_open) opt->on_sock_open(opt);

	// Saving timeouts
	if (opt->send_timeout == 0) opt->send_timeout = SOCK_WAIT_TIMEOUT;
	send_tv.tv_sec = opt->send_timeout;
	send_tv.tv_usec = opt->send_timeout * 1000;
	if (opt->recv_timeout == 0) opt->recv_timeout = SOCK_WAIT_TIMEOUT;
	recv_tv.tv_sec = opt->recv_timeout;
	recv_tv.tv_usec = opt->recv_timeout * 1000;

	// Initializing Winsock
	status = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (status != 0) {
		printf("WSAStartup failed with error: %d\n", status);
		return false;
	}

	// Configure socket
	memset(&addr, 0, sizeof(addr));
	addr.ai_family = AF_INET;
	addr.ai_socktype = SOCK_STREAM;
	addr.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	status = getaddrinfo(opt->server, opt->port, &addr, &result);
	if (status != 0) {
		printf("getaddrinfo failed with error: %d\n", status);
		goto Exit;
	}

	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		// Create a SOCKET for connecting to server
		opt->socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (opt->socket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			goto Exit;
		}
		// Setup blocking mode
		status = ioctlsocket(opt->socket, FIONBIO, (unsigned long *)&opt->non_blocking);
		if (status != NO_ERROR) {
			printf("ioctlsocket failed with error: %ld\n", status);
			sock_close(opt);
			goto Exit;
		}
		// Connect to server.
		status = connect(opt->socket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (opt->non_blocking) {
			if (status == EWOULDBLOCK) {
				sock_close(opt);
				opt->state = opt->state & ~SOCK_STATE_CLOSED;
				continue;
			}
		}
		else {
			if (status != NO_ERROR) {
				sock_close(opt);
				opt->state = opt->state & ~SOCK_STATE_CLOSED;
				continue;
			}
		}
		break;
	}
	// Cleanup address info pointer
	freeaddrinfo(result);

	// Wait for socket ready
	FD_ZERO(&fd);
	FD_SET(opt->socket, &fd);
	int ready = select(opt->socket, NULL, &fd, NULL, &send_tv);
	if (ready < 1) {
		printf("select failed with error: %d\n", WSAGetLastError());
		sock_close(opt);
		goto Exit;
	}

	if (opt->socket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		goto Exit;
	}

	// Send an initial buffer
	status = send(opt->socket, opt->query, (int)strlen(opt->query), 0);
	if (status == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		sock_close(opt);
		goto Exit;
	}

	FD_ZERO(&fd);
	FD_SET(opt->socket, &fd);
	ready = select(opt->socket, &fd, NULL, NULL, &recv_tv);
	if (ready < 1) {
		printf("select failed with error: %d\n", WSAGetLastError());
		sock_close(opt);
		goto Exit;
	}

	// Receive until the peer closes the connection
	do {
		memset(opt->recvbuf, 0, SOCK_RECV_BUFFER + 1);
		status = recv(opt->socket, opt->recvbuf, SOCK_RECV_BUFFER, 0);
		if (status > 0) {
			fprintf_s(stdout, "%s", opt->recvbuf);
		}
		else if (status < 0) {
			printf("recv failed with error: %d\n", WSAGetLastError());
		}
	} while (status > 0);

	// Shutdown
	status = shutdown(opt->socket, SD_BOTH);
	if (status == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		sock_close(opt);
		goto Exit;
	}

	// Successfully exiting
	sock_close(opt);
	retval = true;

Exit:
	WSACleanup();
	return retval;
}

