/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef xlabel_h
#define xlabel_h

#include <QLabel>

#include "widgets.h"
#include "xdatawidgetmapper.h"

class QDoubleValidator;
class QIntValidator;
class QVariant;

class XTUPLEWIDGETS_EXPORT XLabel : public QLabel
{
  Q_OBJECT
  Q_PROPERTY(QString image        READ image       WRITE setImage)
  Q_PROPERTY(QString fieldName    READ fieldName   WRITE setFieldName)
  Q_PROPERTY(int     precision    READ precision   WRITE setPrecision)
  
  public:
    XLabel(QWidget *, const char * = 0);

    virtual QString fieldName()   const { return _fieldName; }
    virtual QString image()       const { return _image; }
    virtual void    setPrecision(int);
    int precision() const { return _precision; }
    Q_INVOKABLE virtual void    setPrecision(QDoubleValidator *);
    Q_INVOKABLE virtual void    setPrecision(QIntValidator *);
    Q_INVOKABLE double          toDouble(bool * = 0);

  public slots:
    virtual void setDataWidgetMap(XDataWidgetMapper* m);
    virtual void setDouble(const double, const int = -1);
    virtual void setFieldName(QString p)    { _fieldName = p; }
    virtual void setImage(QString image);
    virtual void setText(const QString &);
    virtual void setText(const char *);
    virtual void setText(const QVariant &);
    virtual void setTextColor(const QString &);

  private:
    QString _image;
    QString _fieldName;
    int     _precision;
};

#endif
