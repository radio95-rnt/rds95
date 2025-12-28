#include "common.h"
#include <signal.h>
#include <getopt.h>
#include <pthread.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#include "../inih/ini.h"

#include "rds.h"
#include "fs.h"
#include "modulator.h"
#include "udp_server.h"
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
		"\t-b,--bitstream\tEnable bitstream mode where the RDS modulator will be disabled and encoded RDS bits with checkwords will be sent from stderr\n"
		"\n",
		name,
		DEFAULT_CONFIG_PATH
	);
}

typedef struct
{
	uint16_t udp_port;
	char rds_device_name[48];
	uint8_t num_streams : 3;
	uint8_t bitstream : 1;
} RDS95_Config;

static int config_handler(void* user, const char* section, const char* name, const char* value) {
    RDS95_Config* config = (RDS95_Config*)user;

    #define MATCH(s, n) (strcmp(section, s) == 0 && strcmp(name, n) == 0)

    if (MATCH("rds95", "udp_port")) config->udp_port = (uint16_t)atoi(value);
    else if (MATCH("devices", "rds95")) {
        strncpy(config->rds_device_name, value, sizeof(config->rds_device_name) - 1);
        config->rds_device_name[sizeof(config->rds_device_name) - 1] = '\0';
    } else if (MATCH("rds95", "streams")) {
        int streams = atoi(value);
        if (streams > MAX_STREAMS || streams == 0) return 0;
        config->num_streams = (uint8_t)streams;
    } else if (MATCH("rds95", "bitstream")) {
		int bitstream = atoi(value);
        if (bitstream > 1 || bitstream < 0) return 0;
        config->bitstream = (uint8_t)bitstream;
    } else return 0;
    return 1;
}

int main(int argc, char **argv) {
	printf("rds95 (a RDS encoder by radio95) version %s\n", VERSION);

	char config_path[64] = DEFAULT_CONFIG_PATH;
	RDS95_Config config = {
		.udp_port = 0,
		.rds_device_name = "\0",
		.num_streams = DEFAULT_STREAMS,
		.bitstream = false
	};

	pa_simple *rds_device = NULL;
	pa_sample_spec format = {0};
	pa_buffer_attr buffer = {0};

	pthread_attr_t attr;
	pthread_t udp_server_thread;

	const char	*short_opt = "c:bh";

	struct option	long_opt[] =
	{
		{"config",		required_argument, NULL, 'c'},
		{"bitstream",		no_argument, NULL, 'b'},
		{"help",	no_argument,       NULL, 'h'},
		{ 0,		0,		0,	0 }
	};

	int opt;
	while((opt = getopt_long(argc, argv, short_opt, long_opt, NULL)) != -1) {
		switch (opt) {
			case 'c':
				memcpy(config_path, optarg, 62);
				config_path[48] = '\0';
				break;
			case 'b':
				config.bitstream = 1;
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

	if(_strnlen(config.rds_device_name, 48) == 0) {
		printf("Error: No output device\n");
		return 1;
	}

	printf("Using %d RDS stream(s)\n", config.num_streams);

	pthread_attr_init(&attr);

	struct sigaction sa_stop;

	sa_stop.sa_handler = stop;
	sigemptyset(&sa_stop.sa_mask);
	sa_stop.sa_flags = 0;

	sigaction(SIGINT, &sa_stop, NULL);
	sigaction(SIGTERM, &sa_stop, NULL);

	format.format = PA_SAMPLE_FLOAT32NE;
	format.channels = config.num_streams;
	format.rate = RDS_SAMPLE_RATE;

	buffer.prebuf = 0;
	buffer.tlength = buffer.maxlength = NUM_MPX_FRAMES * config.num_streams;

	if(config.bitstream == 0) {
		rds_device = pa_simple_new(
			NULL,
			"rds95",
			PA_STREAM_PLAYBACK,
			config.rds_device_name,
			"RDS Generator",
			&format,
			NULL,
			&buffer,
			NULL
		);
		if (rds_device == NULL) {
			fprintf(stderr, "Error: cannot open sound device.\n");
			goto exit;
		}
	}

	RDSModulator rdsModulator = {0};
    init_lua(&rdsModulator);
	
	RDSEncoder rdsEncoder = {0};
	init_rds_modulator(&rdsModulator, &rdsEncoder, config.num_streams);
	init_rds_encoder(&rdsEncoder);

	if(open_udp_server(config.udp_port, &rdsModulator) == 0) {
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

	if(config.bitstream == 0) {
		int pulse_error;

		float *rds_buffer = (float*)malloc(NUM_MPX_FRAMES * config.num_streams * sizeof(float));
		if (rds_buffer == NULL) {
			fprintf(stderr, "Error: Could not allocate memory for RDS buffer\n");
			goto exit;
		}

		while(!stop_rds) {
			for (uint16_t i = 0; i < NUM_MPX_FRAMES * config.num_streams; i++) rds_buffer[i] = get_rds_sample(&rdsModulator, i % config.num_streams);

			if (pa_simple_write(rds_device, rds_buffer, NUM_MPX_FRAMES * config.num_streams * sizeof(float), &pulse_error) != 0) {
				fprintf(stderr, "Error: could not play audio. (%s : %d)\n", pa_strerror(pulse_error), pulse_error);
				break;
			}
		}

		free(rds_buffer);
	} else {
		uint8_t bit_buffer[BITS_PER_GROUP] = {0};
		#ifdef _WIN32
		_setmode(_fileno(stderr), _O_BINARY);
		#endif
		setvbuf(stderr, NULL, _IONBF, 0);
		while(!stop_rds) {
			unsigned char end = 0xff;
			for(uint8_t i = 0; i < config.num_streams; i++) {
				get_rds_bits(&rdsEncoder, bit_buffer, i);
				fwrite(&i, 1, 1, stderr);
				fwrite(bit_buffer, 1, BITS_PER_GROUP, stderr);
				fwrite(&end, 1, 1, stderr);
				fflush(stderr);
			}
		}
	}

exit:
	if(config.udp_port) {
		fprintf(stderr, "Waiting for UDP thread to shut down.\n");
		pthread_join(udp_server_thread, NULL);
	}

	encoder_saveToFile(&rdsEncoder);
	Modulator_saveToFile(&rdsModulator.params);
	printf("Saved to file\n");

	cleanup_rds_modulator(&rdsModulator);
	pthread_attr_destroy(&attr);
	if (rds_device != NULL && config.bitstream == 0) pa_simple_free(rds_device);

	return 0;
}
