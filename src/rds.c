#include "rds.h"
#include "fs.h"
#include "modulator.h"
#include "lib.h"
#include "lua_rds.h"
#include <time.h>

// declarations (stupid c)
uint16_t get_next_af(RDSEncoder* enc);
void get_next_af_oda(RDSEncoder* enc, uint16_t* af_group);
uint16_t get_next_af_eon(RDSEncoder* enc, uint8_t eon_index);
void get_rds_ps_group(RDSEncoder* enc, RDSGroup *group);
void get_rds_fasttuning_group(RDSEncoder* enc, RDSGroup *group);
void get_rds_rt_group(RDSEncoder* enc, RDSGroup *group);
void get_rdsp_rtp_oda_group(RDSGroup *group);
void get_rdsp_ertp_oda_group(RDSGroup *group);
void get_rdsp_oda_af_oda_group(RDSGroup *group);
void get_rds_oda_af_group(RDSEncoder* enc, RDSGroup *group);
void get_rdsp_ct_group(RDSGroup *group, time_t now);
void get_rds_lps_group(RDSEncoder* enc, RDSGroup *group);
void get_rds_ecc_group(RDSEncoder* enc, RDSGroup *group);
void get_rds_slcdata_group(RDSEncoder* enc, RDSGroup *group);
void get_rds_ptyn_group(RDSEncoder* enc, RDSGroup *group);
void get_rds_rtplus_group(RDSEncoder* enc, RDSGroup *group);
void get_rds_ertplus_group(RDSEncoder* enc, RDSGroup *group);
void get_rds_eon_group(RDSEncoder* enc, RDSGroup *group);
uint8_t get_rds_custom_groups(RDSEncoder* enc, RDSGroup *group);
uint8_t get_rds_custom_groups2(RDSEncoder* enc, RDSGroup *group);
int get_rdsp_lua_group(RDSGroup *group, const char grp);
void get_rds_user_oda_group(RDSEncoder* enc, RDSGroup *group);
int get_rds_user_oda_group_content(RDSEncoder* enc, RDSGroup *group);

