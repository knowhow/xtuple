/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QSqlError>

#include <currcluster.h>

#include "guiclient.h"
#include "externalccprocessor.h"
#include "externalCCTransaction.h"

#define DEBUG false

ExternalCCProcessor::ExternalCCProcessor() : CreditCardProcessor()
{
  _defaultLivePort   = 0;
  _defaultLiveServer = "";
  _defaultTestPort   = 0;
  _defaultTestServer = "";
}

/** \brief Simplify CreditCardProcessor's implementation - we really don't care about
           much with the external credit card processor.
 */
int ExternalCCProcessor::testConfiguration()
{
  if (DEBUG)
    qDebug("ExternalCCProcessor::testConfiguration()");
  reset();

  if (!_privileges->check("ProcessCreditCards"))
  {
    _errorMsg = errorMsg(-2);
    return -2;
  }

  if(!_metrics->boolean("CCAccept"))
  {
    _errorMsg = errorMsg(-3);
    return -3;
  }

  XSqlQuery ccbankq("SELECT ccbank_id"
                    "  FROM ccbank"
                    " WHERE (ccbank_bankaccnt_id IS NOT NULL);");
  if (ccbankq.first())
    ; // we're ok - we can accept at least one credit card
  else if (ccbankq.lastError().type() != QSqlError::NoError)
  {
    _errorMsg = ccbankq.lastError().text();
    return -1;
  }
  else
  {
    _errorMsg = errorMsg(-4);
    return -4;
  }

  if (omfgThis->_key.isEmpty())
  {
    _errorMsg = errorMsg(-5);
    return -5;
  }

  return 0;
}

int ExternalCCProcessor::doAuthorize(const int pccardid, const int pcvv, double &pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString& pneworder, QString& preforder, int &pccpayid, ParameterList &pparams)
{
  if (DEBUG)
    qDebug("ExtCC:doAuthorize(%d, %d, %f, %f, %d, %f, %f, %d, %s, %s, %d)",
	   pccardid, pcvv, pamount, ptax, ptaxexempt,  pfreight,  pduty, pcurrid,
	   qPrintable(pneworder), qPrintable(preforder), pccpayid);

  int returnValue = handleTrans(pccardid, "A", pcvv, pamount, pcurrid,
                                pneworder, preforder, pccpayid, pparams);

  return returnValue;
}

int  ExternalCCProcessor::doCharge(const int pccardid, const int pcvv, const double pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString& pneworder, QString& preforder, int &pccpayid, ParameterList &pparams)
{
  if (DEBUG)
    qDebug("ExtCC:doCharge(%d, %d, %f, %f, %d, %f, %f, %d, %s, %s, %d)",
	   pccardid, pcvv, pamount,  ptax, ptaxexempt,  pfreight,  pduty, pcurrid,
	   qPrintable(pneworder), qPrintable(preforder), pccpayid);

  int returnValue = handleTrans(pccardid, "C", pcvv, pamount, pcurrid,
                                pneworder, preforder, pccpayid, pparams);
  return returnValue;
}

int ExternalCCProcessor::doChargePreauthorized(const int pccardid, const int pcvv, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, ParameterList &pparams)
{
  if (DEBUG)
    qDebug("ExtCC:doChargePreauthorized(%d, %d, %f, %d, %s, %s, %d)",
	   pccardid, pcvv, pamount,  pcurrid,
	   qPrintable(pneworder), qPrintable(preforder), pccpayid);

  int returnValue = handleTrans(pccardid, "CP", pcvv, pamount, pcurrid,
                                pneworder, preforder, pccpayid, pparams);
  return returnValue;
}

int ExternalCCProcessor::doCredit(const int pccardid, const int pcvv, const double pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, ParameterList &pparams)
{
  if (DEBUG)
    qDebug("ExtCC:doCredit(%d, %d, %f, %f, %d, %f, %f, %d, %s, %s, %d)",
	   pccardid, pcvv, pamount, ptax, ptaxexempt,  pfreight,  pduty, pcurrid,
	   qPrintable(pneworder), qPrintable(preforder), pccpayid);

  int returnValue = handleTrans(pccardid, "R", pcvv, pamount, pcurrid,
                                pneworder, preforder, pccpayid, pparams);
  return returnValue;
}

