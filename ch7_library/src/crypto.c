#include <string.h>

int encrypt(char *plaintext, char *ciphertext, int size)
{
	int	i;

	if( size < strlen(plaintext) )
	{
		return -1;
	}

	for(i=0; i<strlen(plaintext); i++)
	{
		ciphertext[i] = plaintext[i]+3;
	}

	return 0;
}

int decrypt(char *ciphertext, char *plaintext, int size)
{
 	int 	i;

 	if( size < strlen(ciphertext) )
 	{
 		return -1;
 	}

 	for(i=0; i<strlen(ciphertext); i++)
 	{
 		plaintext[i] = ciphertext[i]-3;
 	}
 	
	return 0;
}
