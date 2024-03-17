#include <stdio.h>
#include <string.h>
#include "crypto.h"

int main(int argc, char *argv[])
{
	char	*message = "asdasdfsdf";
	char	ciphertext[64];
	char	plaintext[64];

	memset(ciphertext, 0, sizeof(ciphertext));
 	if( encrypt(message, ciphertext, sizeof(ciphertext))< 0 )
 	{
 		printf("encrypt plaintext failure\n");
 		return -1;
 	}
 	
	printf("encrypt ciphertext: %s\n", ciphertext);

	memset(plaintext, 0, sizeof(plaintext));
 	if( decrypt(ciphertext, plaintext, sizeof(plaintext))< 0 )
 	{
 		printf("decrypt ciphertext failure\n");
 		return -1;
 	}
 	printf("decrypt plaintext: %s\n", plaintext);
 	
	return 0;
}
