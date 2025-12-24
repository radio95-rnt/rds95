#include "rds.h"
#include "lib.h"
#include "lua_rds.h"

uint16_t get_next_af(RDSEncoder* enc) {
	uint16_t out;

	if (enc->data[enc->program].af.num_afs) {
		if (enc->state[enc->program].af_state == 0) {
			out = (AF_CODE_NUM_AFS_BASE + enc->data[enc->program].af.num_afs) << 8 | enc->data[enc->program].af.afs[0];
			enc->state[enc->program].af_state++;
		} else {
			out = enc->data[enc->program].af.afs[enc->state[enc->program].af_state] << 8;
			if (enc->data[enc->program].af.afs[enc->state[enc->program].af_state + 1]) out |= enc->data[enc->program].af.afs[enc->state[enc->program].af_state + 1];
			else out |= AF_CODE_FILLER;
			enc->state[enc->program].af_state += 2;
		}
		if (enc->state[enc->program].af_state >= enc->data[enc->program].af.num_entries) enc->state[enc->program].af_state = 0;
	} else out = AF_CODE_NUM_AFS_BASE << 8 | AF_CODE_FILLER;

	return out;
}

void get_next_af_oda(RDSEncoder* enc, uint16_t* af_group) {
	uint8_t offset = 0;
	if (enc->state[enc->program].af_oda_state == 0) af_group[0] = (AF_CODE_NUM_AFS_BASE + enc->data[enc->program].af_oda.num_afs);
	else {
		af_group[0] = enc->data[enc->program].af_oda.afs[enc->state[enc->program].af_oda_state++];
		offset++;
	}
	for(int i = 0; i < 3; i++) {
		if (enc->data[enc->program].af_oda.afs[enc->state[enc->program].af_oda_state + offset]) af_group[i + 1] = enc->data[enc->program].af_oda.afs[enc->state[enc->program].af_oda_state + offset];
		else af_group[i + 1] = AF_CODE_FILLER;
		enc->state[enc->program].af_oda_state++;
	}
	if (enc->state[enc->program].af_oda_state >= enc->data[enc->program].af_oda.num_entries) enc->state[enc->program].af_oda_state = 0;
}

uint16_t get_next_af_eon(RDSEncoder* enc, uint8_t eon_index) {
	uint16_t out;

	if (enc->data[enc->program].eon[eon_index].af.num_afs) {
		if (enc->state[enc->program].eon_states[eon_index].af_state == 0) {
			out = (AF_CODE_NUM_AFS_BASE + enc->data[enc->program].af.num_afs) << 8 | enc->data[enc->program].eon[eon_index].af.afs[0];
			enc->state[enc->program].eon_states[eon_index].af_state += 1;
		} else {
			out = enc->data[enc->program].eon[eon_index].af.afs[enc->state[enc->program].eon_states[eon_index].af_state] << 8;
			if (enc->data[enc->program].eon[eon_index].af.afs[enc->state[enc->program].eon_states[eon_index].af_state + 1]) out |= enc->data[enc->program].eon[eon_index].af.afs[enc->state[enc->program].eon_states[eon_index].af_state + 1];
			else {
				out |= AF_CODE_FILLER;
				enc->state[enc->program].eon_states[eon_index].af_state += 2;
			}
		}
		if (enc->state[enc->program].eon_states[eon_index].af_state >= enc->data[enc->program].eon[eon_index].af.num_entries) enc->state[enc->program].eon_states[eon_index].af_state = 0;
	} else out = AF_CODE_NUM_AFS_BASE << 8 | AF_CODE_FILLER;

	return out;
}

