#include "rds.h"

void set_rds_rt1(RDSEncoder* enc, const char *rt1) {
	uint8_t i = 0, len = 0;

	enc->state[enc->program].rt_text_timeout_state = enc->data[enc->program].rt_text_timeout;

	enc->state[enc->program].rt_update = 1;

	memset(enc->data[enc->program].rt1, ' ', RT_LENGTH);
	while (*rt1 != 0 && len < RT_LENGTH) enc->data[enc->program].rt1[len++] = *rt1++;

	while (len > 0 && enc->data[enc->program].rt1[len - 1] == ' ') len--;

	if (len < RT_LENGTH) {
		enc->state[enc->program].rt_segments = 0;
		enc->data[enc->program].rt1[len++] = '\r';
		while (i < len) {
			i += 4;
			enc->state[enc->program].rt_segments++;
		}
	} else enc->state[enc->program].rt_segments = 16;
}

void set_rds_rt2(RDSEncoder* enc, const char *rt2) {
	uint8_t i = 0, len = 0;

	enc->state[enc->program].rt2_update = 1;

	memset(enc->data[enc->program].rt2, ' ', RT_LENGTH);
	while (*rt2 != 0 && len < RT_LENGTH) enc->data[enc->program].rt2[len++] = *rt2++;

	while (len > 0 && enc->data[enc->program].rt2[len - 1] == ' ') len--;

	if (len < RT_LENGTH) {
		enc->state[enc->program].rt2_segments = 0;
		enc->data[enc->program].rt2[len++] = '\r';
		while (i < len) {
			i += 4;
			enc->state[enc->program].rt2_segments++;
		}
	} else enc->state[enc->program].rt2_segments = 16;
}

void set_rds_ps(RDSEncoder* enc, const char *ps) {
	uint8_t len = 0;

	enc->state[enc->program].ps_update = 1;
	memset(enc->data[enc->program].ps, ' ', PS_LENGTH);
	while (*ps != 0 && len < PS_LENGTH) enc->data[enc->program].ps[len++] = *ps++;
}

void set_rds_tps(RDSEncoder* enc, const char *tps) {
	uint8_t len = 0;

	enc->state[enc->program].tps_update = 1;
	if(tps[0] == '\0') {
		memset(enc->data[enc->program].tps, 0, PS_LENGTH);
		return;
	}

	memset(enc->data[enc->program].tps, ' ', PS_LENGTH);
	while (*tps != 0 && len < PS_LENGTH) enc->data[enc->program].tps[len++] = *tps++;
}

void set_rds_lps(RDSEncoder* enc, const char *lps) {
	uint8_t i = 0, len = 0;

	enc->state[enc->program].lps_update = 1;
	if(lps[0] == '\0') {
		memset(enc->data[enc->program].lps, 0, LPS_LENGTH);
		return;
	}
	memset(enc->data[enc->program].lps, ' ', LPS_LENGTH);
	while (*lps != 0 && len < LPS_LENGTH) enc->data[enc->program].lps[len++] = *lps++;

	if (len < LPS_LENGTH) {
		enc->state[enc->program].lps_segments = 0;
		enc->data[enc->program].lps[len++] = '\r';
		while (i < len) {
			i += 4;
			enc->state[enc->program].lps_segments++;
		}
	} else enc->state[enc->program].lps_segments = 8;
}

void set_rds_ert(RDSEncoder* enc, const char *ert) {
	uint8_t i = 0, len = 0;

	enc->state[enc->program].ert_update = 1;

	memset(enc->data[enc->program].ert, ' ', ERT_LENGTH);
	while (*ert != 0 && len < ERT_LENGTH) enc->data[enc->program].ert[len++] = *ert++;

	while (len > 0 && enc->data[enc->program].ert[len - 1] == ' ') len--;

	if (len < ERT_LENGTH) {
		enc->state[enc->program].ert_segments = 0;
		enc->data[enc->program].ert[len++] = '\r';
		while (i < len) {
			i += 4;
			enc->state[enc->program].ert_segments++;
		}
	} else enc->state[enc->program].ert_segments = 32;
}

inline void set_rds_rtplus_tags(RDSEncoder* enc, uint8_t *tags) {
	enc->rtpData[enc->program][0].type[0] = tags[0] & 0x3f;
	enc->rtpData[enc->program][0].start[0] = tags[1] & 0x3f;
	enc->rtpData[enc->program][0].len[0] = tags[2] & 0x3f;
	enc->rtpData[enc->program][0].type[1] = tags[3] & 0x3f;
	enc->rtpData[enc->program][0].start[1] = tags[4] & 0x3f;
	enc->rtpData[enc->program][0].len[1] = tags[5] & 0x1f;

	TOGGLE(enc->rtpState[enc->program][0].toggle);
	enc->rtpData[enc->program][0].running = 1;
	enc->rtpData[enc->program][0].enabled = 1;
}

inline void set_rds_ertplus_tags(RDSEncoder* enc, uint8_t *tags) {
	enc->rtpData[enc->program][1].type[0] = tags[0] & 0x3f;
	enc->rtpData[enc->program][1].start[0] = tags[1] & 0x3f;
	enc->rtpData[enc->program][1].len[0] = tags[2] & 0x3f;
	enc->rtpData[enc->program][1].type[1] = tags[3] & 0x3f;
	enc->rtpData[enc->program][1].start[1] = tags[4] & 0x3f;
	enc->rtpData[enc->program][1].len[1] = tags[5] & 0x1f;

	TOGGLE(enc->rtpState[enc->program][1].toggle);
	enc->rtpData[enc->program][1].running = 1;
	enc->rtpData[enc->program][1].enabled = 1;
}

void set_rds_ptyn(RDSEncoder* enc, const char *ptyn) {
	uint8_t len = 0;

	enc->state[enc->program].ptyn_update = 1;

	if(ptyn[0] == '\0') {
		memset(enc->data[enc->program].ptyn, 0, PTYN_LENGTH);
		return;
	}

	memset(enc->data[enc->program].ptyn, ' ', PTYN_LENGTH);
	while (*ptyn != 0 && len < PTYN_LENGTH) enc->data[enc->program].ptyn[len++] = *ptyn++;
}

void set_rds_grpseq(RDSEncoder* enc, const char *grpseq) {
	uint8_t len = 0;

	if(grpseq[0] == '\0') {
		while (DEFAULT_GRPSQC[len] != 0 && len < 24) {
			enc->data[enc->program].grp_sqc[len] = DEFAULT_GRPSQC[len];
			len++;
		}
		return;
	}

	memset(enc->data[enc->program].grp_sqc, 0, 24);
	while (*grpseq != 0 && len < 24) enc->data[enc->program].grp_sqc[len++] = *grpseq++;
}
void set_rds_grpseq2(RDSEncoder* enc, const char *grpseq2) {
	uint8_t len = 0;

	if(grpseq2[0] == '\0') {
		memset(enc->data[enc->program].grp_sqc_rds2, 0, 24);
		memcpy(enc->data[enc->program].grp_sqc_rds2, enc->data[enc->program].grp_sqc, 24);
		return;
	}

	memset(enc->data[enc->program].grp_sqc_rds2, 0, 24);
	while (*grpseq2 != 0 && len < 24) enc->data[enc->program].grp_sqc_rds2[len++] = *grpseq2++;
}