/*
 decrypt AES128 Advanced Encryption Standard  CTR (counter) implementation
 https://github.com/rweather/arduinolibs/tree/master/libraries/Crypto
*/

#include <Crypto.h>
#include <AES.h>
#include <CTR.h>
#include <string.h>

#define MAX_PLAINTEXT_SIZE 36
#define MAX_CIPHERTEXT_SIZE 36

struct TestVector {
  byte key[16];
  byte plaintext[MAX_PLAINTEXT_SIZE];
  byte ciphertext[MAX_CIPHERTEXT_SIZE];
  byte iv[16];
  size_t size;
};

// Test vectors for AES-128 in CTR mode from RFC 3686.
static TestVector const testVectorAES128CTR = {
  .key = { 0xd4, 0xf1, 0xbb, 0x3a, 0x20, 0x29, 0x07, 0x59,
           0xf0, 0xbc, 0xff, 0xab, 0xcf, 0x4e, 0x69, 0x01 },
  .plaintext = { 0x08, 0x01, 0x12, 0x04, 0x54, 0x65, 0x73, 0x74, 0x48, 0x00 },
  .ciphertext = { 0x2d, 0x73, 0xfe, 0xa3, 0x70, 0x6e, 0xbf, 0x6a, 0x16, 0xe0 },
  .iv = { 0xa5, 0x72, 0xa4, 0xab, 0x00, 0x00, 0x00, 0x00,
          0x88, 0x1b, 0xc2, 0x25, 0x00, 0x00, 0x00, 0x00 },
  .size = 10
};


CTR<AES128> ctraes128;


void cipherEncrypt(Cipher *cipher, const struct TestVector *test) {
  byte output[MAX_CIPHERTEXT_SIZE];
  Serial.print("Encrypt");
  cipher->clear();
  if (!cipher->setKey(test->key, cipher->keySize())) {
    Serial.print("setKey ");
    return false;
  }
  if (!cipher->setIV(test->iv, cipher->ivSize())) {
    Serial.print("setIV ");
    return false;
  }

  memset(output, 0xBA, sizeof(output));
  cipher->encrypt(output, test->plaintext, test->size);

  for (int n = 0; n < test->size; n++) {
    Serial.print(output[n], HEX);
    Serial.print(",");
  }
  Serial.println();
}


void cipherDecrypt(Cipher *cipher, const struct TestVector *test) {
  byte output[MAX_CIPHERTEXT_SIZE];

  Serial.print("Encrypt");

  cipher->setKey(test->key, cipher->keySize());
  cipher->setIV(test->iv, cipher->ivSize());

  cipher->decrypt(output, test->ciphertext, test->size);

  for (int n = 0; n < test->size; n++) {
    Serial.print(output[n], HEX);
    Serial.print(",");
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  Serial.println();
}

void loop() {
  cipherEncrypt(&ctraes128, &testVectorAES128CTR);
  cipherDecrypt(&ctraes128, &testVectorAES128CTR);
  delay(5000);
}