void get_rds_ps_group(RDSEncoder* enc, RDSGroup *group) {
	if(enc->state[enc->program].ps_csegment == 0) {
		if(enc->state[enc->program].ps_update) {
			memcpy(enc->state[enc->program].ps_text, enc->data[enc->program].ps, PS_LENGTH);
			enc->state[enc->program].ps_update = 0;
		}

		if(enc->state[enc->program].tps_update) {
			memcpy(enc->state[enc->program].tps_text, enc->data[enc->program].tps, PS_LENGTH);
			enc->state[enc->program].tps_update = 0;
		}
	}

	group->b |= enc->data[enc->program].ta << 4;
	if(enc->state[enc->program].ps_csegment == 0) group->b |= enc->data[enc->program].dpty << 2;
	group->b |= enc->state[enc->program].ps_csegment;
	group->c = get_next_af(enc);
	if(enc->data[enc->program].ta && enc->state[enc->program].tps_text[0] != '\0') group->d = enc->state[enc->program].tps_text[enc->state[enc->program].ps_csegment * 2] << 8 | enc->state[enc->program].tps_text[enc->state[enc->program].ps_csegment * 2 + 1];
	else group->d = enc->state[enc->program].ps_text[enc->state[enc->program].ps_csegment * 2] << 8 |  enc->state[enc->program].ps_text[enc->state[enc->program].ps_csegment * 2 + 1];

	enc->state[enc->program].ps_csegment++;
	if (enc->state[enc->program].ps_csegment == 4) enc->state[enc->program].ps_csegment = 0;
}

void get_rds_fasttuning_group(RDSEncoder* enc, RDSGroup *group) {
	group->b |= 15 << 12;
	group->b |= 1 << 11;
	group->is_type_b = 1;

	group->b |= enc->data[enc->program].ta << 4;
	if(enc->state[enc->program].fasttuning_state == 0) group->b |= enc->data[enc->program].dpty << 2;
	group->b |= enc->state[enc->program].fasttuning_state;
	group->d = group->b;

	enc->state[enc->program].fasttuning_state++;
	if (enc->state[enc->program].fasttuning_state == 4) enc->state[enc->program].fasttuning_state = 0;
}

void get_rds_rt_group(RDSEncoder* enc, RDSGroup *group) {
	if (enc->state[enc->program].rt_update && enc->data[enc->program].rt1_enabled && !enc->data[enc->program].current_rt) {
		memcpy(enc->state[enc->program].rt_text, enc->data[enc->program].rt1, RT_LENGTH);
		TOGGLE(enc->state[enc->program].rt_ab);
		enc->state[enc->program].rt_update = 0;
		enc->state[enc->program].rt_state = 0;
		enc->data[enc->program].current_rt = 0;
	}
	if(enc->state[enc->program].rt2_update && enc->data[enc->program].rt2_enabled && enc->data[enc->program].current_rt) {
		memcpy(enc->state[enc->program].rt_text, enc->data[enc->program].rt2, RT_LENGTH);
		TOGGLE(enc->state[enc->program].rt_ab);
		enc->state[enc->program].rt2_update = 0;
		enc->state[enc->program].rt_state = 0;
		enc->data[enc->program].current_rt = 1;
	}

	uint8_t ab = enc->state[enc->program].rt_ab;
	switch (enc->data[enc->program].rt_type)
	{
	case 0:
		ab = 0;
		break;
	case 1:
		ab = (enc->data[enc->program].current_rt == 0) ? 0 : 1;
		break;
	default: break;
	}

	group->b |= 2 << 12;
	group->b |= ab << 4;
	group->b |= enc->state[enc->program].rt_state;
	group->c =  enc->state[enc->program].rt_text[enc->state[enc->program].rt_state * 4    ] << 8;
	group->c |= enc->state[enc->program].rt_text[enc->state[enc->program].rt_state * 4 + 1];
	group->d =  enc->state[enc->program].rt_text[enc->state[enc->program].rt_state * 4 + 2] << 8;
	group->d |= enc->state[enc->program].rt_text[enc->state[enc->program].rt_state * 4 + 3];

	uint8_t segments = (enc->data[enc->program].current_rt == 1) ? enc->state[enc->program].rt2_segments : enc->state[enc->program].rt_segments;
	enc->state[enc->program].rt_state++;
	if (enc->state[enc->program].rt_state == segments) enc->state[enc->program].rt_state = 0;
}

