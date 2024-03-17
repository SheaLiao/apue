#ifndef _CRYPTO_H_
#define _CRYPTO_H_

extern int encrypt(char *plaintext, char *ciphertext, int size);
extern int decrypt(char *ciphertext, char *plaintext, int size); 

#endif
