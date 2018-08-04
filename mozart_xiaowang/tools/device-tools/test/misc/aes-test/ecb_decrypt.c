#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "jz_aes_v12.h"


int aes_fd;


void ecb_decrypt()
{
	unsigned int key[4] =  {0xff565984, 0xe4bb8295, 0xad20496f, 0x9b53e899};
//	unsigned int iv[4] = {0x7f444fd5,0xd2002d29,0x4b96c34d,0xc57d297e};
	unsigned int input[4] = {0x661b2c7a, 0x4c6b5ec2, 0xd58882bc, 0xd51cd190};
	unsigned int output[4] = {0};
	unsigned int src[4] = {0x69cfa45b, 0x18a1eb8d, 0x6c62fba9, 0xda051983};


	struct aes_key aes_key;
	struct aes_data aes_data;
	int ret = 0;

	aes_key.key = (unsigned char *)key;
	aes_key.keylen = 16;
	aes_key.bitmode = AES_KEY_128BIT;
	aes_key.aesmode = AES_MODE_ECB;
	aes_key.encrypt = 1;
//	aes_key.iv = iv;
//	aes_key.ivlen = 16;


	ret = ioctl(aes_fd, AES_LOAD_KEY, &aes_key);
	if(ret < 0) {
		perror("ioctl! AES_LOAD_KEY");
	}

	aes_data.input = (unsigned char *)input;
	aes_data.input_len = 16;
	aes_data.output = (unsigned char *)output;

	ret = ioctl(aes_fd, AES_DO_CRYPT, &aes_data);

	int i;
	for(i = 0; i < aes_data.input_len / 4; i++) {
		printf("decrypt-output[%d]: %x ,src[%d]: %x \n", i, output[i], i, src[i]);
	}

}

int main()
{
	int fd = open("/dev/jz-aes", O_RDWR);
	if(fd < 0) {
		perror("open!");
		return -1;
	}

	printf("#########ECB DECRYPT##########xxxxxxx \n");
	aes_fd = fd;

	ecb_decrypt();

	close(fd);
}


