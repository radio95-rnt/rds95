#include "rds.h"

void set_rds_rt(RDSEncoder* enc, const char *rt, uint8_t program) {
	uint8_t i = 0, len = 0;

	enc->state[program].rt_update = 1;

	memset(enc->data[program].rt, ' ', RT_LENGTH);
	while (*rt != '\r' && *rt != 0 && len < RT_LENGTH) enc->data[program].rt[len++] = *rt++;

	while (len > 0 && enc->data[program].rt[len - 1] == ' ') len--;

	if (len < RT_LENGTH) {
		enc->state[program].rt_segments = 0;
		enc->data[program].rt[len++] = '\r';
		while (i < len) {
			i += 4;
			enc->state[program].rt_segments++;
		}
	} else enc->state[program].rt_segments = 16;
	if(enc->state[program].rt_segments > 16) enc->state[program].rt_segments = 16; //make sure
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
	while (*lps != '\r' && *lps != 0 && len < LPS_LENGTH) enc->data[program].lps[len++] = *lps++;

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

void set_rds_grpseq(RDSEncoder* enc, const char *grpseq, size_t size, uint8_t program) {
	uint8_t len = 0;

	enc->data[program].grp_sqc_len = size;
	memset(enc->data[program].grp_sqc, 0, 32);
	while (len < size && len < 32) enc->data[program].grp_sqc[len++] = *grpseq++;
}