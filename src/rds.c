#include "rds.h"
#include "fs.h"
#include "modulator.h"
#include "lib.h"
#include "lua_rds.h"
#include <time.h>

// declarations (stupid c)
uint16_t get_next_af(RDSEncoder* enc);
uint16_t get_next_af_eon(RDSEncoder* enc, uint8_t eon_index);
void get_rds_ps_group(RDSEncoder* enc, RDSGroup *group);
void get_rds_fasttuning_group(RDSEncoder* enc, RDSGroup *group);
void get_rds_rt_group(RDSEncoder* enc, RDSGroup *group);
void get_rdsp_ct_group(RDSGroup *group, time_t now);
void get_rds_lps_group(RDSEncoder* enc, RDSGroup *group);
void get_rds_ecc_group(RDSEncoder* enc, RDSGroup *group);
void get_rds_slcdata_group(RDSEncoder* enc, RDSGroup *group);
void get_rds_ptyn_group(RDSEncoder* enc, RDSGroup *group);
void get_rds_eon_group(RDSEncoder* enc, RDSGroup *group);
uint8_t get_rds_custom_groups(RDSEncoder* enc, RDSGroup *group);
uint8_t get_rds_custom_groups2(RDSEncoder* enc, RDSGroup *group);
int get_rdsp_lua_group(RDSGroup *group, const char grp);

void get_rds_sequence_group(RDSEncoder* enc, RDSGroup *group, uint8_t good_group, char* grp) {
	if(good_group == 0) {
		if(get_rdsp_lua_group(group, *grp) == 0) get_rds_ps_group(enc, group);
		return;
	}

	switch (*grp) {
		case 0:
			get_rds_ps_group(enc, group);
			break;
		case 2:
			if(enc->state[enc->program].data_ecc == 0 && enc->data[enc->program].slc_data != 0) get_rds_slcdata_group(enc, group);
			else get_rds_ecc_group(enc, group);
			TOGGLE(enc->state[enc->program].data_ecc);
			break;
		case 4:
			get_rds_rt_group(enc, group);
			break;
		case 20:
			get_rds_ptyn_group(enc, group);
			break;
		case 28:
			get_rds_eon_group(enc, group);
			break;
		case 30:
			get_rds_lps_group(enc, group);
			break;
		case 31:
			get_rds_fasttuning_group(enc, group);
			break;
		default:
			if(get_rdsp_lua_group(group, *grp) == 0) get_rds_ps_group(enc, group);
			break;
	}
}

uint8_t check_rds_good_group(RDSEncoder* enc, char* grp) {
	if(*grp == 2) {
		if(enc->data[enc->program].ecc != 0 || enc->data[enc->program].slc_data != 0) return 1;
		return 0;
	}
	if(*grp == 4) {
		if(enc->data[enc->program].rt_enabled) return 1;
		return 0;
	}
	if(*grp == 20) {
		if(enc->data[enc->program].ptyn_enabled) return 1;
		return 0;
	}
	if(*grp == 28) {
		for (int i = 0; i < EONs; i++) {
			if (enc->data[enc->program].eon[i].enabled) {
				return 1;
				break;
			}
		}
	}
	if(*grp == 30) {
		if(enc->data[enc->program].lps[0] != '\0') return 1;
		return 0;
	}
	return 1;
}

void get_rds_group(RDSEncoder* enc, RDSGroup *group, uint8_t stream) {
	group->a = enc->data[enc->program].pi;
	group->b = 0;
	group->c = 0;
	group->d = 0;
	group->is_type_b = 0;

	struct tm *utc;
	time_t now;
	time(&now);
	utc = gmtime(&now);

	if(utc->tm_sec != enc->state[enc->program].last_second) {
		enc->state[enc->program].last_second = utc->tm_sec;
		lua_call_tfunction("tick");
	}

	if (utc->tm_min != enc->state[enc->program].last_minute) {
		enc->state[enc->program].last_minute = utc->tm_min;
		lua_call_tfunction("minute_tick");

		if(enc->data[enc->program].ct && stream == 0) {
			get_rdsp_ct_group(group, now);
			goto group_coded;
		}
	}

	uint8_t good_group = 0;
	uint8_t grp_sqc_idx = 0;
	char grp;

	if(stream != 0) {
		group->a = 0;
		if(get_rds_custom_groups2(enc, group)) goto group_coded_rds2;

		int generated = lua_rds2_group(group, stream);
		if(group->a == 0 && generated == 1) group->is_type_b = (IS_TYPE_B(group->b) != 0);
		else if(generated == 0) {
			group->b = enc->state[enc->program].last_stream0_group[0];
			group->c = enc->state[enc->program].last_stream0_group[1];
			group->d = enc->state[enc->program].last_stream0_group[2];
			group->is_type_b = enc->state[enc->program].last_stream0_group_type_b;
		}
		goto group_coded_rds2;
	}

	if(get_rds_custom_groups(enc, group)) goto group_coded;

	grp_sqc_idx = enc->state[enc->program].grp_seq_idx;
	if(grp_sqc_idx > enc->data[enc->program].grp_sqc_len) {
		enc->state[enc->program].grp_seq_idx = 0;
		grp_sqc_idx = 0;
	}
	grp = enc->data[enc->program].grp_sqc[grp_sqc_idx];

	good_group = check_rds_good_group(enc, &grp);

	enc->state[enc->program].grp_seq_idx++;

	get_rds_sequence_group(enc, group, good_group, &grp);

group_coded_rds2:
	if (group->a == 0 && stream != 0) {
		if(group->is_type_b) group->c = enc->data[enc->program].pi;
		group->b |= enc->data[enc->program].tp << 10;
		group->b |= enc->data[enc->program].pty << 5;
	} else if(stream == 0) goto group_coded;
	return;

group_coded:
	if(stream != 0) goto group_coded_rds2;
	group->b |= enc->data[enc->program].tp << 10;
	group->b |= enc->data[enc->program].pty << 5;
	if(group->is_type_b) group->c = enc->data[enc->program].pi;

	enc->state[enc->program].last_stream0_group[0] = group->b;
	enc->state[enc->program].last_stream0_group[1] = group->c;
	enc->state[enc->program].last_stream0_group[2] = group->d;
	enc->state[enc->program].last_stream0_group_type_b = group->is_type_b;
	return;
}

