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

#include "diffiehellmanhelper.h"

DiffieHellmanHelper::DiffieHellmanHelper(QObject *parent) :
    QObject(parent),
    m_server_secret(tr("server_secret")),
    m_private_key(NULL),
    m_key(tr("")),
    m_pub_key_2(NULL),
    m_is_bob(false)
{

}

DiffieHellmanHelper::~DiffieHellmanHelper()
{
    if(m_private_key) {
        DH_free(m_private_key);
        m_private_key = NULL;
    }
}

QString DiffieHellmanHelper::start()
{
    if(m_private_key == NULL) {
        m_private_key = DH_new();
        m_private_key->p = NULL;
        m_private_key->g = NULL;
    }
    // Generate the parameters to be used
    if(DH_generate_parameters_ex(m_private_key, 1024, DH_GENERATOR_2, NULL) != 1) {
    //if(DH_generate_parameters_ex(m_private_key, 2048, DH_GENERATOR_2, NULL) != 1) {
        return tr("p=&g=0&pub_key=0");
    }
    int codes = 0;
    if(DH_check(m_private_key, &codes) != 1 || codes != 0) {
        return tr("p=&g=0&pub_key=0");
    }
    // Generate the public and private key pair
    if(DH_generate_key(m_private_key) != 1) {
        return tr("p=&g=0&pub_key=0");
    }
    char *p_pub_key = BN_bn2hex(m_private_key->pub_key);
    QString pub_key = tr("0");
    if(p_pub_key) {
        pub_key = p_pub_key;
        OPENSSL_free(p_pub_key);
    } else {
        return tr("p=&g=0&pub_key=0");
    }

    char *p_p = BN_bn2hex(m_private_key->p);
    QString p = tr("0");
    if(p_p) {
        p = p_p;
        OPENSSL_free(p_p);
    } else {
        return tr("p=&g=0&pub_key=0");
    }

    char *p_g = BN_bn2hex(m_private_key->g);
    QString g = tr("0");
    if(p_g) {
        g = p_g;
        OPENSSL_free(p_g);
    } else {
        return tr("p=&g=0&pub_key=0");
    }

    return tr("p=%1&g=%2&pub_key=%3").arg(p).arg(g).arg(pub_key);
}
QString DiffieHellmanHelper::startB(QString p, QString g, QString pub_key)
{
    m_is_bob = true;
    if(m_private_key == NULL) {
        m_private_key = DH_new();
        m_private_key->p = NULL;
        m_private_key->g = NULL;
    }

    BN_hex2bn(&(m_private_key->p), p.toStdString().c_str() );
    BN_hex2bn(&(m_private_key->g), g.toStdString().c_str() );

    if(DH_generate_key(m_private_key) != 1) {
        return tr("pub_key_2=0");
    }

    BN_hex2bn(&(m_pub_key_2), pub_key.toStdString().c_str() );

    int key_size_bytes = DH_size(m_private_key);
    unsigned char *shared_key = (unsigned char *)OPENSSL_malloc(key_size_bytes);
    if(shared_key == NULL) {
        return tr("pub_key_2=0");
    }
    DH_compute_key(shared_key, m_pub_key_2, m_private_key);
    m_key = BinaryCharToHexQString(shared_key, key_size_bytes);
    OPENSSL_free(shared_key);

    char *p_pub_key_2 = BN_bn2hex(m_private_key->pub_key);
    QString pub_key_2 = tr("0");
    if(p_pub_key_2) {
        pub_key_2 = p_pub_key_2;
        OPENSSL_free(p_pub_key_2);
    } else {
        return tr("pub_key_2=0");
    }

    return tr("pub_key_2=%1").arg(pub_key_2);
}
bool DiffieHellmanHelper::key(QString pub_key_2)
{
    BN_hex2bn(&(m_pub_key_2), pub_key_2.toStdString().c_str() );

    int key_size_bytes = DH_size(m_private_key);
    unsigned char *shared_key = (unsigned char *)OPENSSL_malloc(key_size_bytes);
    if(shared_key == NULL) {
        return false;
    }
    DH_compute_key(shared_key, m_pub_key_2, m_private_key);
    m_key = BinaryCharToHexQString(shared_key, key_size_bytes);
    OPENSSL_free(shared_key);

    return true;
}
QString DiffieHellmanHelper::get_key()
{
    return m_key;
}
QString DiffieHellmanHelper::get_secret_string()
{
    if(m_pub_key_2 == NULL) {
        return tr("");
    }

    char *p_pub_key = BN_bn2hex(m_private_key->pub_key);
    QString pub_key = tr("0");
    if(p_pub_key) {
        pub_key = p_pub_key;
        OPENSSL_free(p_pub_key);
    } else {
        return tr("");
    }

    char *p_pub_key_2 = BN_bn2hex(m_pub_key_2);
    QString pub_key_2 = tr("0");
    if(p_pub_key_2) {
        pub_key_2 = p_pub_key_2;
        OPENSSL_free(p_pub_key_2);
    } else {
        return tr("");
    }

    QString secret = tr("");
    if(m_is_bob) {
        secret = pub_key_2 + pub_key + m_server_secret;
    } else {
        secret = pub_key + pub_key_2 + m_server_secret;
    }
    QByteArray md5 = QCryptographicHash::hash(secret.toLatin1(), QCryptographicHash::Md5);
    QString md5_string = BinaryCharToHexQString((unsigned char *)(md5.constData()), md5.size());
    return md5_string;
}

QString DiffieHellmanHelper::BinaryCharToHexQString(unsigned char* mem, int size)
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
int DiffieHellmanHelper::HexQStringToBinaryChar(QString src, unsigned char *mem)
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