void get_rdsp_rtp_oda_group(RDSGroup *group) {
	group->b |= 3 << 12;
	group->b |= 11 << 1;
	group->d = ODA_AID_RTPLUS;
}

void get_rdsp_ertp_oda_group(RDSGroup *group) {
	group->b |= 3 << 12;
	group->b |= 13 << 1;
	group->d = ODA_AID_ERTPLUS;
}

void get_rdsp_ert_oda_group(RDSGroup *group) {
	group->b |= 3 << 12;
	group->b |= 12 << 1;
	group->c = 1; // UTF-8
	group->d = ODA_AID_ERT;
}

void get_rdsp_oda_af_oda_group(RDSGroup *group) {
	group->b |= 3 << 12;
	group->b |= 7 << 1;
	group->d = ODA_AID_ODAAF;
}

void get_rds_oda_af_group(RDSEncoder* enc, RDSGroup *group) {
	uint16_t af[4];
	get_next_af_oda(enc, af);

	group->b |= 7 << 12;
	for (int i = 0; i < 4; i++) group->b |= ((af[i] >> 8) & 1) << i;

	group->c = (af[0] & 0xFF) << 8;
	group->c |= af[1] & 0xFF;

	group->d = (af[2] & 0xFF) << 8;
	group->d |= af[3] & 0xFF;
}

void get_rdsp_ct_group(RDSGroup *group, time_t now) {
	struct tm *utc, *local_time;
	uint8_t l;
	uint32_t mjd;
	int16_t offset;

	utc = gmtime(&now);

	l = utc->tm_mon <= 1 ? 1 : 0;
	mjd = 14956 + utc->tm_mday + (uint32_t)((utc->tm_year - l) * 365.25f) + (uint32_t)((utc->tm_mon + (1+1) + l * 12) * 30.6001f);

	group->b |= 4 << 12 | (mjd >> 15);
	group->c = (mjd << 1) | (utc->tm_hour >> 4);
	group->d = (utc->tm_hour & 0xf) << 12 | utc->tm_min << 6;

	local_time = localtime(&now);
	offset = local_time->__tm_gmtoff / (30 * 60);
	if (offset < 0) group->d |= 1 << 5;
	group->d |= abs(offset);
}

void get_rds_lps_group(RDSEncoder* enc, RDSGroup *group) {
	if (enc->state[enc->program].lps_state == 0 && enc->state[enc->program].lps_update) {
		memcpy(enc->state[enc->program].lps_text, enc->data[enc->program].lps, LPS_LENGTH);
		enc->state[enc->program].lps_update = 0;
	}

	group->b |= 15 << 12 | enc->state[enc->program].lps_state;
	group->c =  enc->state[enc->program].lps_text[enc->state[enc->program].lps_state * 4] << 8;
	group->c |= enc->state[enc->program].lps_text[enc->state[enc->program].lps_state * 4 + 1];
	group->d =  enc->state[enc->program].lps_text[enc->state[enc->program].lps_state * 4 + 2] << 8;
	group->d |= enc->state[enc->program].lps_text[enc->state[enc->program].lps_state * 4 + 3];

	enc->state[enc->program].lps_state++;
	if (enc->state[enc->program].lps_state >= enc->state[enc->program].lps_segments) enc->state[enc->program].lps_state = 0;
}

void get_rds_ecc_group(RDSEncoder* enc, RDSGroup *group) {
	group->b |= 1 << 12;
	group->c = enc->state[enc->program].eon_linkage << 15;
	group->c |= enc->data[enc->program].ecc;
}

void get_rds_slcdata_group(RDSEncoder* enc, RDSGroup *group) {
	group->b |= 1 << 12;
	group->c = enc->state[enc->program].eon_linkage << 15;
	group->c |= 0x6000;
	group->c |= enc->data[enc->program].slc_data;
}

