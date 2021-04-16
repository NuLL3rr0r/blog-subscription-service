/**
 * @file
 * @author  Mamadou Babaei <info@babaei.net>
 * @version 0.1.0
 *
 * @section LICENSE
 *
 * (The MIT License)
 *
 * Copyright (c) 2016 - 2021 Mamadou Babaei
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * @section DESCRIPTION
 *
 * Provides AES and SHA512 cryptographic operations in addition to fast Base64
 * encoding or decoding functions.
 */


#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <utility>
#include <cassert>
#include <boost/algorithm/string.hpp>
#if defined ( _WIN32 )
#include <windows.h>
//#include <cryptopp/dll.h>     // msvc-shared only
#endif  // defined ( _WIN32 )
#include <cryptopp/aes.h>
#include <cryptopp/ccm.h>
#include <cryptopp/cryptlib.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>
#include <cryptopp/sha.h>
#include <b64/decode.h>
#include <b64/encode.h>
#include <sodium.h>
#include "make_unique.hpp"
#include "Crypto.hpp"
#include "Log.hpp"

#define     UNKNOWN_ERROR           "Unknown error!"

using namespace std;
using namespace boost;
using namespace CryptoPP;
using namespace CoreLib;

struct Crypto::Impl
{
public:
    Crypto::Byte *Key;
    std::size_t KeyLen;
    Crypto::Byte *IV;
    std::size_t IVLen;

public:
    Impl();
    ~Impl();

};

void Crypto::Initialize()
{
    LOG_INFO("Initializing CoreLib::Crypto...");

    LOG_INFO("Initializing crypto module sodium...");

    /// sodium_init()
    /// returns 0 on success,
    /// -1 on failure,
    /// and 1 is the library had already been initialized.
    int ret = sodium_init();
    assert(ret == 0);

    LOG_INFO("Crypto module sodium initialized successfully!");

    LOG_INFO("CoreLib::Crypto initialized successfully!");
}

bool Crypto::Encrypt(const std::string &plainText, std::string &out_encodedText,
                     const Byte *key, const std::size_t keyLen, const Byte *iv, const std::size_t ivLen)
{
    string err;
    return Crypto::Encrypt(plainText, out_encodedText, err, key, keyLen, iv, ivLen);
}

bool Crypto::Encrypt(const std::string &plainText, std::string &out_encodedText,
                     std::string &out_error,
                     const Byte *key, const std::size_t keyLen, const Byte *iv, const std::size_t ivLen)
{
    try {
        CBC_Mode<AES>::Encryption enc;
        enc.SetKeyWithIV(key, keyLen, iv, ivLen);

        string cipher;
        StringSource(plainText, true,
                     new StreamTransformationFilter(enc, new StringSink(cipher)));

        string encoded;
        StringSource(cipher, true, new HexEncoder(new StringSink(encoded)));
        out_encodedText.assign(std::move(encoded));

        return true;
    }

    catch (const CryptoPP::Exception &ex) {
        out_error.assign(ex.what());
    }

    catch (const std::exception &ex) {
        out_error.assign(ex.what());
    }

    catch (...) {
        out_error.assign(UNKNOWN_ERROR);
    }

    return false;
}

bool Crypto::Decrypt(const std::string &cipherText, std::string &out_recoveredText,
                     const Byte *key, const std::size_t keyLen, const Byte *iv, const std::size_t ivLen)
{
    string err;
    return Crypto::Decrypt(cipherText, out_recoveredText, err, key, keyLen, iv, ivLen);
}

bool Crypto::Decrypt(const std::string &cipherText, std::string &out_recoveredText,
                     std::string &out_error,
                     const Byte *key, const std::size_t keyLen, const Byte *iv, const std::size_t ivLen)
{
    try {
        CBC_Mode<AES>::Decryption dec;
        dec.SetKeyWithIV(key, keyLen, iv, ivLen);

        string cipher;
        StringSource(cipherText, true, new HexDecoder(new StringSink(cipher)));

        string decoded;
        StringSource(cipher, true,
                     new StreamTransformationFilter(dec, new StringSink(decoded)));
        out_recoveredText.assign(std::move(decoded));

        return true;
    }

    catch (const CryptoPP::Exception &ex) {
        out_error.assign(ex.what());
    }

    catch (const std::exception &ex) {
        out_error.assign(ex.what());
    }

    catch (...) {
        out_error.assign(UNKNOWN_ERROR);
    }

    return false;
}

bool Crypto::Hash(const std::string &text, std::string &out_digest)
{
    string err;
    return Hash(text, out_digest, err);
}

bool Crypto::Hash(const std::string &text, std::string &out_digest,
                  std::string &out_error)
{
    try {
        SHA512 hash;

        string digest;
        StringSource(text, true,
                     new HashFilter(hash, new HexEncoder(new StringSink(digest))));
        out_digest.assign(std::move(digest));

        return true;
    }

    catch (const CryptoPP::Exception &ex) {
        out_error.assign(ex.what());
    }

    catch (const std::exception &ex) {
        out_error.assign(ex.what());
    }

    catch (...) {
        out_error.assign(UNKNOWN_ERROR);
    }

    return false;
}

