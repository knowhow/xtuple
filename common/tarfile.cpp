/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "tarfile.h"

#include <qtextstream.h>
#include <qbuffer.h>

struct tarHeaderBlock {
    char name[100];     // name of file
    char mode[8];       // file mode
    char uid[8];        // owner user ID
    char gid[8];        // owner group ID
    char size[12];      // length of file in bytes
    char mtime[12];     // modified time of file
    char chksum[8];     // header checksum
    char typeflag;      // see type constants
    char linkname[100]; // name of linked file
    char magic[8];      // "ustar  " + null terminator
    char uname[32];     // owner user name
    char gname[32];     // owner group name
    char devmajor[8];   // device major number
    char devminor[8];   // device minor number
    char prefix[155];   // prefix for file names longer than 100 bytes
    char filler[12];    // filler to make header even 512 bytes
};

const char TYPE_REGULAR_ALT = '\0'; // old compatible flag for regular file
const char TYPE_REGULAR     = '0';  // regular file
const char TYPE_LINK        = '1';  // link to another file in archive
const char TYPE_SYMLINK     = '2';  // Symbolic link
const char TYPE_CHAR        = '3';  // Character special device
const char TYPE_BLOCK       = '4';  // Block special device
const char TYPE_DIR         = '5';  // Directory
const char TYPE_FIFO        = '6';  // FIFO special file
const char TYPE_CONTIGUOS   = '7';  // RESERVERED/Contiguous file


TarFile::TarFile(const QByteArray & bytes)
{
  _valid = false;

  QByteArray localBytes(bytes);
  QBuffer fin(&localBytes);
  if(!fin.open(QIODevice::ReadOnly))
    return;

  bool valid = false;
  long size = 0;
  long blocks = 0;
  char block[512];
  QString name, str;

  while(!fin.atEnd())
  {
    tarHeaderBlock head;
    fin.read((char*)&head, sizeof(head));

    if(head.name[0] == '\0' && head.size[0] == '\0' && head.typeflag == '\0')
      continue;

    QString magic(head.magic);
    if(magic != "ustar  ")
      return;

    name = QString(head.name);
    str = QString(head.size);
    size = str.toLong(&valid, 8);

    if(head.typeflag == TYPE_REGULAR_ALT)
      head.typeflag = TYPE_REGULAR;

    blocks = (size + 511) / 512;

    if(head.typeflag == TYPE_REGULAR)
    {
      QByteArray mybytes;
      QBuffer fout(&mybytes);
      fout.open(QIODevice::WriteOnly);
      for(int i = 0; i < blocks; i++)
      {
        fin.read(&block[0], 512);
        fout.write(&block[0], qMin(size,512L));
        size -= 512;
      }
      fout.close();
      _list.insert(name, mybytes);
    }
    else
    {
      for(int i = 0; i < blocks; i++)
        fin.read(&block[0], 512);
    }
  }
  _valid = true;
}

TarFile::~TarFile()
{
}
