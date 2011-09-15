
#include "xtupleproductkey.h"
#include "xbase32.h"

#include <QDebug>

QString keyNormalize(const QString & key)
{
  QString nkey = key.toUpper();
  nkey = nkey.replace("-", "");
  nkey = nkey.replace("I", "1", Qt::CaseInsensitive);
  nkey = nkey.replace("L", "1", Qt::CaseInsensitive);
  nkey = nkey.replace("O", "0", Qt::CaseInsensitive);
  return nkey;
}

bool keyCheck(const QString & key)
{
  QString nkey = keyNormalize(key);
  QString csChar = nkey.left(1);
  int cs = 0;
  for(int i = 1; i < nkey.size(); ++i)
    cs += nkey.at(i).toAscii();
  cs = cs % 32;
  static const char _base32Table[] = "ABCDEFGHJKMNPQRSTVWXYZ0123456789";
  return ((csChar.size() > 0) && (csChar.at(0).toAscii() == _base32Table[cs]));
}

class XTupleProductKeyPrivate
{
  public:
    XTupleProductKeyPrivate(const QString &);

    QString key;
    bool valid;
    int version;
    QDate expiration;
    int users;
    QString custid;
};

XTupleProductKeyPrivate::XTupleProductKeyPrivate(const QString & k)
{
  key = k;
  valid = keyCheck(key);
  version = -1;
  expiration.setDate(2000, 1, 1);
  users = -1;

  if(!valid)
  {
    return;
  }

  QByteArray data = XBase32Decode(key.mid(1));
  if(data.size() != 15)
  {
    valid = false;
    return;
  }

  version = (data.at(0) >> 4);
  if(version != 1)
  {
    valid = false;
    return;
  }

  int expM = data.at(0) & 0x0F;
  int expY = data.at(1);
  if(expM >= 1 && expM <= 12 && expY >= 0 && expY <= 255)
  {
    expiration.setDate(2000 + expY, expM, 1);
    expiration.setDate(expiration.year(), expiration.month(), expiration.daysInMonth());
  }
  else
  {
    valid = false;
    return;
  }

  users = data.at(2);

  if(data.at(3) != 0x42)
  {
    valid = false;
    return;
  }

  custid = "";
  for(int i = 4; i < 12; ++i)
    custid.append(data.at(i));

  qint8 b;
  b = (data.at(11) & 0xC0)
     |(data.at(10) & 0x30)
     |(data.at(9)  & 0x0C)
     |(data.at(8)  & 0x03);
  if(data.at(12) != b)
  {
    valid = false;
    return;
  }
  b = (data.at(7) & 0xC0)
     |(data.at(6) & 0x30)
     |(data.at(5) & 0x0C)
     |(data.at(4) & 0x03);
  if(data.at(13) != b)
  {
    valid = false;
    return;
  }
  b = (data.at(3) & 0xC0)
     |(data.at(2) & 0x30)
     |(data.at(1) & 0x0C)
     |(data.at(0) & 0x03);
  if(data.at(14) != b)
  {
    valid = false;
    return;
  }
}

XTupleProductKey::XTupleProductKey(const QString & key)
{
  _private = new XTupleProductKeyPrivate(key);
}

XTupleProductKey::~XTupleProductKey()
{
  if(_private)
    delete _private;
  _private = 0;
}

bool XTupleProductKey::valid() const
{
  return _private->valid;
}

int XTupleProductKey::version() const
{
  return _private->version;
}

QDate XTupleProductKey::expiration() const
{
  return _private->expiration;
}

int XTupleProductKey::users() const
{
  return _private->users;
}

QString XTupleProductKey::customerId() const
{
  return _private->custid;
}

