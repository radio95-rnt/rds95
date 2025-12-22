#include "ascii_cmd.h"

typedef struct {
	const char *cmd;
	void (*handler)(char *arg, RDSModulator* mod, char* output);
	uint8_t cmd_length;
} command_handler_t;

typedef struct {
	const char *prefix;
	const char *suffix;
	void (*handler)(char *arg, char *pattern, RDSModulator* mod, char* output);
} pattern_command_handler_t;

static void handle_udg(char *arg, char *pattern, RDSModulator* mod, char* output) {
	uint8_t all_scanned = 1, bad_format = 0;
	uint16_t blocks[8][3];
	int sets = 0;
	char *ptr = arg;

	while (sets < 8) {
		int count = sscanf(ptr, "%4hx%4hx%4hx", &blocks[sets][0], &blocks[sets][1], &blocks[sets][2]);
		if (count != 3) {
			all_scanned = 0;
			break;
		}
		sets++;
		while (*ptr && *ptr != ',') ptr++;
		if (*ptr == ',') ptr++;
		else {
			bad_format = 1;
			break;
		}
	}

	if (strcmp(pattern, "1") == 0) {
		memcpy(&(mod->enc->data[mod->enc->program].udg1), &blocks, sets * sizeof(uint16_t[3]));
		mod->enc->data[mod->enc->program].udg1_len = sets;
	} else if(strcmp(pattern, "2") == 0) {
		memcpy(&(mod->enc->data[mod->enc->program].udg2), &blocks, sets * sizeof(uint16_t[3]));
		mod->enc->data[mod->enc->program].udg2_len = sets;
	} else strcpy(output, "!");
	if(bad_format) strcpy(output, "-");
	else if(all_scanned) strcpy(output, "+");
	else strcpy(output, "/");
}

static void handle_udg2(char *arg, char *pattern, RDSModulator* mod, char* output) {
	uint8_t all_scanned = 1, bad_format = 0;
	uint16_t blocks[8][4];
	int sets = 0;
	char *ptr = arg;

	while (sets < 8) {
		int count = sscanf(ptr, "%4hx%4hx%4hx%4hx", &blocks[sets][0], &blocks[sets][1], &blocks[sets][2], &blocks[sets][3]);
		if (count != 4) {
			all_scanned = 0;
			break;
		}
		sets++;
		while (*ptr && *ptr != ',') ptr++;
		if (*ptr == ',') ptr++;
		else {
			bad_format = 1;
			break;
		}
	}

	if (strcmp(pattern, "1") == 0) {
		memcpy(&(mod->enc->data[mod->enc->program].udg1_rds2), &blocks, sets * sizeof(uint16_t[4]));
		mod->enc->data[mod->enc->program].udg1_len_rds2 = sets;
	} else if(strcmp(pattern, "2") == 0) {
		memcpy(&(mod->enc->data[mod->enc->program].udg2_rds2), &blocks, sets * sizeof(uint16_t[4]));
		mod->enc->data[mod->enc->program].udg2_len_rds2 = sets;
	} else strcpy(output, "!");
	if(bad_format) strcpy(output, "-");
	else if(all_scanned) strcpy(output, "+");
	else strcpy(output, "/");
}

static void handle_eonen(char *arg, char *pattern, RDSModulator* mod, char* output) {
	mod->enc->data[mod->enc->program].eon[atoi(pattern)-1].enabled = atoi(arg);
	strcpy(output, "+");
}

static void handle_eonpi(char *arg, char *pattern, RDSModulator* mod, char* output) {
	mod->enc->data[mod->enc->program].eon[atoi(pattern)-1].pi = strtoul(arg, NULL, 16);
	strcpy(output, "+");
}

static void handle_eonps(char *arg, char *pattern, RDSModulator* mod, char* output) {
	arg[PS_LENGTH * 2] = 0;

	RDSEON *eon = &mod->enc->data[mod->enc->program].eon[atoi(pattern)-1];
	memset(eon->ps, ' ', sizeof(eon->ps));

	uint16_t len = 0;
	while (*arg != 0 && len < 24) eon->ps[len++] = *arg++;

	strcpy(output, "+");
}

static void handle_eonpty(char *arg, char *pattern, RDSModulator* mod, char* output) {
	mod->enc->data[mod->enc->program].eon[atoi(pattern)-1].pty = atoi(arg);
	strcpy(output, "+");
}

static void handle_eonta(char *arg, char *pattern, RDSModulator* mod, char* output) {
	if (!mod->enc->data[mod->enc->program].eon[atoi(pattern)-1].enabled ||
		!mod->enc->data[mod->enc->program].eon[atoi(pattern)-1].tp) {
		strcpy(output, "-");
		return;
	}
	mod->enc->data[mod->enc->program].eon[atoi(pattern)-1].ta = atoi(arg);
	if(mod->enc->data[mod->enc->program].eon[atoi(pattern)-1].ta) mod->enc->data[mod->enc->program].ta = 1;
	strcpy(output, "+");
}

