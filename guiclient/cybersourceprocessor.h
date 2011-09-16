/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CYBERSOURCEPROCESSOR_H
#define CYBERSOURCEPROCESSOR_H

#include <QDomElement>
#include <QObject>
#include <QString>
#include <QVariant>

#include "creditcardprocessor.h"

class QDomDocument;
class QXmlQuery;

class CyberSourceProcessor : public CreditCardProcessor
{
  Q_OBJECT

  public:
    CyberSourceProcessor();
    virtual ~CyberSourceProcessor();

  protected:
    virtual int  buildCommon(const int pccardid, const int pcvv, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, const CCTransaction ptranstype);
    virtual int  doTestConfiguration();
    virtual int  doAuthorize(const int pccardid, const int pcvv, double &pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString& pneworder, QString& preforder, int &pccpayid, ParameterList &pparams);
    virtual int  doCharge(const int pccardid, const int pcvv, const double pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString& pneworder, QString& preforder, int &pccpayid, ParameterList &pparams);
    virtual int doChargePreauthorized(const int pccardid, const int pcvv, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, ParameterList &pparams);
    virtual int doCredit(const int pccardid, const int pcvv, const double pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, ParameterList &pparams);
    virtual int  doReverseAuthorize(const int pccardid, const double pamount, const int pcurrid, QString& pneworder, QString& preforder, int &pccpayid, ParameterList &pparams);
    virtual int doVoidPrevious(const int pccardid, const int pcvv, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, QString &papproval, int &pccpayid, ParameterList &pparams);
    virtual bool handlesCreditCards();
    virtual int  handleResponse(const QString &presponse, const int pccardid, const CCTransaction ptype, double &pamount, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, ParameterList &pparams);
    virtual QVariant xqresult(QXmlQuery *xq, QString query, QVariant::Type type = QVariant::String);

    QDomDocument *_doc;
    QDomElement   _requestMessage;
};

#endif