uint16_t offset_words_typea[] = {
	0x0FC, /*  A  */
	0x198, /*  B  */
	0x168, /*  C  */
	0x1B4, /*  D  */
};
uint16_t offset_words_typeb[] = {
	0x0FC, /*  A  */
	0x198, /*  B  */
	0x350, /*  C' */
	0x1B4, /*  D  */
};

void add_checkwords(RDSGroup *group, uint8_t *bits) {
	uint16_t* offset_words;
	if(group->is_type_b) offset_words = offset_words_typeb;
	else offset_words = offset_words_typea;

	for (uint8_t i = 0; i < GROUP_LENGTH; i++) {
		uint16_t block = get_block_from_group(group, i);

		uint16_t block_crc = 0;
		uint8_t j;
		for (j = 0; j < BLOCK_SIZE; j++) {
			uint8_t bit = (block & (0x8000 >> j)) != 0;
			uint8_t msb = (block_crc >> (POLY_DEG - 1)) & 1;
			block_crc <<= 1;
			if (msb ^ bit) block_crc ^= POLY;
			*bits++ = bit; // Write the data itself to the bits
		}
		for (j = 0; j < POLY_DEG; j++) *bits++ = ((block_crc ^ offset_words[i]) >> (POLY_DEG - 1 - j)) & 1; // Write the checkword to the bits
	}
}

void get_rds_bits(RDSEncoder* enc, uint8_t *bits, uint8_t stream) {
	RDSGroup group;
	get_rds_group(enc, &group, stream);
	add_checkwords(&group, bits);
}

void reset_rds_state(RDSEncoder* enc, uint8_t program) {
	RDSEncoder tempCoder = {0};
	tempCoder.program = program;
	memset(&tempCoder.state[program], 0, sizeof(RDSState));

	tempCoder.state[program].ptyn_ab = 1;
	set_rds_rt(&tempCoder, enc->data[program].rt, program);
	set_rds_ps(&tempCoder, enc->data[program].ps, program);
	set_rds_tps(&tempCoder, enc->data[program].tps, program);
	set_rds_lps(&tempCoder, enc->data[program].lps, program);
	set_rds_ptyn(&tempCoder, enc->data[program].ptyn, program);

	struct tm *utc;
	time_t now;
	time(&now);
	utc = gmtime(&now);
	tempCoder.state[program].last_minute = utc->tm_min;
	tempCoder.state[program].last_second = utc->tm_sec;

	memcpy(&enc->state[program], &tempCoder.state[program], sizeof(RDSState));

	for(int i = 0; i < EONs; i++) enc->data[program].eon[i].ta = 0;
	enc->data[program].ta = 0;
}

void set_rds_defaults(RDSEncoder* enc, uint8_t program) {
	memset(&(enc->data[program]), 0, sizeof(RDSData));

	enc->data[program].ct = 1;
	strcpy((char *)enc->data[program].grp_sqc, "0");
	enc->data[program].tp = 1;
	enc->data[program].pi = 0xFFFF;
	strcpy((char *)enc->data[program].ps, "* RDS * ");
	enc->data[program].rt_enabled = 1;

	memset(enc->data[program].rt, ' ', 64);

	reset_rds_state(enc, program);
}

void init_rds_encoder(RDSEncoder* enc) {
	memset(enc, 0, sizeof(RDSEncoder));
	for(int i = 0; i < PROGRAMS; i++) set_rds_defaults(enc, i);

	if (encoder_loadFromFile(enc)) {
		printf("Encoder file will be reinitialized.\n");
    	lua_call_tfunction("on_init");
	}
	for(int i = 0; i < PROGRAMS; i++) reset_rds_state(enc, i);
	lua_call_tfunction("on_start");
    lua_call_tfunction("on_state");
	encoder_saveToFile(enc);
}