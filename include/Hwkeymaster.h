#ifndef _HWKEYMASTER
#define _HWKEYMASTER

#include <stdio.h>
#include <openssl/aes.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>

enum key_type {
    NOKEY = 1,
    TOKEN_X509_ENCODED_RSA_PUBLIC,
    TOKEN_EVP_RSA_KEYPAIR,
    TOKEN_X509_ENCODED_DH_PUBLIC
};

class HWcrypto
{

private:
        HWcrypto() {}

public:
	static HWcrypto& getInstance()
	{ 
	  static HWcrypto instance;
	  return instance;
	}

	EVP_PKEY *hwkey1 =NULL;

	//Crypto API
	void inithwkey();
	void gethwkey1(unsigned char** pubkey, size_t* plen);

	int bps_get_pubkey(EVP_PKEY * key, unsigned char ** pubkey, size_t * pubkey_len, enum key_type * keytype);
	int hw_generate_rsa_keys(int bits, EVP_PKEY ** new_key);

	char *base64( unsigned char *input, int length);
	char *unbase64(unsigned char *input, int length,int* outlen);

	int bps_decrypt(const unsigned char * msg, size_t msg_len,
                   unsigned char ** signature, size_t* siglen,
                   EVP_PKEY * private_key);


};
#endif

