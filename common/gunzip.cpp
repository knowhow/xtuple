/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "gunzip.h"

#include <zlib.h>
#include <qbuffer.h>

QByteArray gunzipFile(const QString & file)
{
  QByteArray data;

  gzFile fin = gzopen(file.toLatin1().data(), "rb");
  if(!fin)
    return data;

  QBuffer fout(&data);
  if(!fout.open(QIODevice::WriteOnly))
  {
    gzclose(fin);
    return data;
  }

  char bytes[1024];
  int byte_count;

  while(!gzeof(fin))
  {
    byte_count = gzread(fin, &bytes[0], 1024);
    if(byte_count == -1)
      break;
    if(byte_count > 0)
      fout.write(&bytes[0], byte_count);
  }

  fout.close();
  gzclose(fin);

  return data;
}
