// ----------------------------------------------------------------------------
// Copyright 2019 PFP Cybersecurity.
//
// SPDX-License-Identifier: Apache-2.0
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------------------------------------------------------
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include<stdio.h>

#define MAX 80
#define PORT 7001
#define SA struct sockaddr

uint8_t PFP_REGISTER_DIGITIZER = 87;
uint8_t PFP_CLK_FREQ = 11; // 11 [MATCH("TimeTrace", "SampleFreq")]
uint8_t PFP_ADC_GET_RAW = 3; // 3 Just send me the RAW ADC vals
uint8_t PFP_ADC_INIT = 0; // 0 Bring up the ADC w/ the ZAP & OCP Stuff
uint8_t PFP_TRIG_CONFIG = 26; // 26 Configure the trigger

struct DeviceData {
	int channel;

	int numberOfTraces;
	int traceLength;

	int sampleRate;
	int digitizer;

	int trigMode;
	int trigSource;

	float trigLevel;
	float trigHyst;

	int trigPercentage;
	int gain;
};

struct DeviceData deviceData;

int bytesToInt(int8_t b0, int8_t b1, int8_t b2, int8_t b3) {
	int value = (b3 << 24) & 0xff000000 | (b2 << 16) & 0x00ff0000
			| (b1 << 8) & 0x0000ff00 | (b0 << 0) & 0x000000ff;
	return value;
}

float bytesToFloat(int8_t b0, int8_t b1, int8_t b2, int8_t b3) {

	float val = 0;
	unsigned long result = 0;
	result |= ((unsigned long) (b0) << 0x18);
	result |= ((unsigned long) (b1) << 0x10);
	result |= ((unsigned long) (b2) << 0x08);
	result |= ((unsigned long) (b3));
	memcpy(&val, &result, 4);

	return val;
}

int8_t * intToBytes(int value) {
	static int8_t b[4];
	b[0] = ((int8_t) (value >> 0));
	b[1] = ((int8_t) (value >> 8));
	b[2] = ((int8_t) (value >> 16));
	b[3] = ((int8_t) (value >> 24));
	return b;
}

int8_t * longToBytes(long value) {
	static int8_t b[8];
	b[0] = (int8_t) ((value >> 0) & 0xFF);
	b[1] = (int8_t) ((value >> 8) & 0xFF);
	b[2] = (int8_t) ((value >> 16) & 0xFF);
	b[3] = (int8_t) ((value >> 24) & 0xFF);
	b[4] = (int8_t) ((value >> 32) & 0xFF);
	b[5] = (int8_t) ((value >> 40) & 0xFF);
	b[6] = (int8_t) ((value >> 48) & 0xFF);
	b[7] = (int8_t) ((value >> 56) & 0xFF);
	return b;
}
long  bytesToLong(int8_t l0,int8_t l1,int8_t l2,int8_t l3,int8_t l4,int8_t l5,int8_t l6,int8_t l7)
{
    long l = ((long) (l0 & 0xff))
             | ((long) (l1& 0xff) << 8)
             | ((long) (l3 & 0xff) << 16)
             | ((long) (l4 & 0xff) << 24)
             | ((long) (l5 & 0xff) << 32)
             | ((long) (l5 & 0xff) << 40)
             | ((long) (l6 & 0xff) << 48)
             | ((long) (l7 & 0xff) << 56);
    return l;
}


int8_t * pfp_emon_create_ack_for_client(int commandType, long numberOfBytes) {
	static int8_t b[24];

	int8_t *totalBytes = longToBytes(numberOfBytes);
	int8_t *successBytes = intToBytes(3);
	int8_t *returnBytes = intToBytes(commandType);

	// EMON HEADER
	b[0] = 69;
	b[1] = 77;
	b[2] = 79;
	b[3] = 78;

	// NUMBER OF BYTES
	b[4] = totalBytes[0];
	b[5] = totalBytes[1];
	b[6] = totalBytes[2];
	b[7] = totalBytes[3];
	b[8] = totalBytes[4];
	b[9] = totalBytes[5];
	b[10] = totalBytes[6];
	b[11] = totalBytes[7];

	// SKIP BYTES
	b[12] = 0;
	b[13] = 0;
	b[14] = 0;
	b[15] = 0;

	// SUCCESS COMMAND
	b[16] = successBytes[0];
	b[17] = successBytes[1];
	b[18] = successBytes[2];
	b[19] = successBytes[3];

	// RETURN COMMAND
	b[20] = returnBytes[0];
	b[21] = returnBytes[1];
	b[22] = returnBytes[2];
	b[23] = returnBytes[3];

	return b;
}