void get_rds_ptyn_group(RDSEncoder* enc, RDSGroup *group) {
	if (enc->state[enc->program].ptyn_state == 0 && enc->state[enc->program].ptyn_update) {
		memcpy(enc->state[enc->program].ptyn_text, enc->data[enc->program].ptyn, PTYN_LENGTH);
		TOGGLE(enc->state[enc->program].ptyn_ab);
		enc->state[enc->program].ptyn_update = 0;
	}

	group->b |= 10 << 12 | enc->state[enc->program].ptyn_state;
	group->b |= enc->state[enc->program].ptyn_ab << 4;
	group->c =  enc->state[enc->program].ptyn_text[enc->state[enc->program].ptyn_state * 4] << 8;
	group->c |= enc->state[enc->program].ptyn_text[enc->state[enc->program].ptyn_state * 4 + 1];
	group->d =  enc->state[enc->program].ptyn_text[enc->state[enc->program].ptyn_state * 4 + 2] << 8;
	group->d |= enc->state[enc->program].ptyn_text[enc->state[enc->program].ptyn_state * 4 + 3];

	enc->state[enc->program].ptyn_state++;
	if (enc->state[enc->program].ptyn_state == 2) enc->state[enc->program].ptyn_state = 0;
}

void get_rds_rtplus_group(RDSEncoder* enc, RDSGroup *group) {
	group->b |= 11 << 12;
	group->b |= enc->rtpState[enc->program][0].toggle << 4 | enc->rtpData[enc->program][0].running << 3;
	group->b |= (enc->rtpData[enc->program][0].type[0] & 0xf8) >> 3;

	group->c =  (enc->rtpData[enc->program][0].type[0] & 0x07) << 13;
	group->c |= (enc->rtpData[enc->program][0].start[0] & 0x3f) << 7;
	group->c |= (enc->rtpData[enc->program][0].len[0] & 0x3f) << 1;
	group->c |= (enc->rtpData[enc->program][0].type[1] & 0xe0) >> 5;

	group->d =  (enc->rtpData[enc->program][0].type[1] & 0x1f) << 11;
	group->d |= (enc->rtpData[enc->program][0].start[1] & 0x3f) << 5;
	group->d |= enc->rtpData[enc->program][0].len[1] & 0x1f;
}

void get_rds_ertplus_group(RDSEncoder* enc, RDSGroup *group) {
	group->b |= 13 << 12;
	group->b |= enc->rtpState[enc->program][1].toggle << 4 | enc->rtpData[enc->program][1].running << 3;
	group->b |= (enc->rtpData[enc->program][1].type[0] & 0xf8) >> 3;

	group->c =  (enc->rtpData[enc->program][1].type[0] & 0x07) << 13;
	group->c |= (enc->rtpData[enc->program][1].start[0] & 0x3f) << 7;
	group->c |= (enc->rtpData[enc->program][1].len[0] & 0x3f) << 1;
	group->c |= (enc->rtpData[enc->program][1].type[1] & 0xe0) >> 5;

	group->d =  (enc->rtpData[enc->program][1].type[1] & 0x1f) << 11;
	group->d |= (enc->rtpData[enc->program][1].start[1] & 0x3f) << 5;
	group->d |= enc->rtpData[enc->program][1].len[1] & 0x1f;
}

