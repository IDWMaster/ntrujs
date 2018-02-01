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
using namespace v8;
void GenKey(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  uint16_t publen;
  uint16_t privlen;
  ntru_crypto_ntru_encrypt_keygen(rand,NTRU_EES743EP1,&publen,0,&privlen,0);
  uint8_t* pubkey = new uint8_t[publen];
  uint8_t* privkey = new uint8_t[privlen];
  ntru_crypto_ntru_encrypt_keygen(rand,NTRU_EES743EP1,&publen,pubkey,&privlen,privkey);
  v8::Local<Object> obj = v8::Object::New(isolate);
  obj->Set(v8::String::NewFromUtf8(isolate,"private"),node::Encode(isolate,(const char*)privkey,privlen,node::encoding::BUFFER));
  obj->Set(v8::String::NewFromUtf8(isolate,"public"),node::Encode(isolate,(const char*)pubkey,publen,node::encoding::BUFFER));
  args.GetReturnValue().Set(obj);
  free(pubkey);
  free(privkey);
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