int ExternalCCProcessor::doVoidPrevious(const int pccardid, const int pcvv, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, QString &papproval, int &pccpayid, ParameterList &pparams)
{
  if (DEBUG)
    qDebug("ExtCC:doVoidPrevious(%d, %d, %f, %d, %s, %s, %s, %d)",
	   pccardid, pcvv, pamount, pcurrid,
	   qPrintable(pneworder), qPrintable(preforder),
	   qPrintable(papproval), pccpayid);

  int returnValue = handleTrans(pccardid, "V", pcvv, pamount, pcurrid,
                                pneworder, preforder, pccpayid, pparams);
  return returnValue;
}

int ExternalCCProcessor::handleTrans(const int pccardid, const QString &ptype, const int pcvv, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, ParameterList &pparams)
{
  if (DEBUG)
    qDebug("ExtCC::handleTrans(%d, %s, %d, %f, %d, %s, %s, %d, pparams)",
	   pccardid, qPrintable(ptype), pcvv, pamount, pcurrid,
           qPrintable(pneworder), qPrintable(preforder), pccpayid);

  // TODO: if check and not credit card transaction do something else
  XSqlQuery extq;
  extq.prepare(
    "SELECT ccard_active,"
    "  formatccnumber(decrypt(setbytea(ccard_number),setbytea(:key),'bf')) AS ccard_number_x,"
    "  formatbytea(decrypt(setbytea(ccard_month_expired),setbytea(:key),'bf')) AS ccard_month_expired,"
    "  formatbytea(decrypt(setbytea(ccard_year_expired),setbytea(:key), 'bf')) AS ccard_year_expired,"
    "  ccard_cust_id "
    "  FROM ccard "
    "WHERE (ccard_id=:ccardid);");
  extq.bindValue(":ccardid", pccardid);
  extq.bindValue(":key",     omfgThis->_key);
  extq.exec();

  if (extq.first())
  {
    if (!extq.value("ccard_active").toBool())
    {
      _errorMsg = errorMsg(-10);
      return -10;
    }
  }
  else if (extq.lastError().type() != QSqlError::NoError)
  {
    _errorMsg = extq.lastError().databaseText();
    return -1;
  }
  else
  {
    _errorMsg = errorMsg(-17).arg(pccardid);
    return -17;
  }

  pparams.append("cust_id",        extq.value("ccard_cust_id"));
  pparams.append("ccard_number_x", extq.value("ccard_number_x"));
  pparams.append("ccard_exp",      QDate(extq.value("ccard_year_expired").toInt(),
                                   extq.value("ccard_month_expired").toInt(), 1));
  pparams.append("ccard_id",   pccardid);
  pparams.append("currid",     pcurrid);
  pparams.append("amount",     pamount);
  pparams.append("auth_charge",ptype);
  pparams.append("type",       ptype);
  pparams.append("reforder",   (preforder.isEmpty()) ? pneworder : preforder);
  pparams.append("ordernum",   pneworder);
  pparams.append("auth",       QVariant(ptype == "A"));

  externalCCTransaction newdlg(0, "", true);
  newdlg.set(pparams);
  int returnValue = 0;
  if (newdlg.exec() == QDialog::Rejected)
  {
    if (ptype == "A")
      returnValue = -70;
    else if (ptype == "C")
      returnValue = -72;
    else if (ptype == "CP")
      returnValue = -71;
    else if (ptype == "R")
      returnValue = -73;
    else if (ptype == "V")
      returnValue = -74;
    else
      returnValue = -19;

    _errorMsg = errorMsg(returnValue).arg(ptype);
  }
  else
  {
    newdlg.getResults(pparams);

    QVariant param;
    bool valid;

    param = pparams.value("passedavs", &valid);
    if (valid)
      _passedAvs = param.toBool();

    param = pparams.value("passedcvv", &valid);
    if (valid)
      _passedCvv = param.toBool();

    param = pparams.value("approved", &valid);
    if (valid)
    {
      if (param.toString() == "ERROR")
      {
        _errorMsg = errorMsg(-12).arg(tr("User reported that an error occurred."));
        returnValue = -12;
      }
      else if (param.toString() == "DECLINED")
      {
        _errorMsg = errorMsg(-92).arg("");
        returnValue = -92;
      } 
      else if (param.toString().isEmpty())
      {
        _errorMsg = errorMsg(-100).arg(tr("The 'approved' parameter is empty; "
                                          "this should never happen."));
        returnValue = -100;
      }
    }
  }

  if (DEBUG)
    qDebug("ExtCC::handleTrans returning %d %s",
           returnValue, qPrintable(errorMsg()));
  return returnValue;
}

bool ExternalCCProcessor::handlesCreditCards()
{
  return true;
}