void get_rds_eon_group(RDSEncoder* enc, RDSGroup *group) {
	RDSEON eon;
	eon = enc->data[enc->program].eon[enc->state[enc->program].eon_index];
	while(1) {
		group->b |= 14 << 12;
		group->b |= eon.tp << 4;

		switch (enc->state[enc->program].eon_state)
		{
		case 0:
		case 1:
		case 2:
		case 3:
			group->c = eon.ps[enc->state[enc->program].eon_state*2] << 8;
			group->c |= eon.ps[enc->state[enc->program].eon_state*2+1];
			group->b |= enc->state[enc->program].eon_state;
			break;
		case 4:
			group->c = get_next_af_eon(enc, enc->state[enc->program].eon_index);
			group->b |= enc->state[enc->program].eon_state;
			break;
		case 5: // 13
			group->c = eon.pty << 11;
			if(eon.tp) group->c |= eon.ta;
			group->b |= 13;
			break;
		case 6: // 15
			group->c = eon.data;
			group->b |= 15;
			break;
		}

		group->d = eon.pi;

		if(enc->state[enc->program].eon_state == 6) {
			enc->state[enc->program].eon_index++;

			uint8_t i = 0;
			while(i < 4 && !enc->data[enc->program].eon[enc->state[enc->program].eon_index].enabled) {
				enc->state[enc->program].eon_index++;
				if(enc->state[enc->program].eon_index >= EONs) enc->state[enc->program].eon_index = 0;
				eon = enc->data[enc->program].eon[enc->state[enc->program].eon_index];
				i++;
			}

			enc->state[enc->program].eon_state = 0;
		} else {
			enc->state[enc->program].eon_state++;
			break;
		}
	}
}

void get_rds_ert_group(RDSEncoder* enc, RDSGroup *group) {
	if (enc->state[enc->program].ert_state == 0 && enc->state[enc->program].ert_update) {
		memcpy(enc->state[enc->program].ert_text, enc->data[enc->program].ert, ERT_LENGTH);
		enc->state[enc->program].ert_update = 0;
	}

	group->b |= 12 << 12 | (enc->state[enc->program].ert_state & 31);
	group->c = enc->state[enc->program].ert_text[enc->state[enc->program].ert_state * 4] << 8;
	group->c |= enc->state[enc->program].ert_text[enc->state[enc->program].ert_state * 4 + 1];
	group->d = enc->state[enc->program].ert_text[enc->state[enc->program].ert_state * 4 + 2] << 8;
	group->d |= enc->state[enc->program].ert_text[enc->state[enc->program].ert_state * 4 + 3];

	enc->state[enc->program].ert_state++;
	if (enc->state[enc->program].ert_state >= enc->state[enc->program].ert_segments) enc->state[enc->program].ert_state = 0;
}

uint8_t get_rds_custom_groups(RDSEncoder* enc, RDSGroup *group) {
	if(enc->state[enc->program].custom_group[0] == 1) {
		enc->state[enc->program].custom_group[0] = 0;
		group->a = enc->data[enc->program].pi;
		group->b = enc->state[enc->program].custom_group[1];
		group->c = enc->state[enc->program].custom_group[2];
		group->d = enc->state[enc->program].custom_group[3];
		group->is_type_b = (IS_TYPE_B(group->b) != 0);
		return 1;
	}
	return 0;
}
uint8_t get_rds_custom_groups2(RDSEncoder* enc, RDSGroup *group) {
	if(enc->state[enc->program].custom_group2[0] == 1) {
		enc->state[enc->program].custom_group2[0] = 0;
		group->a = enc->state[enc->program].custom_group2[1];
		group->b = enc->state[enc->program].custom_group2[2];
		group->c = enc->state[enc->program].custom_group2[3];
		group->d = enc->state[enc->program].custom_group2[4];
		group->is_type_b = (group->a == 0 && IS_TYPE_B(group->b));
		return 1;
	}
	return 0;
}
void get_rdsp_lua_group(RDSGroup *group) {
	lua_group(group);
	group->is_type_b = (IS_TYPE_B(group->b) != 0);
}

void get_rds_user_oda_group(RDSEncoder* enc, RDSGroup *group) {
	uint8_t pointer = enc->state[enc->program].user_oda.oda_pointer++;
	if(enc->state[enc->program].user_oda.oda_pointer >= enc->state[enc->program].user_oda.oda_len) enc->state[enc->program].user_oda.oda_pointer = 0;
	RDSODA oda = enc->state[enc->program].user_oda.odas[pointer];

	group->b |= 3 << 12;
	group->b |= oda.group << 1;
	group->b |= oda.group_version;
	group->c = oda.id_data;
	group->d = oda.id;
}