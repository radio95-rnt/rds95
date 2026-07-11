#include "ipc_client.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

int ipc_connect(IPC_Client *client, const char *socket_path) {
	client->fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (client->fd < 0) { perror("fm95 ipc: socket"); return -1; }

	struct sockaddr_un addr = { .sun_family = AF_UNIX };
	strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

	if (connect(client->fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("fm95 ipc: connect");
		close(client->fd);
		client->fd = -1;
		return -1;
	}
	memset(client->prev_output, 0, sizeof(client->prev_output));
	return 0;
}

void ipc_close(IPC_Client *client) {
	if (client->fd >= 0) close(client->fd);
	client->fd = -1;
}

// Pulls one group's worth of bits (BITS_PER_GROUP), differentially encodes,
// packs 8-to-a-byte, sends as opcode 112 with the stream index.
int ipc_send_bits(IPC_Client *client, RDSEncoder *enc, uint8_t stream) {
	if (client->fd < 0) return -1;

	uint8_t raw_bits[BITS_PER_GROUP];
	get_rds_bits(enc, raw_bits, stream);

	uint8_t packed[(BITS_PER_GROUP + 7) / 8];
	memset(packed, 0, sizeof(packed));

	uint8_t prev = client->prev_output[stream];
	for (int i = 0; i < BITS_PER_GROUP; i++) {
		uint8_t cur = prev ^ raw_bits[i]; // the differential step, moved here
		prev = cur;
		if (cur) packed[i / 8] |= (0x80 >> (i % 8));
	}
	client->prev_output[stream] = prev;

	uint8_t msg[2 + sizeof(packed)];
	msg[0] = 112;
	msg[1] = stream;
	memcpy(msg + 2, packed, sizeof(packed));

	ssize_t sent = send(client->fd, msg, sizeof(msg), 0);
	if (sent != (ssize_t)sizeof(msg)) {
		fprintf(stderr, "fm95 ipc: short/failed send\n");
		return -1;
	}

	uint8_t reply;
	if (recv(client->fd, &reply, 1, 0) <= 0) {
		fprintf(stderr, "fm95 ipc: no reply / disconnected\n");
		return -1;
	}
	if (reply == 2) fprintf(stderr, "fm95 reported bitring overrun on stream %d\n", stream);
	else if (reply == 1) fprintf(stderr, "fm95 rejected message (unknown opcode?)\n");

	return 0;
}