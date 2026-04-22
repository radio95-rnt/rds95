#include "rds.h"

void set_rds_rt1(RDSEncoder* enc, const char *rt1, uint8_t program) {
	uint8_t i = 0, len = 0;

	enc->state[program].rt_text_timeout_state = enc->data[program].rt_text_timeout;

	enc->state[program].rt_update = 1;

	memset(enc->data[program].rt1, ' ', RT_LENGTH);
	while (*rt1 != 0 && len < RT_LENGTH) enc->data[program].rt1[len++] = *rt1++;

	while (len > 0 && enc->data[program].rt1[len - 1] == ' ') len--;

	if (len < RT_LENGTH) {
		enc->state[program].rt_segments = 0;
		enc->data[program].rt1[len++] = '\r';
		while (i < len) {
			i += 4;
			enc->state[program].rt_segments++;
		}
	} else enc->state[program].rt_segments = 16;
	if(enc->state[program].rt_segments > 16) enc->state[program].rt_segments = 16; //make sure
}

void set_rds_default_rt(RDSEncoder* enc, const char *rt, uint8_t program) {
	uint8_t i = 0, len = 0;

	memset(enc->data[program].default_rt, ' ', RT_LENGTH);
	while (*rt != 0 && len < RT_LENGTH) enc->data[program].default_rt[len++] = *rt++;

	while (len > 0 && enc->data[program].default_rt[len - 1] == ' ') len--;

	if (len < RT_LENGTH) {
		enc->state[program].default_rt_segments = 0;
		enc->data[program].default_rt[len++] = '\r';
		while (i < len) {
			i += 4;
			enc->state[program].default_rt_segments++;
		}
	} else enc->state[program].default_rt_segments = 16;
	if(enc->state[program].default_rt_segments > 16) enc->state[program].default_rt_segments = 16; //make sure
}

void set_rds_rt2(RDSEncoder* enc, const char *rt2, uint8_t program) {
	uint8_t i = 0, len = 0;

	enc->state[program].rt2_update = 1;

	memset(enc->data[program].rt2, ' ', RT_LENGTH);
	while (*rt2 != 0 && len < RT_LENGTH) enc->data[program].rt2[len++] = *rt2++;

	while (len > 0 && enc->data[program].rt2[len - 1] == ' ') len--;

	if (len < RT_LENGTH) {
		enc->state[program].rt2_segments = 0;
		enc->data[program].rt2[len++] = '\r';
		while (i < len) {
			i += 4;
			enc->state[program].rt2_segments++;
		}
	} else enc->state[program].rt2_segments = 16;
	if(enc->state[program].rt2_segments > 16) enc->state[program].rt2_segments = 16; //make sure
}

void set_rds_ps(RDSEncoder* enc, const char *ps, uint8_t program) {
	uint8_t len = 0;

	enc->state[program].ps_update = 1;
	memset(enc->data[program].ps, ' ', PS_LENGTH);
	while (*ps != 0 && len < PS_LENGTH) enc->data[program].ps[len++] = *ps++;
}

void set_rds_tps(RDSEncoder* enc, const char *tps, uint8_t program) {
	uint8_t len = 0;

	enc->state[program].tps_update = 1;
	if(tps[0] == '\0') {
		memset(enc->data[program].tps, 0, PS_LENGTH);
		return;
	}

	memset(enc->data[program].tps, ' ', PS_LENGTH);
	while (*tps != 0 && len < PS_LENGTH) enc->data[program].tps[len++] = *tps++;
}

void set_rds_lps(RDSEncoder* enc, const char *lps, uint8_t program) {
	uint8_t i = 0, len = 0;

	enc->state[program].lps_update = 1;
	if(lps[0] == '\0') {
		memset(enc->data[program].lps, 0, LPS_LENGTH);
		return;
	}
	memset(enc->data[program].lps, ' ', LPS_LENGTH);
	while (*lps != 0 && len < LPS_LENGTH) enc->data[program].lps[len++] = *lps++;

	if (len < LPS_LENGTH) {
		enc->state[program].lps_segments = 0;
		enc->data[program].lps[len++] = '\r';
		while (i < len) {
			i += 4;
			enc->state[program].lps_segments++;
		}
	} else enc->state[program].lps_segments = 8;
	if(enc->state[program].lps_segments > 8) enc->state[program].lps_segments = 8; //make sure
}

void set_rds_ptyn(RDSEncoder* enc, const char *ptyn, uint8_t program) {
	uint8_t len = 0;

	enc->state[program].ptyn_update = 1;

	if(ptyn[0] == '\0') {
		memset(enc->data[program].ptyn, 0, PTYN_LENGTH);
		return;
	}

	memset(enc->data[program].ptyn, ' ', PTYN_LENGTH);
	while (*ptyn != 0 && len < PTYN_LENGTH) enc->data[program].ptyn[len++] = *ptyn++;
}

void set_rds_grpseq(RDSEncoder* enc, const char *grpseq, uint8_t program) {
	uint8_t len = 0;

	if(grpseq[0] == '\0') {
		while (DEFAULT_GRPSQC[len] != 0 && len < 32) {
			enc->data[program].grp_sqc[len] = DEFAULT_GRPSQC[len];
			len++;
		}
		return;
	}

	memset(enc->data[program].grp_sqc, 0, 32);
	while (*grpseq != 0 && len < 32) enc->data[program].grp_sqc[len++] = *grpseq++;
}
void set_rds_grpseq2(RDSEncoder* enc, const char *grpseq2, uint8_t program) {
	uint8_t len = 0;

	if(grpseq2[0] == '\0') {
		memset(enc->data[program].grp_sqc_rds2, 0, 32);
		memcpy(enc->data[program].grp_sqc_rds2, enc->data[program].grp_sqc, 32);
		return;
	}

	memset(enc->data[program].grp_sqc_rds2, 0, 32);
	while (*grpseq2 != 0 && len < 32) enc->data[program].grp_sqc_rds2[len++] = *grpseq2++;
}