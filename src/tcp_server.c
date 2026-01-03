#include "tcp_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>

#define MAX_TCP_CLIENTS 10

typedef struct {
	int socket;
	struct sockaddr_in address;
	uint8_t active;
} TCPClient;

typedef struct {
	int server_socket;
	uint16_t port;
	TCPClient clients[MAX_TCP_CLIENTS];
	pthread_mutex_t clients_mutex;
	uint8_t running;
} TCPServer;

static TCPServer tcp_server = {
	.server_socket = -1,
	.port = 0,
	.running = 0
};

int init_tcp_server(uint16_t port) {
	tcp_server.server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (tcp_server.server_socket < 0) {
		fprintf(stderr, "Error: Could not create TCP socket\n");
		return -1;
	}

	int opt = 1;
	if (setsockopt(tcp_server.server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		fprintf(stderr, "Error: Could not set socket options\n");
		close(tcp_server.server_socket);
		return -1;
	}

	int flags = fcntl(tcp_server.server_socket, F_GETFL, 0);
	fcntl(tcp_server.server_socket, F_SETFL, flags | O_NONBLOCK);

	struct sockaddr_in server_addr = {0};
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);

	if (bind(tcp_server.server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		fprintf(stderr, "Error: Could not bind TCP socket to port %d\n", port);
		close(tcp_server.server_socket);
		return -1;
	}

	if (listen(tcp_server.server_socket, 5) < 0) {
		fprintf(stderr, "Error: Could not listen on TCP socket\n");
		close(tcp_server.server_socket);
		return -1;
	}

	tcp_server.port = port;
	tcp_server.running = 1;
	pthread_mutex_init(&tcp_server.clients_mutex, NULL);

	for (int i = 0; i < MAX_TCP_CLIENTS; i++) {
		tcp_server.clients[i].socket = -1;
		tcp_server.clients[i].active = 0;
	}

	printf("TCP server listening on port %d\n", port);
	return 0;
}

void accept_tcp_clients(void) {
	if (!tcp_server.running) return;

	struct sockaddr_in client_addr;
	socklen_t addr_len = sizeof(client_addr);
	
	int client_socket = accept(tcp_server.server_socket, (struct sockaddr *)&client_addr, &addr_len);
	if (client_socket < 0) {
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			fprintf(stderr, "Error accepting client: %s\n", strerror(errno));
		}
		return;
	}

	int flags = fcntl(client_socket, F_GETFL, 0);
	fcntl(client_socket, F_SETFL, flags | O_NONBLOCK);

	pthread_mutex_lock(&tcp_server.clients_mutex);
	
	int slot = -1;
	for (int i = 0; i < MAX_TCP_CLIENTS; i++) {
		if (!tcp_server.clients[i].active) {
			slot = i;
			break;
		}
	}

	if (slot >= 0) {
		tcp_server.clients[slot].socket = client_socket;
		tcp_server.clients[slot].address = client_addr;
		tcp_server.clients[slot].active = 1;
		printf("Client connected from %s (slot %d)\n", inet_ntoa(client_addr.sin_addr), slot);
	} else {
		fprintf(stderr, "Max clients reached, rejecting connection\n");
		close(client_socket);
	}

	pthread_mutex_unlock(&tcp_server.clients_mutex);
}

void send_to_tcp_clients(const char *data, size_t len) {
	if (!tcp_server.running) return;

	pthread_mutex_lock(&tcp_server.clients_mutex);

	for (int i = 0; i < MAX_TCP_CLIENTS; i++) {
		if (!tcp_server.clients[i].active) continue;

		ssize_t sent = send(tcp_server.clients[i].socket, data, len, MSG_NOSIGNAL);
		if (sent < 0) {
			if (errno != EAGAIN && errno != EWOULDBLOCK) {
				printf("Client disconnected (slot %d)\n", i);
				close(tcp_server.clients[i].socket);
				tcp_server.clients[i].socket = -1;
				tcp_server.clients[i].active = 0;
			}
		}
	}

	pthread_mutex_unlock(&tcp_server.clients_mutex);
}

int is_tcp_server_running(void) {
	return tcp_server.running;
}

void close_tcp_server(void) {
	if (!tcp_server.running) return;

	tcp_server.running = 0;

	pthread_mutex_lock(&tcp_server.clients_mutex);
	for (int i = 0; i < MAX_TCP_CLIENTS; i++) {
		if (tcp_server.clients[i].active) {
			close(tcp_server.clients[i].socket);
			tcp_server.clients[i].socket = -1;
			tcp_server.clients[i].active = 0;
		}
	}
	pthread_mutex_unlock(&tcp_server.clients_mutex);

	if (tcp_server.server_socket >= 0) {
		close(tcp_server.server_socket);
		tcp_server.server_socket = -1;
	}

	pthread_mutex_destroy(&tcp_server.clients_mutex);
	printf("TCP server closed\n");
}