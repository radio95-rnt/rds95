#include "fs.h"

void encoder_saveToFile(RDSEncoder *enc) {
	char encoderPath[128];
	char encoderPath_tmp[128];
	snprintf(encoderPath, sizeof(encoderPath), "%s/.rdsEncoder", getenv("HOME"));
	snprintf(encoderPath_tmp, sizeof(encoderPath_tmp), "%s/.rdsEncoder.tmp", getenv("HOME"));

	RDSEncoder tempEncoder;
	FILE *file = fopen(encoderPath, "rb");
	if (file) {
		fread(&tempEncoder, sizeof(RDSEncoder), 1, file);
		fclose(file);
	} else memcpy(&tempEncoder, enc, sizeof(RDSEncoder));

    memcpy(tempEncoder.data, enc->data, sizeof(RDSData) * PROGRAMS);
	tempEncoder.program = enc->program;
	tempEncoder.enabled_streams = enc->enabled_streams;

	RDSEncoderFile rdsEncoderfile = {.file_starter = 225, .file_middle = 160, .file_ender = 95, .program = tempEncoder.program, .enabled_streams = tempEncoder.enabled_streams};
	memcpy(&rdsEncoderfile.data, &tempEncoder.data, sizeof(RDSData)*PROGRAMS);

	rdsEncoderfile.crc = crc16_ccitt((char *)&rdsEncoderfile, offsetof(RDSEncoderFile, crc));

	file = fopen(encoderPath_tmp, "wb");
	if (!file) {
		perror("Error opening file");
		return;
	}
	fwrite(&rdsEncoderfile, sizeof(RDSEncoderFile), 1, file);
	fclose(file);
	rename(encoderPath_tmp, encoderPath);
}

int encoder_loadFromFile(RDSEncoder *enc) {
	char encoderPath[128];
	snprintf(encoderPath, sizeof(encoderPath), "%s/.rdsEncoder", getenv("HOME"));

	RDSEncoderFile rdsEncoderfile;
	FILE *file = fopen(encoderPath, "rb");
	if (!file) {
		perror("Error opening file");
		return 1;
	}
	fread(&rdsEncoderfile, sizeof(rdsEncoderfile), 1, file);
	fclose(file);
	
	if (rdsEncoderfile.file_starter != 225 || rdsEncoderfile.file_ender != 95 || rdsEncoderfile.file_middle != 160) {
		fprintf(stderr, "[RDSENCODER-FILE] Invalid file format\n");
		return 1;
	}

	if (crc16_ccitt((char*)&rdsEncoderfile, offsetof(RDSEncoderFile, crc)) != rdsEncoderfile.crc) {
		fprintf(stderr, "[RDSENCODER-FILE] CRC mismatch! Data may be corrupted\n");
		return 1;
	}

	memcpy(&(enc->data), &(rdsEncoderfile.data), sizeof(RDSData)*PROGRAMS);
	enc->program = rdsEncoderfile.program;
	enc->enabled_streams = rdsEncoderfile.enabled_streams;
	return 0;
}