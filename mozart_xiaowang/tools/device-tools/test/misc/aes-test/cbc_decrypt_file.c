#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#include "jz_aes_v12.h"

#define AES_BLOCK_SIZE	16
#define DMA_MAX_LEN	4096

int aes_fd;

int cbc_decrypt(FILE *fp_read, FILE *fp_write)
{
	unsigned int key[4] = {0xd55fda32,0x14d99bd7,0x9f7a0ef8,0x972df216};
	unsigned int iv[4] = {0x7f444fd5,0xd2002d29,0x4b96c34d,0xc57d297e};

	struct aes_key aes_key;
	struct aes_data aes_data;
	int ret = 0;
	long fsize;
	int rd_byte = 0, wr_byte = 0;

	aes_key.key = (char *)key;
	aes_key.keylen = 16;
	aes_key.bitmode = AES_KEY_128BIT;
	aes_key.aesmode = AES_MODE_CBC;
	aes_key.iv = (char *)iv;
	aes_key.ivlen = 16;
	aes_key.encrypt = 1;


	ret = ioctl(aes_fd, AES_LOAD_KEY, &aes_key);
	if(ret < 0) {
		printf("ioctl! AES_LOAD_KEY");
	}

	if (fseek(fp_read, 0, SEEK_END) == -1) {
		printf("fseek SEEK_END error!\n");
		return -1;
	}

	fsize = ftell(fp_read);
	if (fsize == -1) {
		printf("ftell!\n");
		return -1;
	}

	if (fseek(fp_read, 0, SEEK_SET) == -1) {
		printf("fseek SEEK_SET error!\n");
		return -1;
	}

	if (fsize == 0){
		printf("fsize error!\n");
		return -1;
	}

	unsigned char input[DMA_MAX_LEN];
	unsigned char output[DMA_MAX_LEN];
	unsigned int align_len = 0;
	unsigned int pad, len, i;

	len = fsize;

	while(len) {
		rd_byte = fread(input, 1, DMA_MAX_LEN, fp_read);
		if (!rd_byte) {
			printf("fread! error\n");
			return -1;
		}

		aes_data.input = input;
		aes_data.input_len = rd_byte;
		aes_data.output = output;

		ret = ioctl(aes_fd, AES_DO_CRYPT, &aes_data);

		len -= rd_byte;

		if (len == 0) {
			pad = output[rd_byte - 1];
			if (pad > AES_BLOCK_SIZE) {
				printf("pad error\n");
				return -1;
			}
			if (!(rd_byte - pad))
				continue;
			wr_byte = fwrite(output, 1, rd_byte - pad, fp_write);
			if (!wr_byte) {
				printf("last fwrite! error\n");
				return -1;
			}
		}
		else {
			wr_byte = fwrite(output, 1, rd_byte, fp_write);
			if (!wr_byte) {
				printf("fwrite! error\n");
				return -1;
			}
		}
	}
}


int main(int argc, char *argv[])
{
	int fd;
	FILE *fp_read, *fp_write;
	char *readpath = NULL;
	char *writepath = NULL;
	clock_t start,end;
	double TheTimes;

	if(argc != 3) {
		printf("please intput correctly arguments!\n");
		return -1;
	}

	readpath = argv[1];
	fp_read = fopen(readpath, "r");
	if (!fp_read) {
		printf("fopen %s!\n",readpath);
		return -1;
	}

	writepath = argv[2];
	fp_write = fopen(writepath, "w+");
	if (!fp_write) {
		printf("fopen %s!\n",writepath);
		return -1;
	}

	fd = open("/dev/jz-aes", O_RDWR);
	if(fd < 0) {
		printf("open error!");
		return -1;
	}

	aes_fd = fd;

	printf("#########CBC DECRYPT########## \n");
	start = clock();
	cbc_decrypt(fp_read, fp_write);
	end = clock();
	TheTimes = (double)(end - start) / CLOCKS_PER_SEC;
	printf("decrypt %f s.\n",TheTimes);

	close(fd);

	fclose(fp_read);
	fclose(fp_write);
	return 0;
}