int Crypto::Base64Decode(const char value)
{
    base64::decoder decoder;
    return decoder.decode(value);
}

int Crypto::Base64Decode(const char *code, const int length, char *out_plainText)
{
    base64::decoder decoder;
    return decoder.decode(code, length, out_plainText);
}

void Crypto::Base64Decode(std::istream &inputStream, std::ostream &outputStream)
{
    base64::decoder decoder;
    decoder.decode(inputStream, outputStream);
}

int Crypto::Base64Encode(char value)
{
    base64::encoder encoder;
    return encoder.encode(value);
}

int Crypto::Base64Encode(const char *code, const int length, char *out_plainText)
{
    base64::encoder encoder;
    return encoder.encode(code, length, out_plainText);
}

int Crypto::Base64EncodeBlockEnd(char *out_plainText)
{
    base64::encoder encoder;
    return encoder.encode_end(out_plainText);
}

void Crypto::Base64Encode(std::istream &inputStream, std::ostream &outputStream)
{
    base64::encoder encoder;
    encoder.encode(inputStream, outputStream);
}

bool Crypto::Argon2(const std::string &passwd, std::string &out_hashedPasswd,
                    const Argon2OpsLimit &opsLimit, const Argon2MemLimit &memLimit)
{
    out_hashedPasswd.clear();
    char hashed[crypto_pwhash_STRBYTES];

    if (crypto_pwhash_str(hashed, passwd.c_str(), passwd.size(),
                          static_cast<unsigned long long>(opsLimit),
                          static_cast<std::size_t>(memLimit)) != 0) {
        // out of memory
        return false;
    }

    out_hashedPasswd.assign(hashed);
    return true;
}

bool Crypto::Argon2Verify(const std::string &passwd, const std::string &hashedPasswd)
{
    if (crypto_pwhash_str_verify(hashedPasswd.c_str(), passwd.c_str(), passwd.size()) != 0) {
        // wrong password
        return false;
    }

    return true;
}

Crypto::Crypto(const Byte *key, const std::size_t keyLen, const Byte *iv, const std::size_t ivLen) :
    m_pimpl(make_unique<Crypto::Impl>())
{
    m_pimpl->Key = new Byte[keyLen];
    m_pimpl->IV = new Byte[ivLen];

    std::copy(key, key + keyLen, m_pimpl->Key);
    std::copy(iv, iv + ivLen, m_pimpl->IV);

    m_pimpl->KeyLen = keyLen;
    m_pimpl->IVLen = ivLen;
}

Crypto::~Crypto()
{

}

bool Crypto::Encrypt(const std::string &plainText, std::string &out_encodedText)
{
    return Crypto::Encrypt(plainText, out_encodedText, m_pimpl->Key, m_pimpl->KeyLen, m_pimpl->IV, m_pimpl->IVLen);
}

bool Crypto::Encrypt(const std::string &plainText, std::string &out_encodedText,
                     std::string &out_error)
{
    return Crypto::Encrypt(plainText, out_encodedText, out_error, m_pimpl->Key, m_pimpl->KeyLen, m_pimpl->IV, m_pimpl->IVLen);
}

bool Crypto::Decrypt(const std::string &cipherText, std::string &out_recoveredText)
{
    return Crypto::Decrypt(cipherText, out_recoveredText, m_pimpl->Key, m_pimpl->KeyLen, m_pimpl->IV, m_pimpl->IVLen);
}

bool Crypto::Decrypt(const std::string &cipherText, std::string &out_recoveredText,
                     std::string &out_error)
{
    return Crypto::Encrypt(cipherText, out_recoveredText, out_error, m_pimpl->Key, m_pimpl->KeyLen, m_pimpl->IV, m_pimpl->IVLen);
}

std::string Crypto::ByteArrayToString(const unsigned char *array, const size_t length)
{
    ostringstream oss;

    for (size_t i = 0; i < length; ++i) {
        oss << array[i];
    }

    return oss.str();
}

std::wstring Crypto::WCharArrayToString(const wchar_t *array, const size_t length)
{
    wostringstream woss;

    for (size_t i = 0; i < length; ++i) {
        woss << array[i];
    }

    return woss.str();
}

std::string Crypto::HexStringToString(const std::string &hexString)
{
    vector<string> bytes;
    algorithm::iter_split(bytes, hexString, algorithm::first_finder(":"));

    ostringstream oss;

    for (vector<string>::iterator it = bytes.begin(); it != bytes.end(); ++it) {
        oss << static_cast<char>(strtol(it->c_str(), NULL, 16));
    }

    return oss.str();
}

std::wstring Crypto::HexStringToWString(const std::wstring &hexString)
{
    vector<wstring> bytes;
    algorithm::iter_split(bytes, hexString, algorithm::first_finder(":"));

    wostringstream woss;

    for (vector<wstring>::iterator it = bytes.begin(); it != bytes.end(); ++it) {
        woss << static_cast<wchar_t>(wcstol(it->c_str(), NULL, 16));
    }

    return woss.str();
}

Crypto::Impl::Impl()
{

}

Crypto::Impl::~Impl()
{
    delete IV;
    delete Key;
}
