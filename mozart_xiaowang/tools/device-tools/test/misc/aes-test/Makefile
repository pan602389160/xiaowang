CROSS   = mipsel-linux-
CC      = $(CROSS)gcc

all: ecb_encrypt ecb_decrypt cbc_encrypt_file cbc_decrypt_file

ecb_encrypt : ecb_encrypt.c
	$(CC) -o $@ ecb_encrypt.c

ecb_decrypt : ecb_decrypt.c
	$(CC) -o $@ ecb_decrypt.c

cbc_encrypt_file : cbc_encrypt_file.c
	$(CC) -o $@ cbc_encrypt_file.c

cbc_decrypt_file : cbc_decrypt_file.c
	$(CC) -o $@ cbc_decrypt_file.c

clean:
	rm ecb_encrypt ecb_decrypt cbc_encrypt_file cbc_decrypt_file
