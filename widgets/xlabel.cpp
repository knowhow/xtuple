/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xlabel.h"

#include <QLocale>
#include <QMessageBox>
#include <QSqlError>
#include <QValidator>

#include "format.h"
#include <quuencode.h>
#include "xsqlquery.h"

#define DEBUG false

XLabel::XLabel(QWidget *parent, const char *name) :
  QLabel(parent)
{
  if(name)
    setObjectName(name);
  _fieldName = "";
  _precision = 0;
}

void XLabel::setDataWidgetMap(XDataWidgetMapper* m)
{
  m->addMapping(this, _fieldName, QByteArray("text"), QByteArray("defaultText"));
}

void XLabel::setPrecision(int pPrec)
{
  _precision = pPrec;
}

void XLabel::setPrecision(QDoubleValidator *pVal)
{
  _precision = pVal->decimals();
}

void XLabel::setPrecision(QIntValidator * /*pVal*/)
{
  _precision = 0;
}

void XLabel::setDouble(const double pDouble, const int pPrec)
{
  QLabel::setText(formatNumber(pDouble, (pPrec < 0) ? _precision : pPrec));
}

void XLabel::setImage(QString image)
{
  if (_image == image)
    return;

  _image = image;
  XSqlQuery qry;
  qry.prepare("SELECT image_data "
              "FROM image "
              "WHERE (image_name=:image);");
  qry.bindValue(":image", _image);
  qry.exec();
  if (qry.first())
  {
    QImage img;
    img.loadFromData(QUUDecode(qry.value("image_data").toString()));
    setPixmap(QPixmap::fromImage(img));
    return;
  }
  else if (qry.lastError().type() != QSqlError::NoError)
    QMessageBox::critical(this, tr("A System Error occurred at %1::%2.")
                          .arg(__FILE__)
                          .arg(__LINE__),
                          qry.lastError().databaseText());
  setPixmap(QPixmap());
}

void XLabel::setText(const QVariant &pVariant)
{
  if (DEBUG)
    qDebug("XLabel::setText(const QVariant & = %s)",
           qPrintable(pVariant.toString()));
  if (pVariant.type() == QVariant::Double ||
      pVariant.type() == QVariant::Int)
    QLabel::setText(formatNumber(pVariant.toDouble(), _precision));
  else
    QLabel::setText(pVariant.toString());
}

void XLabel::setText(const char *pText)
{
  if (DEBUG) qDebug("XLabel::setText(const char * = %s)", pText);
  setText(QString(pText));
}

void XLabel::setText(const QString &pText)
{
  if (DEBUG)
    qDebug("XLabel::setText(const QString & = %s)", qPrintable(pText));
  if (_precision == 0)
    QLabel::setText(pText);
  else
  {
    bool ok;
    double val = QLocale().toDouble(pText, &ok);
    if (DEBUG) qDebug("XLabel::setText() %f %d", val, ok);
    if (ok)
      setDouble(val);
    else
      QLabel::setText(pText);
  }
}

double XLabel::toDouble(bool *pIsValid)
{
  return QLocale().toDouble(text(), pIsValid);
}

void XLabel::setTextColor(const QString &pColorName)
{
  QColor c = namedColor(pColorName);
  QPalette p = palette();
  p.setColor(foregroundRole(), c);
  setPalette(p);
}
