#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <opencv/highgui.h>
#include <opencv/cxcore.h>
#include <opencv/cv.h>
#include <wiringPi.h>
#include "command.h"
#include "jpegutil.h"

#define PORT 8999
#define PACKET_LENGTH 4
#define BYTE unsigned char
#define BLOCK_SIZE 4096

/*************  GPIO ************/
#define EA  1   //PINS 12
#define IA1 4   //PINS 16
#define IA2 5   //PINS 18

#define EB  0   // PINS 11
#define IB1 2   // PINS 13
#define IB2 3   // PINS 15
/************   END  ************/

int sockfd;
struct sockaddr_in server_addr;
struct sockaddr_in client_addr;
BYTE buffer[4];

/**********************************/
// 采用定长的消息格式定义 7E(头) xx(指令 01 向前 02 左 03 右 04 后 05 stop) xx(数据位，没有用00填充) 1A(尾部)
void process(BYTE*);
void left(BYTE);
void right(BYTE);
void forward(BYTE);
void back(BYTE);
void stop();
void vardump(BYTE* data, long unsigned int length);
void setupPins();

void setupPins(){
	int pins[] = {EA,IA1,IA2,EB,IB1,IB2};
	int i = 0;
	for (i = 0; i < 6; ++i) {
		pinMode(pins[i],OUTPUT);
	}
}


int main(int argc, char **argv) {

	if(wiringPiSetup() == -1){
		perror("raspberry pi setup failed!");
		return -1;
	}

	setupPins();
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		perror("create socket failed");
		return -1;
	}
	memset((void *)&server_addr, 0, sizeof(struct sockaddr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(struct sockaddr))
			== -1) {
		perror("bind port failed");
		return -1;
	}
	if (listen(sockfd, 5) == -1) {
		perror("listen error");
		return -1;
	}
	while (1) {
		socklen_t sockaddrSize = sizeof(struct sockaddr_in);
		int newFd;
		newFd = accept(sockfd, (struct sockaddr *) &client_addr, &sockaddrSize);
		if (newFd == -1) {
			perror("accept error");
			continue;
		}
		printf("Welcome client!\n");
		//receive data

		int fpid = fork();
		if (fpid < 0) {
			perror("fork failed");
		} else if (fpid == 0) {
			int readCount;
			while ((readCount = recv(newFd, buffer, PACKET_LENGTH, 0)) != 0) {
				process(buffer);
			}
			printf("Bye bye...\n");
		} else {
			printf("fork pid excute\n");
			// send message to client
			//open camera
			CvCapture *capture = cvCreateCameraCapture(0);
			cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, 320);
			cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, 240);
			if (!capture) {
				perror("open camera failed!");
				return -1;
			}
			IplImage *frame;
			BYTE* outBuffer;
			long unsigned int outLen;
			while ((frame = cvQueryFrame(capture)) != NULL) {
				ipl2Jpeg(frame, &outBuffer, &outLen);
				int packet =
						outLen % BLOCK_SIZE == 0 ?
								outLen / BLOCK_SIZE : outLen / BLOCK_SIZE + 1;
				//printf("total packet is:%d\n", packet);
				int i;
				for (i = 0; i < packet; i++) {
					BYTE* index = outBuffer + i * BLOCK_SIZE;
					//printf("current index is :%d\n", i * BLOCK_SIZE);
					if (i < packet - 1) {
						send(newFd, index, BLOCK_SIZE, 0);
						//printf("send a block\n");
					} else {
						send(newFd, index, outLen - i * BLOCK_SIZE, 0);
						//printf("send last packet\n");
					}
				}


				if(NULL != outBuffer){
					free(outBuffer);
					outBuffer = NULL;
				}
			}

		}
	}

}

void process(BYTE* data) {
	BYTE header = data[0];
	if (header != 0x7E && data[3] != 0x1A) {
		perror("data packet error!");
		return;
	}
	BYTE cmd = data[1];
	BYTE value = data[2];
	switch (cmd) {
	case FORWARD:
		forward(value);
		//printf("forward value is :0x%x\n", data[2]);
		break;
	case LEFT:
		left(value);
		printf("left value is :0x%x\n", data[2]);
		break;
	case RIGHT:
		right(value);
		printf("right value is :0x%x\n", data[2]);
		break;
	case BACK:
		back(value);
		printf("back value is :0x%x\n", data[2]);
		break;
	case STOP:
		stop();
		printf("stop value is :0x%x\n", data[2]);
		break;
	default:
		printf("unknow command\n");
		break;
	}
}

void left(BYTE data) {

	pwmWrite(EA,data);
	digitalWrite(IA1,LOW);
	digitalWrite(IA2,LOW);

	//right wheel
	digitalWrite(IB1,HIGH);
	digitalWrite(IB2,LOW);
	pwmWrite(EB,0);

}
void right(BYTE data) {

	pwmWrite(EB,data);
	digitalWrite(IB1,LOW);
	digitalWrite(IB2,LOW);

	//right wheel
	digitalWrite(IA1,HIGH);
	digitalWrite(IA2,LOW);
	pwmWrite(EA,0);


}
void forward(BYTE data) {


}
void back(BYTE data) {

}
void stop() {

	digitalWrite(EA,LOW);
	digitalWrite(IA1,LOW);
	digitalWrite(IA2,LOW);
	digitalWrite(EB,LOW);
	digitalWrite(IB1,LOW);
	digitalWrite(IB2,LOW);
}

void vardump(BYTE* data, long unsigned int length) {
	int i;
	for (i = 0; i < length; i++) {
		printf("%02x\n", *(data + i));
	}
}
