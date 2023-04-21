/*
Copyright (c) 2011, Andre Somers
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the Rathenau Instituut, Andre Somers nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL ANDRE SOMERS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR #######; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "tcachehandler.h"

#include <QIODevice>
#include <QByteArray>
#include <QByteArrayView>
#include <QtDebug>
#include <QtGlobal>
#include <QDateTime>
#include <QCryptographicHash>
#include <QDataStream>

// Handler to manage cache data

TCacheHandler::TCacheHandler(): id(0), m_compressionMode(CompressionAuto), m_protectionMode(ProtectionChecksum),
                                m_lastError(ErrorNoError)
{
    generator = QRandomGenerator(uint(QDateTime::currentMSecsSinceEpoch() & 0xFFFF));

    // qsrand(uint(QDateTime::currentMSecsSinceEpoch() & 0xFFFF));
}

TCacheHandler::TCacheHandler(quint64 key): id(key), m_compressionMode(CompressionAuto), m_protectionMode(ProtectionChecksum),
    m_lastError(ErrorNoError)
{
    // qsrand(uint(QDateTime::currentMSecsSinceEpoch() & 0xFFFF));

    generator = QRandomGenerator(uint(QDateTime::currentMSecsSinceEpoch() & 0xFFFF));
    splitParameter();
}

void TCacheHandler::setParameter(quint64 key)
{
    id = key;
    splitParameter();
}

void TCacheHandler::splitParameter()
{
    parts.clear();
    parts.resize(8);

    for (int i=0;i<8;i++) {
        quint64 part = id;
        for (int j=i; j>0; j--)
            part = part >> 8;
        part = part & 0xff;
        parts[i] = static_cast<char>(part);
    }
}

QByteArray TCacheHandler::saveRecordAsByteArray(const QString& plaintext)
{
    QByteArray plaintextArray = plaintext.toUtf8();

    return saveRecordAsByteArray(plaintextArray);
}

QByteArray TCacheHandler::saveRecordAsByteArray(QByteArray plaintext)
{
    if (parts.isEmpty()) {
        #ifdef TUP_DEBUG
            qWarning() << "No key set.";
        #endif
        m_lastError = ErrorNoKeySet;
        return QByteArray();
    }

    QByteArray ba = plaintext;

    CacheFlags flags = EncodingFlagNone;
    if (m_compressionMode == CompressionAlways) {
        ba = qCompress(ba, 9); //maximum compression
        flags |= EncodingFlagCompression;
    } else if (m_compressionMode == CompressionAuto) {
        QByteArray compressed = qCompress(ba, 9);
        if (compressed.size() < ba.size()) {
            ba = compressed;
            flags |= EncodingFlagCompression;
        }
    }

    QByteArray integrityProtection;
    if (m_protectionMode == ProtectionChecksum) {
        flags |= EncodingFlagChecksum;
        QDataStream s(&integrityProtection, QIODevice::WriteOnly);
        s << qChecksum(QByteArrayView(ba.constData()));
        // s << qChecksum(ba.constData(), ba.length());
    } else if (m_protectionMode == ProtectionHash) {
        flags |= EncodingFlagHash;
        QCryptographicHash hash(QCryptographicHash::Sha1);
        hash.addData(ba);

        integrityProtection += hash.result();
    }

    // char randomChar = char(qrand() & 0xFF);
    char randomChar = char(generator.generate() & 0xFF);
    ba = randomChar + integrityProtection + ba;

    int pos(0);
    char lastChar(0);

    int cnt = ba.size();

    while (pos < cnt) {
        ba[pos] = ba.at(pos) ^ parts.at(pos % 8) ^ lastChar;
        lastChar = ba.at(pos);
        ++pos;
    }

    QByteArray resultArray;
    resultArray.append(char(0x03));
    resultArray.append(char(flags));
    resultArray.append(ba);
    m_lastError = ErrorNoError;

    return resultArray;
}

QString TCacheHandler::getRecord(const QString &plaintext)
{
    QByteArray plaintextArray = plaintext.toUtf8();
    QByteArray cypher = saveRecordAsByteArray(plaintextArray);
    QString cypherString = QString::fromLatin1(cypher.toBase64());

    return cypherString;
}

QString TCacheHandler::getRecord(QByteArray plaintext)
{
    QByteArray cypher = saveRecordAsByteArray(plaintext);
    QString cypherString = QString::fromLatin1(cypher.toBase64());

    return cypherString;
}

QString TCacheHandler::setRecord(const QString &cyphertext)
{
    QByteArray cyphertextArray = QByteArray::fromBase64(cyphertext.toLatin1());
    QByteArray plaintextArray = restoreFromByteArray(cyphertextArray);
    QString plaintext = QString::fromUtf8(plaintextArray, plaintextArray.size());

    return plaintext;
}

QString TCacheHandler::setRecord(QByteArray cypher)
{
    QByteArray ba = restoreFromByteArray(cypher);
    QString plaintext = QString::fromUtf8(ba, ba.size());

    return plaintext;
}

QByteArray TCacheHandler::restoreFromByteArray(const QString& cyphertext)
{
    QByteArray cyphertextArray = QByteArray::fromBase64(cyphertext.toLatin1());
    QByteArray ba = restoreFromByteArray(cyphertextArray);

    return ba;
}

QByteArray TCacheHandler::restoreFromByteArray(QByteArray cypher)
{
    if (parts.isEmpty()) {
        #ifdef TUP_DEBUG
            qWarning() << "No key set.";
        #endif
        m_lastError = ErrorNoKeySet;

        return QByteArray();
    }

    QByteArray ba = cypher;

    if( cypher.size() < 3 )
        return QByteArray();

    char version = ba.at(0);

    if (version !=3) {
        m_lastError = ErrorUnknownVersion;
        #ifdef TUP_DEBUG
            qWarning() << "Invalid version.";
        #endif
        return QByteArray();
    }

    CacheFlags flags = CacheFlags(ba.at(1));

    ba = ba.mid(2);
    int pos(0);
    int cnt(ba.size());
    char lastChar = 0;

    while (pos < cnt) {
        char currentChar = ba[pos];
        ba[pos] = ba.at(pos) ^ lastChar ^ parts.at(pos % 8);
        lastChar = currentChar;
        ++pos;
    }

    ba = ba.mid(1);

    bool integrityOk(true);
    if (flags.testFlag(EncodingFlagChecksum)) {
        if (ba.length() < 2) {
            m_lastError = ErrorIntegrityFailed;
            return QByteArray();
        }
        quint16 storedChecksum;
        {
            QDataStream s(&ba, QIODevice::ReadOnly);
            s >> storedChecksum;
        }
        ba = ba.mid(2);
        quint16 checksum = qChecksum(QByteArrayView(ba.constData()));
        // quint16 checksum = qChecksum(ba.constData(), ba.length());

        integrityOk = (checksum == storedChecksum);
    } else if (flags.testFlag(EncodingFlagHash)) {
        if (ba.length() < 20) {
            m_lastError = ErrorIntegrityFailed;
            return QByteArray();
        }
        QByteArray storedHash = ba.left(20);
        ba = ba.mid(20);
        QCryptographicHash hash(QCryptographicHash::Sha1);
        hash.addData(ba);
        integrityOk = (hash.result() == storedHash);
    }

    if (!integrityOk) {
        m_lastError = ErrorIntegrityFailed;
        return QByteArray();
    }

    if (flags.testFlag(EncodingFlagCompression))
        ba = qUncompress(ba);

    m_lastError = ErrorNoError;

    return ba;
}
