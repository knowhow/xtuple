/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#ifndef metricsenc_h
#define metricsenc_h

#include <QObject>
#include <QString>
#include <QMap>
#include <QVariant>

typedef QMap<QString, QString> MetricMap;

class Parametersenc : public QObject
{
  Q_OBJECT

  protected:
    MetricMap _values;
    QString   _readSql;
    QString   _setSql;
    QString   _username;
    QString   _key;
    bool      _dirty;

  public:
    Parametersenc(QObject * parent = 0);
    virtual ~Parametersenc() {};

    void load();

    QString value(const char *);
    bool    boolean(const char *);

    void set(const char *, bool);
    void set(const QString &, bool);
    void set(const char *, int);
    void set(const QString &, int);
    void set(const char *, const QString &);
    void set(const QString &, const QString &);

    QString parent(const QString &);

  public slots:
    QString value(const QString &);
    bool    boolean(const QString &);

  protected:
    void _set(const QString &, QVariant);

};

class Metricsenc : public Parametersenc
{
  public:
//    metricsenc();
    Metricsenc() {};
    Metricsenc(const QString &);
};

#endif