#define HANDLE_UDG_STREAM(chan_idx, udg_prefix) \
    do { \
        if (stream != 0) { \
            udg_idx = enc->state[enc->program].udg_idxs_rds2[chan_idx]; \
            group->a = enc->data[enc->program].udg_prefix##_rds2[udg_idx][0]; \
            group->b = enc->data[enc->program].udg_prefix##_rds2[udg_idx][1]; \
            group->c = enc->data[enc->program].udg_prefix##_rds2[udg_idx][2]; \
            group->d = enc->data[enc->program].udg_prefix##_rds2[udg_idx][3]; \
            enc->state[enc->program].udg_idxs_rds2[chan_idx]++; \
            if (enc->state[enc->program].udg_idxs_rds2[chan_idx] == enc->data[enc->program].udg_prefix##_len_rds2) \
                enc->state[enc->program].udg_idxs_rds2[chan_idx] = 0; \
            group->is_type_b = (group->a == 0 && IS_TYPE_B(group->b)); \
        } else { \
            udg_idx = enc->state[enc->program].udg_idxs[chan_idx]; \
            group->b = enc->data[enc->program].udg_prefix[udg_idx][0]; \
            group->c = enc->data[enc->program].udg_prefix[udg_idx][1]; \
            group->d = enc->data[enc->program].udg_prefix[udg_idx][2]; \
            enc->state[enc->program].udg_idxs[chan_idx]++; \
            if (enc->state[enc->program].udg_idxs[chan_idx] == enc->data[enc->program].udg_prefix##_len) \
                enc->state[enc->program].udg_idxs[chan_idx] = 0; \
            group->is_type_b = (IS_TYPE_B(group->b) != 0); \
        } \
    } while(0)

static void get_rds_sequence_group(RDSEncoder* enc, RDSGroup *group, char* grp, uint8_t stream) {
	uint8_t udg_idx;
	uint8_t ps_seq_idx = (stream == 0) ? 1 : 3;
	uint8_t grp_seq_idx = (stream == 0) ? 0 : 2;
	switch (*grp)
	{
		default:
		case '0':
			if(enc->state[enc->program].grp_seq_idx[ps_seq_idx] < 4) enc->state[enc->program].grp_seq_idx[grp_seq_idx]--;
			else enc->state[enc->program].grp_seq_idx[ps_seq_idx] = 0;

			enc->state[enc->program].grp_seq_idx[ps_seq_idx]++;
			get_rds_ps_group(enc, group);
			break;
		case '1':
			if(enc->state[enc->program].data_ecc == 0 && enc->data[enc->program].slc_data != 0) get_rds_slcdata_group(enc, group);
			else get_rds_ecc_group(enc, group);
			TOGGLE(enc->state[enc->program].data_ecc);
			break;
		case '2':
			get_rds_rt_group(enc, group);
			break;
		case 'A':
			get_rds_ptyn_group(enc, group);
			break;
		case 'E':
			get_rds_eon_group(enc, group);
			break;
		case 'X':
			HANDLE_UDG_STREAM(0, udg1);
			break;
		case 'Y':
			HANDLE_UDG_STREAM(1, udg2);
			break;
		case 'R':
			if(enc->state[enc->program].rtp_oda == 0) get_rds_rtplus_group(enc, group);
			else get_rdsp_rtp_oda_group(group);
			TOGGLE(enc->state[enc->program].rtp_oda);
			break;
		case 'P':
			if(enc->state[enc->program].ert_oda == 0) get_rds_ertplus_group(enc, group);
			else get_rdsp_ertp_oda_group(group);
			TOGGLE(enc->state[enc->program].ert_oda);
			break;
		case 'F':
			get_rds_lps_group(enc, group);
			break;
		case 'T':
			get_rds_fasttuning_group(enc, group);
			break;
		case 'L':
			if(get_rdsp_lua_group(group, *grp) == 0) get_rds_ps_group(enc, group);
			break;
		case 'O':
			get_rds_user_oda_group(enc, group);
			break;
		case 'S':
		case 'K':
			if(get_rds_user_oda_group_content(enc, group) == 0) get_rds_ps_group(enc, group);
			break;
		case 'U':
			if(enc->state[enc->program].af_oda == 0) get_rds_oda_af_group(enc, group);
			else get_rdsp_oda_af_oda_group(group);
			TOGGLE(enc->state[enc->program].af_oda);
			break;
	}
}

static uint8_t check_rds_good_group(RDSEncoder* enc, char* grp) {
	uint8_t good_group = 0;
	if(*grp == '0') good_group = 1;
	if(*grp == '1' && (enc->data[enc->program].ecc != 0 || enc->data[enc->program].slc_data != 0)) good_group = 1;
	if(*grp == '2' && (enc->data[enc->program].rt1_enabled || enc->data[enc->program].rt2_enabled)) good_group = 1;
	if(*grp == 'A' && enc->data[enc->program].ptyn_enabled) good_group = 1;
	if(*grp == 'E') {
		for (int i = 0; i < EONs; i++) {
			if (enc->data[enc->program].eon[i].enabled) {
				good_group = 1;
				break;
			}
		}
	}
	if(*grp == 'X' && enc->data[enc->program].udg1_len != 0) good_group = 1;
	if(*grp == 'Y' && enc->data[enc->program].udg2_len != 0) good_group = 1;
	if(*grp == 'R' && enc->rtpData[enc->program][0].enabled) good_group = 1;
	if(*grp == 'P' && enc->rtpData[enc->program][1].enabled) good_group = 1;
	if(*grp == 'F' && enc->data[enc->program].lps[0] != '\0') good_group = 1;
	if(*grp == 'T') good_group = 1;
	if(*grp == 'L') good_group = 1;
	if(*grp == 'O' && enc->state[enc->program].user_oda.oda_len != 0) good_group = 1;
	if(*grp == 'K') {
		for (int i = 0; i < enc->state->user_oda.oda_len; i++) {
			if (enc->state->user_oda.odas[i].lua_handler != 0) {
				good_group = 1;
				break;
			}
		}
	}
	if(*grp == 'U' && enc->data[enc->program].af_oda.num_afs) good_group = 1;
	return good_group;
}

static void get_rds_group(RDSEncoder* enc, RDSGroup *group, uint8_t stream) {
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
		lua_call_function("tick");
	}

	if (utc->tm_min != enc->state[enc->program].last_minute) {
		enc->state[enc->program].last_minute = utc->tm_min;

		uint8_t eon_has_ta = 0;
		for (int i = 0; i < EONs; i++) {
			if (enc->data[enc->program].eon[i].enabled && enc->data[enc->program].eon[i].ta) {
				eon_has_ta = 1;
				break;
			}
		}
		if(enc->data[enc->program].tp && enc->data[enc->program].ta && enc->state[enc->program].ta_timeout && !eon_has_ta) {
			enc->state[enc->program].ta_timeout--;
			if(enc->state[enc->program].ta_timeout == 0) {
				enc->data[enc->program].ta = 0;
				enc->state[enc->program].ta_timeout_state = enc->state[enc->program].ta_timeout;
			};
		}

		if(enc->data[enc->program].rt1_enabled && enc->data[enc->program].rt2_enabled && enc->state[enc->program].rt_switching_period_state) {
			enc->state[enc->program].rt_switching_period_state--;
			if(enc->state[enc->program].rt_switching_period_state == 0) {
				TOGGLE(enc->data[enc->program].current_rt);
				if (enc->data[enc->program].current_rt == 1) memcpy(enc->state[enc->program].rt_text, enc->data[enc->program].rt2, RT_LENGTH);
				else memcpy(enc->state[enc->program].rt_text, enc->data[enc->program].rt1, RT_LENGTH);
				enc->state[enc->program].rt_state = 0;
				enc->state[enc->program].rt_switching_period_state = enc->data[enc->program].rt_switching_period;
			}
		}

		if(enc->data[enc->program].rt1_enabled && enc->data[enc->program].current_rt == 0 && enc->state[enc->program].rt_text_timeout_state) {
			enc->state[enc->program].rt_text_timeout_state--;
			if(enc->state[enc->program].rt_text_timeout_state == 0) {
				enc->state[enc->program].rt_update = 1;
				memcpy(enc->state[enc->program].rt_text, enc->data[enc->program].default_rt, RT_LENGTH);
			}
		}

		if(enc->data[enc->program].ct && stream == 0) {
			get_rdsp_ct_group(group, now);
			goto group_coded;
		}
	}

	uint8_t good_group = 0;
	uint8_t cant_find_group = 0;
	uint8_t grp_sqc_idx = 0;
	char grp;

	if(stream != 0) {
		group->a = 0;
		if(get_rds_custom_groups2(enc, group)) goto group_coded_rds2;

		if(enc->encoder_data.rds2_mode == 0) {
			group->b = enc->state[enc->program].last_stream0_group[0];
			group->c = enc->state[enc->program].last_stream0_group[1];
			group->d = enc->state[enc->program].last_stream0_group[2];
			group->is_type_b = enc->state[enc->program].last_stream0_group_type_b;
			goto group_coded_rds2;
		} else if(enc->encoder_data.rds2_mode == 1) {
			while(good_group == 0) {
				grp_sqc_idx = enc->state[enc->program].grp_seq_idx[2];
				if(enc->data[enc->program].grp_sqc_rds2[grp_sqc_idx] == '\0') {
					enc->state[enc->program].grp_seq_idx[2] = 0;
					grp_sqc_idx = 0;
				}
				grp = enc->data[enc->program].grp_sqc_rds2[grp_sqc_idx];

				good_group = check_rds_good_group(enc, &grp);

				if(!good_group) cant_find_group++;
				else cant_find_group = 0;
				
				if(!good_group && cant_find_group == 23) {
					cant_find_group = 0;
					break;
				}

				enc->state[enc->program].grp_seq_idx[2]++;
			}
			if(!good_group) grp = '0';

			get_rds_sequence_group(enc, group, &grp, stream);

			goto group_coded_rds2;
		}
	}

	if(get_rds_custom_groups(enc, group)) goto group_coded;

	while(good_group == 0) {
		grp_sqc_idx = enc->state[enc->program].grp_seq_idx[0];
		if(enc->data[enc->program].grp_sqc[grp_sqc_idx] == '\0') {
			enc->state[enc->program].grp_seq_idx[0] = 0;
			grp_sqc_idx = 0;
		}
		grp = enc->data[enc->program].grp_sqc[grp_sqc_idx];

		good_group = check_rds_good_group(enc, &grp);

		if(!good_group) cant_find_group++;
		else cant_find_group = 0;
		if(!good_group && cant_find_group == 23) {
			cant_find_group = 0;
			break;
		}

		enc->state[enc->program].grp_seq_idx[0]++;
	}
	if(!good_group) grp = '0';

	get_rds_sequence_group(enc, group, &grp, stream);

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

void add_checkwords(RDSGroup *group, uint8_t *bits)
{	
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
	RDSEncoder tempCoder;
	tempCoder.program = program;
	memset(&(tempCoder.state[program]), 0, sizeof(RDSState));
	memset(&(tempCoder.rtpState[program]), 0, sizeof(RDSRTPlusState)*2);

	tempCoder.state[program].rt_ab = 1;
	tempCoder.state[program].ptyn_ab = 1;
	set_rds_rt1(&tempCoder, enc->data[program].rt1);
	set_rds_rt2(&tempCoder, enc->data[program].rt2);
	set_rds_ps(&tempCoder, enc->data[program].ps);
	set_rds_tps(&tempCoder, enc->data[program].tps);
	set_rds_ptyn(&tempCoder, enc->data[program].ptyn);
	set_rds_lps(&tempCoder, enc->data[program].lps);
	set_rds_grpseq(&tempCoder, enc->data[program].grp_sqc);
	set_rds_grpseq2(&tempCoder, enc->data[program].grp_sqc_rds2);

	struct tm *utc;
	time_t now;
	time(&now);
	utc = gmtime(&now);
	tempCoder.state[program].last_minute = utc->tm_min;
	tempCoder.state[program].last_second = utc->tm_sec;

	for(int i = 0; i < EONs; i++) tempCoder.data[program].eon[i].ta = 0;

	memcpy(&(enc->state[program]), &(tempCoder.state[program]), sizeof(RDSState));
	memcpy(&(enc->rtpState[program]), &(tempCoder.rtpState[program]), sizeof(RDSRTPlusState));
}

void set_rds_defaults(RDSEncoder* enc, uint8_t program) {
	memset(&(enc->data[program]), 0, sizeof(RDSData));
	memset(&(enc->rtpData[program]), 0, sizeof(RDSRTPlusData)*2);
	memset(&(enc->encoder_data), 0, sizeof(RDSEncoderData));

	enc->data[program].ct = 1;
	strcpy((char *)enc->data[program].grp_sqc, DEFAULT_GRPSQC);
	enc->data[program].tp = 1;
	enc->data[program].pi = 0xFFFF;
	strcpy((char *)enc->data[program].ps, "* RDS * ");
	enc->data[program].rt1_enabled = 1;

	memset(enc->data[program].rt1, ' ', 59);

	enc->data[program].rt_type = 2;

	reset_rds_state(enc, program);
}

void init_rds_encoder(RDSEncoder* enc) {
	memset(enc, 0, sizeof(RDSEncoder));
	for(int i = 0; i < PROGRAMS; i++) set_rds_defaults(enc, i);

	if (encoder_loadFromFile(enc)) {
		printf("Encoder file will be reinitialized.\n");
    	lua_call_function("on_init");
	}
	for(int i = 0; i < PROGRAMS; i++) reset_rds_state(enc, i);
	lua_call_function("on_start");
    lua_call_function("on_state");
	encoder_saveToFile(enc);
}