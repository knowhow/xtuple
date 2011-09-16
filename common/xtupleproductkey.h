
#ifndef __KEYTOOLS_H__
#define __KEYTOOLS_H__

#include <QString>
#include <QDate>

class XTupleProductKeyPrivate;

class XTupleProductKey
{
  public:
    XTupleProductKey(const QString &);
    virtual ~XTupleProductKey();

    bool valid() const;

    int version() const;
    QDate expiration() const;
    int users() const;
    QString customerId() const;
    bool perpetual() const;

  private:
    XTupleProductKeyPrivate * _private;
};

#endif
