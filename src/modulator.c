#include "modulator.h"
#include "fs.h"

void init_rds_modulator(RDSModulator* rdsMod, RDSEncoder* enc, uint8_t num_streams) {
	memset(rdsMod, 0, sizeof(RDSModulator));
	rdsMod->params.level = 1.0f;
	rdsMod->params.rdsgen = 1;
	rdsMod->num_streams = num_streams;

	memset(enc, 0, sizeof(RDSEncoder));
	rdsMod->enc = enc;

	rdsMod->data = (RDSModulatorModulationData*)calloc(num_streams, sizeof(RDSModulatorModulationData));
	if (rdsMod->data == NULL) {
		fprintf(stderr, "Error: Could not allocate memory for RDS modulation data\n");
		return;
	}

	for (uint8_t i = 0; i < num_streams; i++) {
		rdsMod->data[i].symbol_shift = i * M_PI / 2.0f;
		rdsMod->data[i].bit_pos = BITS_PER_GROUP;
	}

	if(modulatorsaved()) Modulator_loadFromFile(&rdsMod->params);
	else Modulator_saveToFile(&rdsMod->params);
}

void cleanup_rds_modulator(RDSModulator* rdsMod) {
	if (rdsMod->data) {
		free(rdsMod->data);
		rdsMod->data = NULL;
	}
}

float get_rds_sample(RDSModulator* rdsMod, uint8_t stream) {
	if (stream >= rdsMod->num_streams) return 0.0f;

	rdsMod->data[stream].phase += 1187.5 / RDS_SAMPLE_RATE;

	if (rdsMod->data[stream].phase >= 1.0f) {
		rdsMod->data[stream].phase -= 1.0f;
		if (rdsMod->data[stream].bit_pos == BITS_PER_GROUP) {
			get_rds_bits(rdsMod->enc, rdsMod->data[stream].bit_buffer, stream);
			rdsMod->data[stream].bit_pos = 0;
		}

		rdsMod->data[stream].cur_bit = rdsMod->data[stream].bit_buffer[rdsMod->data[stream].bit_pos++];
		rdsMod->data[stream].prev_output = rdsMod->data[stream].cur_output;
		rdsMod->data[stream].cur_output = rdsMod->data[stream].prev_output ^ rdsMod->data[stream].cur_bit;
	}

	float sample = sinf(M_2PI * rdsMod->data[stream].phase + rdsMod->data[stream].symbol_shift);
	if(rdsMod->data[stream].cur_output == 0) sample = -sample; // do bpsk, if you comment this part out, nothing will be decoded
	
	return sample*rdsMod->params.level*(rdsMod->params.rdsgen > stream ? 1 : 0);
}