void startClientConnection(int sockfd) {
	int8_t bytes[MAX];
	int16_t rawDataLen = 2048;
	int16_t *rawData = calloc(rawDataLen, sizeof(int16_t));

	for (;;) {
		bzero(bytes, MAX);
		// read the message from client and copy it in buffer
		int len = read(sockfd, bytes, sizeof(bytes));

		if (len < 1) {
			printf("Connection is closed....\n");
			break;
		}

		int commandType = 0;
		int commandLength = 0;

		int8_t *sendBytes = NULL;

		if (len < 0) {
			return;
		}
		if (len == 8) {
			commandType = bytesToInt(bytes[0], bytes[1], bytes[2], bytes[3]);
		} else if (len == 12) {
			commandType = bytesToInt(bytes[0], bytes[1], bytes[2], bytes[3]);
			commandLength = bytesToInt(bytes[4], bytes[5], bytes[6], bytes[7]);
			if (commandType == PFP_REGISTER_DIGITIZER) {

				deviceData.digitizer = bytesToInt(bytes[8], bytes[9], bytes[10],
						bytes[11]);
			} else if (commandType == PFP_CLK_FREQ) {
				deviceData.sampleRate = bytesToInt(bytes[8], bytes[9],
						bytes[10], bytes[11]);
			} else if (commandType == PFP_ADC_GET_RAW) {
				deviceData.channel = bytesToInt(bytes[8], bytes[9], bytes[10],
						bytes[11]);
			}

		}

		else if (len == 40) {
			// Trig Configuration

			commandType = bytesToInt(bytes[0], bytes[1], bytes[2], bytes[3]);
			commandLength = bytesToInt(bytes[4], bytes[5], bytes[6], bytes[7]);

			deviceData.channel = bytesToInt(bytes[8], bytes[9], bytes[10],
					bytes[11]);
			deviceData.traceLength = bytesToInt(bytes[12], bytes[13], bytes[14],
					bytes[15]);

			deviceData.trigMode = bytesToInt(bytes[16], bytes[17], bytes[18],
					bytes[19]);
			deviceData.trigSource = bytesToInt(bytes[20], bytes[21], bytes[22],
					bytes[23]);

			deviceData.trigLevel = bytesToFloat(bytes[24], bytes[25], bytes[26],
					bytes[27]);
			deviceData.trigHyst = bytesToFloat(bytes[28], bytes[29], bytes[30],
					bytes[31]);

			deviceData.trigPercentage = bytesToInt(bytes[32], bytes[33],
					bytes[34], bytes[35]);
			deviceData.gain = bytesToInt(bytes[36], bytes[37], bytes[38],
					bytes[39]);

		}

		// Got command form client. Send back header
		if (commandType == PFP_ADC_INIT) {
			printf("Got init\n");
			sendBytes = pfp_emon_create_ack_for_client(commandType, 0);
		} else if (commandType == PFP_REGISTER_DIGITIZER) {
			printf("Got digitizer\n");
			sendBytes = pfp_emon_create_ack_for_client(commandType, 0);
		} else if (commandType == PFP_TRIG_CONFIG) {
			printf("Got trig config\n");
			sendBytes = pfp_emon_create_ack_for_client(commandType,
					deviceData.traceLength * 2);
		} else if (commandType == PFP_CLK_FREQ) {
			printf("Got clock freq\n");
			sendBytes = pfp_emon_create_ack_for_client(commandType, 0);
		} else if (commandType == PFP_ADC_GET_RAW) {
			printf("Got get raw\n");
			sendBytes = pfp_emon_create_ack_for_client(commandType,
					deviceData.traceLength * 2);
		}

		if (sendBytes != NULL) {
			write(sockfd, sendBytes, 3 * sizeof(sendBytes));
		}

		if (commandType == PFP_ADC_GET_RAW) {

			usleep(50);

			////////////////////////////////////////////////////////////////////
			// This where you read the adc and send in the raw data.
			// For this example we only send
			/////////////////////////////////////////////////////////////////////
			if (deviceData.traceLength != rawDataLen) {
				rawDataLen = deviceData.traceLength;
				rawData = calloc(rawDataLen, sizeof(int16_t));
			}
			for (int i = 0; i < rawDataLen; i++) {
				rawData[i] = i;
			}
			if (sendBytes != NULL) {
				write(sockfd, rawData, rawDataLen * sizeof(int16_t));
			}
		}
	}

	free(rawData);
}

int main() {

	int sockfd, connfd, len;
	struct sockaddr_in servaddr, cli;

	while (1) {
		// socket create and verification
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd == -1) {
			printf("socket creation failed...\n");
			exit(0);
		} else {
			//printf("Socket successfully created..\n");
		}
		bzero(&servaddr, sizeof(servaddr));

		// assign IP, PORT
		servaddr.sin_family = AF_INET;
		servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
		servaddr.sin_port = htons(PORT);

		// Binding newly created socket to given IP and verification
		if ((bind(sockfd, (SA*) &servaddr, sizeof(servaddr))) != 0) {
			printf("socket bind failed...\n");
			exit(0);
		} else {
			//	printf("Socket successfully binded..\n");
		}

		// Now server is ready to listen and verification
		if ((listen(sockfd, 5)) != 0) {
			//printf("Listen failed...\n");
			exit(0);
		} else {
			printf("Emon server waiting for client on port %i\n",PORT);
		}
		len = sizeof(cli);

		// Accept the data packet from client and verification
		connfd = accept(sockfd, (SA*) &cli, &len);
		if (connfd < 0) {
			printf("server acccept failed...\n");
			exit(0);
		} else {
			printf("server acccept the client...\n");
		}

		// Function for chatting between client and server
		startClientConnection(connfd);

		// After chatting close the socket
		close(connfd);
		close(sockfd);
	}
}
