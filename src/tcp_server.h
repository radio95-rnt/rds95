#pragma once

#include <stdint.h>
#include <stddef.h>

int init_tcp_server(uint16_t port);
void accept_tcp_clients(void);
void send_to_tcp_clients(const char *data, size_t len);
int is_tcp_server_running(void);
void close_tcp_server(void);