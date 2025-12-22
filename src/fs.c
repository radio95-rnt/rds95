#include "fs.h"

void encoder_saveToFile(RDSEncoder *enc) {
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

	for (int i = 0; i < PROGRAMS; i++) {
		memcpy(&(enc->data[i]), &(rdsEncoderfile.data[i]), sizeof(RDSData));
		memcpy(&(enc->rtpData[i]), &(rdsEncoderfile.rtpData[i]), sizeof(RDSRTPlusData)*2);
	}
	memcpy(&(enc->encoder_data), &(rdsEncoderfile.encoder_data), sizeof(RDSEncoderData));
	enc->program = rdsEncoderfile.program;
	return 0;
}

int encoder_saved() {
	char encoderPath[128];
	snprintf(encoderPath, sizeof(encoderPath), "%s/.rdsEncoder", getenv("HOME"));
	FILE *file = fopen(encoderPath, "rb");
	if(!file) return 0;
	fclose(file);
	return 1;
}

void Modulator_saveToFile(RDSModulatorParameters *emp) {
	char modulatorPath[128];
	snprintf(modulatorPath, sizeof(modulatorPath), "%s/.rdsModulator", getenv("HOME"));
	FILE *file;
	
	RDSModulatorParameters tempMod;
	RDSModulatorParametersFile tempFile;
	memset(&tempFile, 0, sizeof(tempFile));
	file = fopen(modulatorPath, "rb");
	if (file != NULL) {
		fread(&tempFile, sizeof(RDSModulatorParametersFile), 1, file);
		fclose(file);
	} else {
		memset(&tempFile, 0, sizeof(RDSModulatorParametersFile));
		tempFile.check = 160;
		memcpy(&tempFile.params, emp, sizeof(RDSModulatorParameters));
		tempFile.crc = crc16_ccitt((char*)&tempFile, offsetof(RDSModulatorParametersFile, crc));
	}
	memcpy(&tempMod, &tempFile.params, sizeof(RDSModulatorParameters));
	
	tempMod.level = emp->level;
	tempMod.rdsgen = emp->rdsgen;

	memcpy(&tempFile.params, &tempMod, sizeof(RDSModulatorParameters));
	tempFile.check = 160;
	tempFile.crc = crc16_ccitt((char*)&tempFile, offsetof(RDSModulatorParametersFile, crc));
	
	file = fopen(modulatorPath, "wb");
	if (file == NULL) {
		perror("Error opening file");
		return;
	}
	fwrite(&tempFile, sizeof(RDSModulatorParametersFile), 1, file);
	fclose(file);
}

void Modulator_loadFromFile(RDSModulatorParameters *emp) {
	char modulatorPath[128];
	snprintf(modulatorPath, sizeof(modulatorPath), "%s/.rdsModulator", getenv("HOME"));
	FILE *file = fopen(modulatorPath, "rb");
	if (file == NULL) {
		perror("Error opening file");
		return;
	}
	RDSModulatorParametersFile tempFile;
	memset(&tempFile, 0, sizeof(tempFile));
	fread(&tempFile, sizeof(RDSModulatorParametersFile), 1, file);
	if (tempFile.check != 160) {
		fprintf(stderr, "[RDSMODULATOR-FILE] Invalid file format\n");
		fclose(file);
		return;
	}
	uint16_t calculated_crc = crc16_ccitt((char*)&tempFile, offsetof(RDSModulatorParametersFile, crc));
	if (calculated_crc != tempFile.crc) {
		fprintf(stderr, "[RDSMODULATOR-FILE] CRC mismatch! Data may be corrupted\n");
		fclose(file);
		return;
	}
	memcpy(emp, &tempFile.params, sizeof(RDSModulatorParameters));
	fclose(file);
}

int modulatorsaved() {
	char encoderPath[128];
	snprintf(encoderPath, sizeof(encoderPath), "%s/.rdsModulator", getenv("HOME"));
	FILE *file = fopen(encoderPath, "rb");
	if (file) {
		fclose(file);
		return 1;
	}
	return 0;
}