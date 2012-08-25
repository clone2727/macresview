/* macresview - A simple Mac resource fork dumper
 *
 * macresview is the legal property of its developers, whose names can be
 * found in the AUTHORS file distributed with this source
 * distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include <assert.h>

#include "macresfork.h"

enum RunMode {
	kRunModeUnk,
	kRunModeList,
	kRunModeDump,
	kRunModeConvert
};

RunMode parseMode(const char *modeDesc) {
	if (!strcmp(modeDesc, "list"))
		return kRunModeList;
	else if (!strcmp(modeDesc, "dump"))
		return kRunModeDump;
	else if (!strcmp(modeDesc, "convert"))
		return kRunModeConvert;

	return kRunModeUnk;
}

std::string addExtension(std::string fileName, const std::string &extension) {
	if (fileName.size() > 4 && !memcmp(&fileName[fileName.size() - 4], ".dat", 4))
		fileName = std::string(fileName.c_str(), fileName.size() - 4);

	if (fileName.size() > extension.size() && compareStringIgnoreCase(&fileName[fileName.size() - extension.size()], extension.c_str()))
		fileName += extension;

	return fileName;
}

bool outputDataPair(DataPair *pair, const char *filename) {
	if (!pair || !filename)
		return false;

	FILE *output = fopen(filename, "wb");

	if (!output)
		return false;

	fwrite(pair->data, 1, pair->length, output);
	fflush(output);
	fclose(output);
	return true;
}

bool outputPICT(DataPair *pair, std::string fileName) {
	if (!pair || fileName.empty())
		return false;

	fileName = addExtension(fileName, ".pict");

	FILE *output = fopen(fileName.c_str(), "wb");

	if (!output) {
		fprintf(stderr, "Could not open '%s' for writing\n", fileName.c_str());
		return false;
	}

	// Output the 512 byte zero header
	// (The only difference between resource fork PICTs and normal file PICTs)
	for (int i = 0; i < 512; i++)
		writeByte(output, 0);

	fwrite(pair->data, 1, pair->length, output);
	fflush(output);
	fclose(output);
	return true;
}

bool outputMacSnd(DataPair *data, std::string fileName) {
	if (!data || fileName.empty())
		return false;

	uint16 sndType = READ_UINT16_BE(data->data);

	if (sndType != 1 && sndType != 2) {
		fprintf(stderr, "Unknown snd format type = %d\n", sndType);
		return false;
	}

	uint32 soundHeaderOffset = 0;

	if (sndType == 1) {
		soundHeaderOffset = READ_UINT32_BE(data->data + 16);
	} else {
		if (READ_UINT16_BE(data->data + 2) != 0)
			return false;

		if (READ_UINT16_BE(data->data + 4) != 1)
			return false;

		if (READ_UINT16_BE(data->data + 6) != 0x8050 && READ_UINT16_BE(data->data + 6) != 0x8051)
			return false;

		soundHeaderOffset = READ_UINT32_BE(data->data + 10);
	}

	if (READ_UINT32_BE(data->data + soundHeaderOffset) != 0)
		return false;

	uint32 length = READ_UINT32_BE(data->data + soundHeaderOffset + 4);
	uint16 audioRate = READ_UINT16_BE(data->data + soundHeaderOffset + 8);

	if (*(data->data + soundHeaderOffset + 20) != 0)
		return false;

	fileName = addExtension(fileName, ".wav");

	FILE *output = fopen(fileName.c_str(), "wb");

	if (!output) {
		fprintf(stderr, "Could not open '%s' for writing\n", fileName.c_str());
		return false;
	}

	writeUint32BE(output, 'RIFF');
	writeUint32LE(output, length + 44);
	writeUint32BE(output, 'WAVE');
	writeUint32BE(output, 'fmt ');
	writeUint32LE(output, 16);
	writeUint16LE(output, 1);
	writeUint16LE(output, 1);
	writeUint32LE(output, audioRate);
	writeUint32LE(output, audioRate);
	writeUint16LE(output, 1);
	writeUint16LE(output, 8);
	writeUint32BE(output, 'data');
	writeUint32LE(output, length);

	fwrite(data->data + soundHeaderOffset + 22, 1, length, output);
	fflush(output);
	fclose(output);
	return true;
}

void doMode(ResourceFork &resFork, const char *modeDesc) {
	RunMode mode = parseMode(modeDesc);

	if (mode == kRunModeUnk) {
		fprintf(stderr, "Unknown mode '%s'", modeDesc);
		return;
	}

	std::vector<uint32> typeList = resFork.getTagArray();

	for (uint32 i = 0; i < typeList.size(); i++) {
		std::vector<uint16> idList = resFork.getIDArray(typeList[i]);
		
		for (uint32 j = 0; j < idList.size(); j++) {
			if (mode == kRunModeList) {
				printf("%c%c%c%c %04x", typeList[i] >> 24, (typeList[i] >> 16) & 0xff, (typeList[i] >> 8) & 0xff, typeList[i] & 0xff, idList[j]);

				std::string filename = resFork.getFilename(typeList[i], idList[j]);

				if (!filename.empty())
					printf(" - %s", filename.c_str());

				printf("\n");
			} else if (mode == kRunModeConvert) {
				if (typeList[i] == 'PICT') {
					DataPair *pair = resFork.getResource(typeList[i], idList[j]);
					outputPICT(pair, resFork.createOutputFilename(typeList[i], idList[j]));
					delete pair;
				} else if (typeList[i] == 'snd ') {
					DataPair *pair = resFork.getResource(typeList[i], idList[j]);
					outputMacSnd(pair, resFork.createOutputFilename(typeList[i], idList[j]));
					delete pair;
				}
			} else {
				DataPair *pair = resFork.getResource(typeList[i], idList[j]);
				outputDataPair(pair, resFork.createOutputFilename(typeList[i], idList[j]));
				delete pair;
			}
		}
	}

	printf("\nAll done!\n");
}

void printUsage(const char *appName) {
	printf("Usage: %s <mode> <file name>\n", appName);
	printf("\n");
	printf("Valid Modes:\n");
	printf("================================================================================\n");
	printf("\tlist\t\t\tList all resources in the resource fork.\n");
	printf("\tdump\t\t\tDump all resources (as-is) in the resource\n\t\t\t\tfork.\n");
	printf("\tconvert\t\t\tConvert all known resources types in the\n\t\t\t\tresource fork.\n");
	printf("\n");
	printf("Currently, the 'convert' mode will dump any PICT resource as a proper PICT file\n");
	printf("and dumps snd resources as wave files.\n");
}

#define MACRESVIEW_VERSION "0.0.1"

int main(int argc, const char **argv) {
	printf("\nmacresview " MACRESVIEW_VERSION " - Mac Resource Fork Viewer\n");
	printf("Examines Mac resource forks and extracts/converts certain resources\n");
	printf("Written by Matthew Hoops (clone2727)\n");
	printf("Based on ScummVM code\n");
	printf("See COPYING for the license\n\n");

	if (argc < 3) {
		printUsage(argv[0]);
		return 0;
	}
	
	ResourceFork resFork;
	if (!resFork.load(argv[2])) {
		printf("Failed to open file '%s'\n", argv[2]);
		return -1;
	}

	doMode(resFork, argv[1]);
	return 0;
}
