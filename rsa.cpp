#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "Hwkeymaster.h"


void HWcrypto::inithwkey()
{
   hw_generate_rsa_keys(2048, &hwkey1);
}

void HWcrypto::gethwkey1(unsigned char** pubkey, size_t* plen)
{
 enum key_type ktype;
 unsigned char* pkeytmp; 
 if(hwkey1 !=NULL) 
 {
    bps_get_pubkey(hwkey1, &pkeytmp, plen, &ktype);
    *pubkey = (unsigned char*)base64(pkeytmp,*plen);
   // fprintf(stderr,"\n Got HW Key1 type %d len %d data %s",ktype,*plen,*pubkey); 
 }
 else
  fprintf(stderr,"\n Failed to get HW Key1"); 
}


int HWcrypto::hw_generate_rsa_keys(int bits, EVP_PKEY ** new_key)
{
  *new_key = NULL;

  if (bits < 2048) {
    return(1);
  }

  EVP_PKEY_CTX * ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
  if (!ctx) {
    return(1);
  }

  if (EVP_PKEY_keygen_init(ctx) <= 0) {
    EVP_PKEY_CTX_free(ctx);
    ctx = NULL;
    return(1);
  }

  if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, bits) <= 0) {
    EVP_PKEY_CTX_free(ctx);
    ctx = NULL;
    return(1);
  }

  if (EVP_PKEY_keygen(ctx, new_key) <= 0) {
    EVP_PKEY_CTX_free(ctx);
    ctx = NULL;
    return(1);
  }

  EVP_PKEY_CTX_free(ctx);
  ctx = NULL;
  return(0);
}

int HWcrypto::bps_get_pubkey(EVP_PKEY * key,
                   unsigned char ** pubkey, size_t * pubkey_len, enum key_type * keytype)
{
  *pubkey = NULL;
  *pubkey_len = 0;
  *keytype = NOKEY;

  // Note this is written to support RSA or DH keys at the moment so check for that
  if (key->type != EVP_PKEY_RSA && key->type != EVP_PKEY_DH) {
    return(1);
  }

  // Call with null buffer gets us the needed buffer size first so we allocate it
  *pubkey_len = i2d_PUBKEY(key, NULL);
  *pubkey = (unsigned char *)malloc(*pubkey_len);
  if (!*pubkey) {
      *pubkey_len = 0;
      return(1);
  }

  // openssl likes to modify the ptr so need to pass in tmp copy
  unsigned char * tmp = *pubkey;
  i2d_PUBKEY(key, &tmp);

  switch (key->type) {
    case EVP_PKEY_RSA:
      *keytype = TOKEN_X509_ENCODED_RSA_PUBLIC;
      break;
    case EVP_PKEY_DH:
      *keytype = TOKEN_X509_ENCODED_DH_PUBLIC;
      break;
  }

  return(0);
}



//base 64
char* HWcrypto::base64( unsigned char *input, int length)
{
	BIO *bmem, *b64;
	BUF_MEM *bptr;

	b64 = BIO_new(BIO_f_base64());
	bmem = BIO_new(BIO_s_mem());
	b64 = BIO_push(b64, bmem);
	BIO_write(b64, input, length);
	BIO_flush(b64);
	BIO_get_mem_ptr(b64, &bptr);

	char *buff = (char *)malloc(bptr->length);
	memcpy(buff, bptr->data, bptr->length-1);
	buff[bptr->length-1] = 0;

	BIO_free_all(b64);

	return buff;
}


char* HWcrypto::unbase64(unsigned char *input, int length,int* outlen)
	{
	BIO *b64, *bmem;

	char *buffer = (char *)malloc(length);
	memset(buffer, 0, length);

	b64 = BIO_new(BIO_f_base64());
	bmem = BIO_new_mem_buf(input, length);
	bmem = BIO_push(b64, bmem);

	*outlen = BIO_read(bmem, buffer, length);

	BIO_free_all(bmem);

	return buffer;
}
#define uint32_t unsigned long


int HWcrypto::bps_decrypt(const unsigned char * msg, size_t msg_len,
                   unsigned char **signature, size_t* siglen,
                   EVP_PKEY * private_key)
{

  EVP_PKEY_CTX * ctx = EVP_PKEY_CTX_new(private_key, NULL);
  if (!ctx) {
    return(1);
  }

  if (EVP_PKEY_decrypt_init(ctx) <= 0) {
    EVP_PKEY_CTX_free(ctx);
    ctx = NULL;
    return(1);
  }

  if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0) {
    EVP_PKEY_CTX_free(ctx);
    ctx = NULL;
    return(1);
  }


  *signature = (unsigned char *)malloc(msg_len*4);
  if (!*signature) {
    EVP_PKEY_CTX_free(ctx);
    ctx = NULL;
    return(1);
  }

  if (EVP_PKEY_decrypt(ctx, *signature, siglen, msg, msg_len) <= 0) {
    EVP_PKEY_CTX_free(ctx);
    ctx = NULL;
    return(1);
  }

  EVP_PKEY_CTX_free(ctx);
  ctx = NULL;
  return(0);
}











