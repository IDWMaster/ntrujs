#include <node.h>
#include <node_buffer.h>
#include "lib/include/libntruencrypt/ntru_crypto.h"
#include <fcntl.h>
#ifdef __unix__
#include <unistd.h>
#endif
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#define WINDOWS 1
#include <Windows.h>
#endif
namespace ntru {
#ifdef __unix__
  static int rand_fd;
#endif
#ifdef WINDOWS
  static HCRYPTPROV rand_fd;
#endif
  uint32_t getRandBytes(uint8_t* out,uint32_t numBytes) {
#ifdef __unix__
    read(rand_fd,(void*)out,numBytes);
#endif
#ifdef WINDOWS
	if (!CryptGenRandom(rand_fd, numBytes, (unsigned char*)out)) {
		abort();
	}
#endif
    DRBG_RET(DRBG_OK);
  }
  DRBG_HANDLE rand;
  static DRBG_HANDLE seedDRBG;
  static char *seedPtr;
  static size_t KLENNGTH = 256; // because key size of NTRU_EES743EP1 is 256
using namespace v8;

static uint8_t
get_entropy(
    ENTROPY_CMD  cmd,
    uint8_t     *out)
{
    /* 2k/8 bytes of entropy are needed to instantiate a DRBG with a
     * security strength of k bits. Here k = KLENNGTH.
     */
    static size_t kMaxLength = KLENNGTH * 2 / 8;
    static size_t index;

    if (cmd == INIT) {
        /* Any initialization for a real entropy source goes here. */
        index = 0;
        return 1;
    }

    if (out == NULL)
        return 0;

    if (cmd == GET_NUM_BYTES_PER_BYTE_OF_ENTROPY) {
        /* Here we return the number of bytes needed from the entropy
         * source to obtain 8 bits of entropy.  Maximum is 8.
         */
        *out = 1;                       /* this is a perfectly random source */
        return 1;
    }

    if (cmd == GET_BYTE_OF_ENTROPY) {
        if (index == kMaxLength)
            return 0;                   /* used up all our entropy */

        *out = seedPtr[index++];           /* deliver an entropy byte */
        return 1;
    }
    return 0;
}

static void _GenerateKeyPair(const FunctionCallbackInfo<Value>& args, DRBG_HANDLE drbg) {
  Isolate* isolate = args.GetIsolate();
  uint16_t publen;
  uint16_t privlen;
  ntru_crypto_ntru_encrypt_keygen(drbg, NTRU_EES743EP1, &publen,0,&privlen,0);
  uint8_t* pubkey = new uint8_t[publen];
  uint8_t* privkey = new uint8_t[privlen];
  ntru_crypto_ntru_encrypt_keygen(drbg, NTRU_EES743EP1,&publen,pubkey,&privlen,privkey);
  v8::Local<Object> obj = v8::Object::New(isolate);
  obj->Set(v8::String::NewFromUtf8(isolate,"private"),node::Encode(isolate,(const char*)privkey,privlen,node::encoding::BUFFER));
  obj->Set(v8::String::NewFromUtf8(isolate,"public"),node::Encode(isolate,(const char*)pubkey,publen,node::encoding::BUFFER));
  args.GetReturnValue().Set(obj);
  free(pubkey);
  free(privkey);
}

void GenKey(const FunctionCallbackInfo<Value>& args) {
  if (args[0]->IsUndefined()) {
    _GenerateKeyPair(args, rand);
  } else {
    seedPtr = node::Buffer::Data(args[0]);
    ntru_crypto_drbg_instantiate(KLENNGTH, NULL, 0, (ENTROPY_FN) &get_entropy, &seedDRBG);
    _GenerateKeyPair(args, seedDRBG);
  }
}
void EncryptData(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  size_t dlen = node::Buffer::Length(args[0]);
  char* ptr = node::Buffer::Data(args[0]);
  size_t publen = node::Buffer::Length(args[1]);
  char* pubkey = node::Buffer::Data(args[1]);
  uint16_t outlen;
  
  ntru_crypto_ntru_encrypt(rand,publen,(unsigned char*)pubkey,dlen,(unsigned char*)ptr,&outlen,0);
  
  v8::Local<v8::Value> outbuffer = node::Buffer::New(isolate,outlen).ToLocalChecked();
  ntru_crypto_ntru_encrypt(rand,publen,(unsigned char*)pubkey,dlen,(unsigned char*)ptr,&outlen,(unsigned char*)node::Buffer::Data(outbuffer));
  v8::Local<v8::Object> output = v8::Object::New(isolate);
  output->Set(String::NewFromUtf8(isolate,"buffer"),outbuffer);
  output->Set(String::NewFromUtf8(isolate,"length"),Int32::NewFromUnsigned(isolate,(uint32_t)outlen));
  args.GetReturnValue().Set(output);
  
}

void DecryptData(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  size_t dlen = node::Buffer::Length(args[0]);
  char* ptr = node::Buffer::Data(args[0]);
  size_t privlen = node::Buffer::Length(args[1]);
  char* privkey = node::Buffer::Data(args[1]);
  uint16_t outlen;
  
  ntru_crypto_ntru_decrypt(privlen,(unsigned char*)privkey,dlen,(unsigned char*)ptr,&outlen,0);
  
  v8::Local<v8::Value> outbuffer = node::Buffer::New(isolate,outlen).ToLocalChecked();
  memset(node::Buffer::Data(outbuffer),0,outlen);
  ntru_crypto_ntru_decrypt(privlen,(unsigned char*)privkey,dlen,(unsigned char*)ptr,&outlen,(unsigned char*)node::Buffer::Data(outbuffer));
  v8::Local<v8::Object> output = v8::Object::New(isolate);
  output->Set(String::NewFromUtf8(isolate,"buffer"),outbuffer);
  output->Set(String::NewFromUtf8(isolate,"length"),Int32::NewFromUnsigned(isolate,(uint32_t)outlen));
  args.GetReturnValue().Set(output);
}

void init(Local<Object> exports) {
#ifdef __unix__
  rand_fd = open("/dev/urandom",O_RDONLY);
#endif
#ifdef WINDOWS
  CryptAcquireContextA(&rand_fd, 0, NULL, PROV_RSA_FULL, 0);
#endif
  ntru_crypto_drbg_external_instantiate(getRandBytes,&rand);
  NODE_SET_METHOD(exports, "genKey", GenKey);
  NODE_SET_METHOD(exports,"encrypt",EncryptData);
  NODE_SET_METHOD(exports,"decrypt",DecryptData);
}

NODE_MODULE(addon, init)
}
