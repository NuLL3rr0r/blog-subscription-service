/**
 * @file
 * @author  Mohammad S. Babaei <info@babaei.net>
 * @version 0.1.0
 *
 * @section LICENSE
 *
 * (The MIT License)
 *
 * Copyright (c) 2016 Mohammad S. Babaei
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


#ifndef CORELIB_CRYPTO_HPP
#define CORELIB_CRYPTO_HPP


#include <memory>
#include <string>
#include <cstddef>

namespace CoreLib {
class Crypto;
}

class CoreLib::Crypto
{
public:
    typedef unsigned char Byte_t;

private:
    struct Impl;
    std::unique_ptr<Impl> m_pimpl;

public:
    static bool Encrypt(const std::string &plainText, std::string &out_encodedText,
                        const Byte_t *key, std::size_t keyLen, const Byte_t *iv, std::size_t ivLen);
    static bool Encrypt(const std::string &plainText, std::string &out_encodedText,
                        std::string &out_error,
                        const Byte_t *key, std::size_t keyLen, const Byte_t *iv, std::size_t ivLen);
    static bool Decrypt(const std::string &cipherText, std::string &out_recoveredText,
                        const Byte_t *key, std::size_t keyLen, const Byte_t *iv, std::size_t ivLen);
    static bool Decrypt(const std::string &cipherText, std::string &out_recoveredText,
                        std::string &out_error,
                        const Byte_t *key, std::size_t keyLen, const Byte_t *iv, std::size_t ivLen);
    static bool Hash(const std::string &text, std::string &out_digest);
    static bool Hash(const std::string &text, std::string &out_digest,
                             std::string &out_error);
    static int Base64Decode(char value);
    static int Base64Decode(const char *code, const int length, char *out_plainText);
    static void Base64Decode(std::istream &inputStream, std::ostream &outputStream);
    static int Base64Encode(char value);
    static int Base64Encode(const char *code, const int length, char *out_plainText);
    static int Base64EncodeBlockEnd(char *out_plainText);
    static void Base64Encode(std::istream &inputStream, std::ostream &outputStream);

    static std::string ByteArrayToString(const unsigned char *array, size_t length);
    static std::wstring WCharArrayToString(const wchar_t *array, size_t length);

    static std::string HexStringToString(const std::string &hexString);
    static std::wstring HexStringToWString(const std::wstring &hexString);

public:
    Crypto(const Byte_t *key, std::size_t keyLen, const Byte_t *iv, std::size_t ivLen);
    virtual ~Crypto();

public:
    bool Encrypt(const std::string &plainText, std::string &out_encodedText);
    bool Encrypt(const std::string &plainText, std::string &out_encodedText,
                 std::string &out_error);
    bool Decrypt(const std::string &cipherText, std::string &out_recoveredText);
    bool Decrypt(const std::string &cipherText, std::string &out_recoveredText,
                 std::string &out_error);
};


#endif /* CORELIB_CRYPTO_HPP */


