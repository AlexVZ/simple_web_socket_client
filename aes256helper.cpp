/****************************************************************************
**
** Copyright (C) 2014 Alex
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "aes256helper.h"

AES256Helper::AES256Helper(QObject *parent) :
    QObject(parent)
{
}

AES256Helper::~AES256Helper()
{
}

bool AES256Helper::decrypt(QString msg, QString &msg_decrypted, QString key)
{
    if(key.size() < 32) {
        return false;
    }

    unsigned char p_key[32];
    if(convert_key(key, p_key) == false) {
        return false;
    }

    // data structure that contains the key itself
    AES_KEY aes_256_key;
    // set the decryption key
    AES_set_decrypt_key(p_key, 256, &aes_256_key);
    unsigned char *p_msg = (unsigned char *)OPENSSL_malloc(msg.size());
    if(p_msg == NULL) {
        return false;
    }
    unsigned char *p_decrypted_msg = (unsigned char *)OPENSSL_malloc(msg.size() + 1); // plus zero at end
    if(p_msg == NULL) {
        OPENSSL_free(p_msg);
        return false;
    }
    memset(p_decrypted_msg, 0, msg.size() + 1);
    if(HexQStringToBinaryChar(msg, p_msg) != msg.size()) {
        OPENSSL_free(p_msg);
        OPENSSL_free(p_decrypted_msg);
        return false;
    }
    int msg_size_for_aes = msg.size() / 2;
    while((msg_size_for_aes % AES_BLOCK_SIZE) != 0) {
        msg_size_for_aes++;
    }
    for(int i = 0; i < msg_size_for_aes; i += AES_BLOCK_SIZE) {
        AES_ecb_encrypt(p_msg + i, p_decrypted_msg + i, &aes_256_key, AES_DECRYPT);
    }
    OPENSSL_free(p_msg);
    msg_decrypted = QString::fromUtf8((char *)p_decrypted_msg);
    OPENSSL_free(p_decrypted_msg);

    return true;
}
QString AES256Helper::encrypt(QString msg, QString key)
{
    if(key.size() < 32) {
        return tr("");
    }

    unsigned char p_key[32];
    if(convert_key(key, p_key) == false) {
        return tr("");
    }

    // data structure that contains the key itself
    AES_KEY aes_256_key;
    // set the decryption key
    AES_set_encrypt_key(p_key, 256, &aes_256_key);
    int msg_size_for_aes = msg.size();
    while((msg_size_for_aes % AES_BLOCK_SIZE) != 0) {
        msg_size_for_aes++;
    }
    unsigned char *p_msg = (unsigned char *)OPENSSL_malloc(msg_size_for_aes);
    if(p_msg == NULL) {
        return tr("");
    }
    unsigned char *p_encrypted_msg = (unsigned char *)OPENSSL_malloc(msg_size_for_aes);
    if(p_msg == NULL) {
        OPENSSL_free(p_msg);
        return tr("");
    }
    memset(p_msg, 0, msg_size_for_aes);
    memcpy(p_msg, msg.toUtf8().data(), msg.size()); // BUG: not latin strings will be truncated!
    for(int i = 0; i < msg_size_for_aes; i += AES_BLOCK_SIZE) {
        AES_ecb_encrypt(p_msg + i, p_encrypted_msg + i, &aes_256_key, AES_ENCRYPT);
    }
    OPENSSL_free(p_msg);
    QString msg_encrypted = tr("");
    msg_encrypted = BinaryCharToHexQString(p_encrypted_msg, msg_size_for_aes);
    if(msg_encrypted.size() != msg_size_for_aes * 2) {
        OPENSSL_free(p_encrypted_msg);
        return tr("");
    }
    OPENSSL_free(p_encrypted_msg);
    return msg_encrypted;
}

bool AES256Helper::convert_key(QString key, unsigned char *p_key)
{
    if(key.size() == 32) {
        if(HexQStringToBinaryChar(key, p_key) != 32) {
            return false;
        }
    } else {
        unsigned char *long_key = (unsigned char *)OPENSSL_malloc(key.size());
        if(long_key == NULL) {
            return false;
        }
        if(HexQStringToBinaryChar(key, long_key) != key.size()) {
            OPENSSL_free(long_key);
            return false;
        }
        SHA256_CTX context;
        if(!SHA256_Init(&context)) {
            OPENSSL_free(long_key);
            return false;
        }
        if(!SHA256_Update(&context, long_key, key.size())) {
            OPENSSL_free(long_key);
            return false;
        }
        OPENSSL_free(long_key);
        if(!SHA256_Final(p_key, &context)) {
            return false;
        }
    }
    return true;
}

QString AES256Helper::BinaryCharToHexQString(unsigned char* mem, int size)
{
    QString str;
    QString hex = tr("");
    for(int i = 0; i < size; i++) {
        QString hex = QString("%1").arg((ushort)(mem[i]), 0, 16);
        if(hex.size() < 2) {
            hex = tr("0") + hex;
        }
        str += hex;
    }
    return str.trimmed();
}
int AES256Helper::HexQStringToBinaryChar(QString src, unsigned char *mem)
{
    memset(mem, 0, src.size());
    for(int i = 0; i < src.size(); i += 2) {
        QString str = src.at(i);
        str += src.at(i+1);
        bool ok;
        unsigned int parsedValue = str.toUInt(&ok, 16);
        mem[i/2] = parsedValue;
        if(!ok) {
            return i;
        }
    }
    return src.size();
}
