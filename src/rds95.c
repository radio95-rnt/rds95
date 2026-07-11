#include "common.h"
#include <signal.h>
#include <getopt.h>
#include <pthread.h>
#include "../inih/ini.h"

#include "rds.h"
#include "ipc_client.h"
#include "fs.h"
#include "udp_server.h"
#include "tcp_server.h"
#include "lib.h"

#define DEFAULT_CONFIG_PATH "/etc/rds95.conf"
#define DEFAULT_STREAMS 1
#define MAX_STREAMS 4

#define NUM_MPX_FRAMES	128

static uint8_t stop_rds = 0;

static void stop() {
	printf("Received an stopping signal\n");
	stop_rds = 1;
}

static void *udp_server_worker() {
	while (!stop_rds) {
		poll_udp_server();
		msleep(READ_TIMEOUT_MS);
	}

	close_udp_server();
	pthread_exit(NULL);
}

static inline void show_help(char *name) {
	printf(
		"\n"
		"Usage: %s [options]\n"
		"\n"
		"\t-c,--config\tSet the config path [default: %s]\n"
		"\t-a,--asciig\tEnable asciig mode where the RDS IPC will be disabled and encoded RDS ASCII G data will be sent from stderr\n"
		"\n",
		name,
		DEFAULT_CONFIG_PATH
	);
}

typedef struct {
	uint16_t udp_port;
	uint16_t tcp_port;
	uint8_t num_streams : 3;
	uint8_t asciig : 1;
} RDS95_Config;

static int config_handler(void* user, const char* section, const char* name, const char* value) {
    RDS95_Config* config = (RDS95_Config*)user;

    #define MATCH(s, n) (strcmp(section, s) == 0 && strcmp(name, n) == 0)

    if (MATCH("rds95", "udp_port")) config->udp_port = (uint16_t)atoi(value);
    else if (MATCH("rds95", "tcp_port")) config->tcp_port = (uint16_t)atoi(value);
	else if (MATCH("rds95", "streams")) {
        int streams = atoi(value);
        if (streams > MAX_STREAMS || streams == 0) return 0;
        config->num_streams = (uint8_t)streams;
    } else if (MATCH("rds95", "asciig")) {
		int asciig = atoi(value);
        if (asciig > 1 || asciig < 0) return 0;
        config->asciig = (uint8_t)asciig;
    } else return 0;
    return 1;
}

int main(int argc, char **argv) {
	printf("rds95 (a RDS encoder by radio95) version %s\n", VERSION);

	char config_path[64] = DEFAULT_CONFIG_PATH;
	RDS95_Config config = {
		.udp_port = 0,
		.tcp_port = 0,
		.num_streams = DEFAULT_STREAMS,
		.asciig = 0
	};

	pthread_attr_t attr;
	pthread_t udp_server_thread;

	const char	*short_opt = "c:ah";

	struct option long_opt[] =
	{
		{"config", required_argument, NULL, 'c'},
		{"asciig", no_argument, NULL, 'a'},
		{"help", no_argument, NULL, 'h'},
		{0, 0, 0, 0}
	};

	int opt;
	while((opt = getopt_long(argc, argv, short_opt, long_opt, NULL)) != -1) {
		switch (opt) {
			case 'c':
				memcpy(config_path, optarg, 62);
				config_path[48] = '\0';
				break;
			case 'a':
				config.asciig = 1;
				break;
			case 'h':
				show_help(argv[0]);
				return 0;
			default:
				show_help(argv[0]);
				return 1;
		}
	}

	int res = ini_parse(config_path, config_handler, &config);
	if(res != 0) {
		fprintf(stderr, "Error: Could not read ini config, error code as return code.\n");
		return res;
	}

	printf("Using %d RDS stream(s)\n", config.num_streams);

	pthread_attr_init(&attr);

	struct sigaction sa_stop;
	sa_stop.sa_handler = stop;
	sigemptyset(&sa_stop.sa_mask);
	sa_stop.sa_flags = 0;

	sigaction(SIGINT, &sa_stop, NULL);
	sigaction(SIGTERM, &sa_stop, NULL);
	
	RDSEncoder rdsEncoder = {0};
	uint8_t err = init_lua(&rdsEncoder);
    if(err == 1) {
		fprintf(stderr, "Could not create lua state - not enough memory\n");
		goto exit;
	} else if (err == 2) {
		fprintf(stderr, "Could not load lua script file\n");
		goto exit;
	}
	init_rds_encoder(&rdsEncoder);

	if(open_udp_server(config.udp_port) == 0) {
		printf("Reading control commands on UDP:%d.\n", config.udp_port);
		int r = pthread_create(&udp_server_thread, &attr, udp_server_worker, NULL);
		if (r < 0) {
			fprintf(stderr, "Could not create UDP server thread.\n");
			config.udp_port = 0;
			goto exit;
		} else printf("Created UDP server thread.\n");
	} else {
		fprintf(stderr, "Failed to open UDP server\n");
		config.udp_port = 0;
	}

	if(config.asciig == 1 && config.tcp_port > 0) {
		if (init_tcp_server(config.tcp_port) < 0) {
			fprintf(stderr, "Failed to initialize TCP server\n");
			goto exit;
		}
	}

	if(config.asciig == 0) {
		IPC_Client client;
		if (ipc_connect(&client, "/etc/fm95/ctl.socket") < 0) goto exit;

		while (!stop_rds) {
			for (uint8_t s = 0; s < config.num_streams; s++) {
				ipc_send_bits(&client, &rdsEncoder, s);
			}
			msleep((int)(1000.0 * BITS_PER_GROUP / 1187.5));
		}

		ipc_close(&client);
	} else {
		RDSGroup group;
		char output_buffer[1024];
		char starts[4][5] = {"G:\r\n", "H:\r\n", "I:\r\n", "J:\r\n"};

		setvbuf(stderr, NULL, _IONBF, 0);

		while(!stop_rds) {
			if (is_tcp_server_running()) accept_tcp_clients();

			for(uint8_t i = 0; i < config.num_streams; i++) {
				get_rds_group(&rdsEncoder, &group, i);

				int offset = 0;
				memset(output_buffer, 0, sizeof(output_buffer));
				offset += snprintf(output_buffer + offset, sizeof(output_buffer) - offset, "%s", starts[i]);

				for(uint8_t j = 0; j < GROUP_LENGTH; j++) offset += snprintf(output_buffer + offset, sizeof(output_buffer) - offset, "%04X", get_block_from_group(&group, j));
				offset += snprintf(output_buffer + offset, sizeof(output_buffer) - offset, "\r\n\r\n");

				fwrite(output_buffer, 1, offset, stderr);
				fflush(stderr);

				if (is_tcp_server_running()) send_to_tcp_clients(output_buffer, offset);
			}

			msleep(10);
		}
	}

exit:
	if(config.udp_port) {
		fprintf(stderr, "Waiting for UDP thread to shut down.\n");
		pthread_join(udp_server_thread, NULL);
	}

	if (is_tcp_server_running()) close_tcp_server();

	encoder_saveToFile(&rdsEncoder);
	printf("Saved to file\n");
	
	pthread_attr_destroy(&attr);

	return 0;
}