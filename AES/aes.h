#ifndef AES
#define AES

int aes_encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *ciphertext);
int aes_decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *plaintext);

#endif