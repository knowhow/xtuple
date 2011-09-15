/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xbase32.h"

#include <QString>
#include <QIODevice>
#include <QTextStream>
#include <QBuffer>

// This is based on base64 but modified to use a set of 32 chars instead
static const char _base32Table[] = "ABCDEFGHJKMNPQRSTVWXYZ0123456789";
// I = 1
// L = 1
// O = 0 (zero)
// U Place holder

static char getValue(char c) {
    int i = 0;
    if('I' == c || 'L' == c)
      c = '1';
    else if('O' == c)
      c = '0';
    while(_base32Table[i] != '\0') {
        if(_base32Table[i] == c) return (char)i;
        i++;
    }
    return (char)-1;
}

QString XBase32Encode(QIODevice & iod) {
    QString value;  // the string that holds the final encoded values
    int packet = 0; // there are 9 packets per line (packet=5 bytes in)

    char _in[5];
    unsigned char in[5];
    char out[8];
    qint64 didRead = 0;

    while(!iod.atEnd()) {
        // set the values to 0 in case we don't read all 5
        _in[0] = _in[1] = _in[2] = _in[3] = _in[4] = (char)0;

        // read in up to the next 5 chars
        didRead = iod.read(&_in[0], 5);

        in[0] = (unsigned char)_in[0];
        in[1] = (unsigned char)_in[1];
        in[2] = (unsigned char)_in[2];
        in[3] = (unsigned char)_in[3];
        in[4] = (unsigned char)_in[4];

        if(didRead > 0) {
            // determine the encoded chars
            out[0] = _base32Table[ in[0] >> 3 ];
            out[1] = _base32Table[ ((in[0] & 0x07) << 2) | (in[1] >> 6) ];
            out[2] = _base32Table[ ((in[1] & 0x3E) >> 1) ];
            out[3] = _base32Table[ ((in[1] & 0x01) << 4) | (in[2] >> 4) ];
            out[4] = _base32Table[ ((in[2] & 0x0F) << 1) | (in[3] >> 7) ];
            out[5] = _base32Table[ ((in[3] & 0x7C) >> 2) ];
            out[6] = _base32Table[ ((in[3] & 0x03) << 3) | (in[4] >> 5) ];
            out[7] = _base32Table[ ((in[4] & 0x1F)) ];

            // modify the chars if we were short
            if(didRead < 5) {
                out[7] = 'U';
                if(didRead < 4) {
                  out[6] = 'U';
                  out[5] = 'U';
                }
                if(didRead < 3)
                  out[4] = 'U';
                if(didRead < 2) {
                  out[3] = 'U';
                  out[2] = 'U';
                }
            }

            // place the final chars in the value
            value += out[0];
            value += out[1];
            value += out[2];
            value += out[3];
            value += out[4];
            value += out[5];
            value += out[6];
            value += out[7];
        }


        if(++packet >= 9) {
            packet = 0;    // reset the packet count
            value += '\n'; // add a newline to the value
        }
    }
    value += '\n'; // throw one last newline onto the end

    return value;
}

QByteArray XBase32Decode(const QString & _source) {
    QString source = _source;
    QByteArray value;

    // empty string -- nothing to do
    if(source.isEmpty()) return value;

    QTextStream in(&source, QIODevice::ReadOnly);

    QBuffer buf(&value);
    buf.open(QIODevice::WriteOnly);

    char a[8], b[8], o[5];
    qint64 n = 0;
    int p = 0; // current position in string
    int l = source.length(); // length of string
    char c = 0;
    while (p < l) {
        // read in the next 8 valid bytes
        n = 0;
        while(n < 8 && p < l) {
            c = ((QChar)source.at(p++)).toUpper().toAscii();
            a[n] = c;
            b[n] = (c == 'U' ? 0 : getValue(c));
            if(b[n] != -1) {
                n++;
            }
        }

        if(n < 8 && n != 0) {
            // whoops we have a mismatch in the number of bytes we need
        }
        if(n == 0) break;

        // convert from base32 to binary
        o[0] = (b[0] << 3) | (b[1] >> 2);
        o[1] = (b[1] << 6) | (b[2] << 1) | (b[3] >> 4);
        o[2] = (b[3] << 4) | (b[4] >> 1);
        o[3] = (b[4] << 7) | (b[5] << 2) | (b[6] >> 3);
        o[4] = (b[6] << 5) | b[7];

        // determine how many bytes we should be reading
        if(a[2] == 'U')      n = 1;
        else if(a[4] == 'U') n = 2;
        else if(a[5] == 'U') n = 3;
        else if(a[7] == 'U') n = 4;
        else                 n = 5;

        if(n != buf.write(&o[0], n)) {
            // eek we didn't write the number of block we were supposed to
        }

        if(n < 5) break; // we've reached the end of the data we have to read so just stop
    }

    return value;
}