static void handle_eontp(char *arg, char *pattern, RDSModulator* mod, char* output) {
	mod->enc->data[mod->enc->program].eon[atoi(pattern)-1].tp = atoi(arg);
	strcpy(output, "+");
}

static void handle_eonaf(char *arg, char *pattern, RDSModulator* mod, char* output) {
	if (arg[0] == 0) {
		memset(&(mod->enc->data[mod->enc->program].eon[atoi(pattern)-1].af), 0, sizeof(mod->enc->data[mod->enc->program].eon[atoi(pattern)-1].af));
		strcpy(output, "+");
		return;
	}

	memset(&(mod->enc->data[mod->enc->program].eon[atoi(pattern)-1].af), 0, sizeof(mod->enc->data[mod->enc->program].eon[atoi(pattern)-1].af));
	uint8_t arg_count;
	RDSAFs new_af;
	float af[MAX_AFS], *af_iter;

	arg_count = sscanf(arg,
		"%f,%f,%f,%f,%f,"
		"%f,%f,%f,%f,%f,"
		"%f,%f,%f,%f,%f,"
		"%f,%f,%f,%f,%f,"
		"%f,%f,%f,%f,%f",
		&af[0],  &af[1],  &af[2],  &af[3],  &af[4],
		&af[5],  &af[6],  &af[7],  &af[8],  &af[9],
		&af[10], &af[11], &af[12], &af[13], &af[14],
		&af[15], &af[16], &af[17], &af[18], &af[19],
		&af[20], &af[21], &af[22], &af[23], &af[24]);

	af_iter = af;
	memset(&new_af, 0, sizeof(RDSAFs));

	while (arg_count-- != 0) add_rds_af(&new_af, *af_iter++);

	memcpy(&(mod->enc->data[mod->enc->program].eon[atoi(pattern)-1].af), &new_af, sizeof(mod->enc->data[mod->enc->program].eon[atoi(pattern)-1].af));
	strcpy(output, "+");
}

static void handle_eondt(char *arg, char *pattern, RDSModulator* mod, char* output) {
	mod->enc->data[mod->enc->program].eon[atoi(pattern)-1].data = strtoul(arg, NULL, 16);
	strcpy(output, "+");
}

static const pattern_command_handler_t pattern_commands[] = {
	{"EON", "EN", handle_eonen},
	{"EON", "PI", handle_eonpi},
	{"EON", "PS", handle_eonps},
	{"EON", "PTY", handle_eonpty},
	{"EON", "TA", handle_eonta},
	{"EON", "TP", handle_eontp},
	{"EON", "AF", handle_eonaf},
	{"EON", "DT", handle_eondt},
	{"UDG", "", handle_udg},
	{"2UDG", "", handle_udg2},
};

static bool process_pattern_commands(char *cmd, char *arg, char *output, RDSModulator* mod) {
	size_t cmd_len = strlen(cmd);
	char pattern_buffer[16] = {0};
	for (size_t i = 0; i < sizeof(pattern_commands) / sizeof(pattern_command_handler_t); i++) {
		const pattern_command_handler_t *handler = &pattern_commands[i];
		size_t prefix_len = strlen(handler->prefix);
		size_t suffix_len = strlen(handler->suffix);
		if (cmd_len > (prefix_len + suffix_len) && strncmp(cmd, handler->prefix, prefix_len) == 0 && strcmp(cmd + cmd_len - suffix_len, handler->suffix) == 0) {
			size_t pattern_len = cmd_len - prefix_len - suffix_len;
			if (pattern_len < sizeof(pattern_buffer)) {
				strncpy(pattern_buffer, cmd + prefix_len, pattern_len);
				pattern_buffer[pattern_len] = 0;
				handler->handler(arg, pattern_buffer, mod, output);
				return true;
			}
		}
	}
	return false;
}

void process_ascii_cmd(RDSModulator* mod, char *str, char *cmd_output) {
	char *cmd, *arg;

	char output[255];
	memset(output, 0, sizeof(output));

	char upper_str[CTL_BUFFER_SIZE];
	uint16_t cmd_len = _strnlen((const char*)str, CTL_BUFFER_SIZE);
	for(uint16_t i = 0; i < cmd_len; i++) if(str[i] == '\t') str[i] = ' ';

	strncpy(upper_str, str, CTL_BUFFER_SIZE);
	upper_str[CTL_BUFFER_SIZE-1] = 0;

	for(uint16_t i = 0; i < cmd_len && upper_str[i] != '='; i++) {
		if(upper_str[i] >= 'a' && upper_str[i] <= 'z') upper_str[i] = upper_str[i] - 'a' + 'A';
	}

	char *equals_pos = strchr(upper_str, '=');
	if (equals_pos != NULL) {
		cmd = upper_str;
		cmd[equals_pos - upper_str] = 0;
		arg = equals_pos + 1;
		process_pattern_commands(cmd, arg, output, mod);
	}
}