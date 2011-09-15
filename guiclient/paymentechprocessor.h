/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PAYMENTECGPROCESSOR_H
#define PATNEBTECHPROCESSOR_H

#include <QObject>
#include <QString>

#include "creditcardprocessor.h"

class PaymentechProcessor : public CreditCardProcessor
{
  Q_OBJECT

  public:
    PaymentechProcessor();

  protected:
    virtual int  buildCommon(QString &, const int, const int, const double, const int, QString&, QString, const QString & = QString::null, const QString & = QString::null);
    virtual int  doTestConfiguration();
    virtual int  doAuthorize(const int, const int, double &, const double, const bool, const double, const double, const int, QString&, QString&, int&, ParameterList &);
    virtual int  doCharge(const int, const int, const double, const double, const bool, const double, const double, const int, QString&, QString&, int&, ParameterList &);
    virtual int  doChargePreauthorized(const int, const int, const double, const int, QString&, QString&, int&, ParameterList &);
    virtual int  doCredit(const int, const int, const double, const double, const bool, const double, const double, const int, QString&, QString&, int&, ParameterList &);
    virtual int  doVoidPrevious(const int, const int, const double, const int, QString&, QString&, QString&, int&, ParameterList &);
    virtual bool handlesCreditCards();
    virtual int  handleResponse(const QString&, const int, const QString&, const double, const int, QString&, QString&, int&, ParameterList&);
};

#endif // PAYMENTECHPROCESSOR_H
