#ifndef _MSG_H
#define _MSG_H

#define AES_KEY_128		16
#define BUFSIZE			256
#define AES_BLOCK_SIZE	16

enum MSG_TYPE{
	PUBLIC_KEY,
	SECRET_KEY,
	PUBLIC_KEY_REQUEST,
	IV,
	ENCRYPTED_KEY,
	ENCRYPTED_MSG
	};
	
typedef struct _APP_MSG_
{
	int type;
	unsigned char payload[BUFSIZE + AES_BLOCK_SIZE];
	int msg_len;
}APP_MSG;	

#endif
