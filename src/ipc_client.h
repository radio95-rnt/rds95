#include <stdint.h>
#include "rds.h" // for RDSEncoder, get_rds_bits, BITS_PER_GROUP

typedef struct {
	int fd;
	uint8_t prev_output[4]; // per-stream differential state
} IPC_Client;

int ipc_connect(IPC_Client *client, const char *socket_path);
void ipc_close(IPC_Client *client);
int ipc_send_bits(IPC_Client *client, RDSEncoder *enc, uint8_t stream);