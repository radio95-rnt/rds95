#include "rds_fs.h"

void saveToFile(RDSEncoder *enc) {
	char encoderPath[128];
	snprintf(encoderPath, sizeof(encoderPath), "%s/.rdsEncoder", getenv("HOME"));

	RDSEncoder tempEncoder;
	FILE *file = fopen(encoderPath, "rb");
	if (file) {
		fread(&tempEncoder, sizeof(RDSEncoder), 1, file);
		fclose(file);
	} else memcpy(&tempEncoder, enc, sizeof(RDSEncoder));

    memcpy(tempEncoder.data, enc->data, sizeof(RDSData) * PROGRAMS);
    memcpy(tempEncoder.rtpData, enc->rtpData, sizeof(RDSRTPlusData) * PROGRAMS * 2);
    memcpy(&tempEncoder.encoder_data, &enc->encoder_data, sizeof(RDSEncoderData));
	tempEncoder.program = enc->program;

	RDSEncoderFile rdsEncoderfile = {.file_starter = 225, .file_middle = 160, .file_ender = 95, .program = tempEncoder.program};
	memcpy(&rdsEncoderfile.data[enc->program], &tempEncoder.data[enc->program], sizeof(RDSData));
	memcpy(&rdsEncoderfile.rtpData[enc->program], &tempEncoder.rtpData[enc->program], sizeof(RDSRTPlusData) * 2);
	memcpy(&rdsEncoderfile.encoder_data, &tempEncoder.encoder_data, sizeof(RDSEncoderData));

	rdsEncoderfile.crc = crc16_ccitt((char *)&rdsEncoderfile, offsetof(RDSEncoderFile, crc));

	file = fopen(encoderPath, "wb");
	if (!file) {
		perror("Error opening file");
		return;
	}
	fwrite(&rdsEncoderfile, sizeof(RDSEncoderFile), 1, file);
	fclose(file);
}

void loadFromFile(RDSEncoder *enc) {
	char encoderPath[128];
	snprintf(encoderPath, sizeof(encoderPath), "%s/.rdsEncoder", getenv("HOME"));

	RDSEncoderFile rdsEncoderfile;
	FILE *file = fopen(encoderPath, "rb");
	if (!file) {
		perror("Error opening file");
		return;
	}
	fread(&rdsEncoderfile, sizeof(rdsEncoderfile), 1, file);
	fclose(file);
	
	if (rdsEncoderfile.file_starter != 225 || rdsEncoderfile.file_ender != 95 || rdsEncoderfile.file_middle != 160) {
		fprintf(stderr, "[RDSENCODER-FILE] Invalid file format\n");
		return;
	}

	if (crc16_ccitt((char*)&rdsEncoderfile, offsetof(RDSEncoderFile, crc)) != rdsEncoderfile.crc) {
		fprintf(stderr, "[RDSENCODER-FILE] CRC mismatch! Data may be corrupted\n");
		return;
	}

	for (int i = 0; i < PROGRAMS; i++) {
		memcpy(&(enc->data[i]), &(rdsEncoderfile.data[i]), sizeof(RDSData));
		memcpy(&(enc->rtpData[i]), &(rdsEncoderfile.rtpData[i]), sizeof(RDSRTPlusData)*2);
	}
	memcpy(&(enc->encoder_data), &(rdsEncoderfile.encoder_data), sizeof(RDSEncoderData));
	enc->program = rdsEncoderfile.program;
}

int isFileSaved() {
	char encoderPath[128];
	snprintf(encoderPath, sizeof(encoderPath), "%s/.rdsEncoder", getenv("HOME"));
	FILE *file = fopen(encoderPath, "rb");
	if(!file) return 0;
	fclose(file);
	return 1;
}