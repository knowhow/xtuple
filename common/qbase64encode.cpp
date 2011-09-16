/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qbase64encode.h"

#include <QString>
#include <QIODevice>
#include <QTextStream>
#include <QBuffer>

static const char _base64Table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static char getValue(char c) {
    int i = 0;
    while(_base64Table[i] != '\0') {
        if(_base64Table[i] == c) return (char)i;
        i++;
    }
    return (char)-1;
}

QString QBase64Encode(QIODevice & iod) {
    QString value;  // the string that holds the final encoded values
    int packet = 0; // there are 19 packets per line (packet=3 bytes in)

    char _in[3];
    unsigned char in[3];
    char out[4];
    qint64 didRead = 0;

    while(!iod.atEnd()) {
        // set the values to 0 in case we don't read all 3
        _in[0] = _in[1] = _in[2] = (char)0;

        // read in up to the next 3 chars
        didRead = iod.read(&_in[0], 3);

        in[0] = (unsigned char)_in[0];
        in[1] = (unsigned char)_in[1];
        in[2] = (unsigned char)_in[2];

        if(didRead > 0) {
            // determine the encoded chars
            out[0] = _base64Table[ in[0] >> 2 ];
            out[1] = _base64Table[ ((in[0] & 0x03) << 4) | (in[1] >> 4) ];
            out[2] = _base64Table[ ((in[1] & 0x0F) << 2) | (in[2] >> 6) ];
            out[3] = _base64Table[ in[2] & 0x3F ];

            // modify the chars if we were short
            if(didRead < 3) {
                out[3] = '=';
                if(didRead < 2) {
                    out[2] = '=';
                }
            }

            // place the final chars in the value
            value += out[0];
            value += out[1];
            value += out[2];
            value += out[3];
        }


        if(++packet >= 19) {
            packet = 0;    // reset the packet count
            value += '\n'; // add a newline to the value
        }
    }
    value += '\n'; // throw one last newline onto the end

    return value;
}

QByteArray QBase64Decode(const QString & _source) {
    QString source = _source;
    QByteArray value;

    // empty string -- nothing to do
    if(source.isEmpty()) return value;

    QTextStream in(&source, QIODevice::ReadOnly);

    QBuffer buf(&value);
    buf.open(QIODevice::WriteOnly);

    char a[4], b[4], o[3];
    qint64 n = 0;
    int p = 0; // current position in string
    int l = source.length(); // length of string
    char c = 0;
    while (p < l) {
        // read in the next 4 valid bytes
        n = 0;
        while(n < 4 && p < l) {
            c = ((QChar)source.at(p++)).toAscii();
            a[n] = c;
            b[n] = (c == '=' ? 0 : getValue(c));
            if(b[n] != -1) {
                n++;
            }
        }

        if(n < 4 && n != 0) {
            // whoops we have a mismatch in the number of bytes we need
        }

        // convert from base64 to binary
        o[0] = (b[0] << 2) | (b[1] >> 4);
        o[1] = (b[1] << 4) | (b[2] >> 2);
        o[2] = (b[2] << 6) | b[3];

        // determine how many bytes we should be reading
        if(a[2] == '=')      n = 1;
        else if(a[3] == '=') n = 2;
        else                 n = 3;

        if(n != buf.write(&o[0], n)) {
            // eek we didn't write the number of block we were supposed to
        }

        if(n < 3) break; // we've reached the end of the data we have to read so just stop
    }

    return value;
}

