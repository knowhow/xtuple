/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "externalCCTransaction.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include <QTextStream>
#include <QFileDialog>
#include <QFile>

externalCCTransaction::externalCCTransaction(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);
}

externalCCTransaction::~externalCCTransaction()
{
  // no need to delete child widgets, Qt does it all for us
}

void externalCCTransaction::languageChange()
{
  retranslateUi(this);
}

enum SetResponse externalCCTransaction::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("cust_id", &valid);
  if (valid)
    _cust->setId(param.toInt());

  param = pParams.value("ccard_number_x", &valid);
  if (valid)
    _ccard->setText(param.toString());

  param = pParams.value("currid", &valid);
  if (valid)
    _amount->setId(param.toInt());

  param = pParams.value("amount", &valid);
  if (valid)
    _amount->setLocalValue(param.toDouble());

  param = pParams.value("type", &valid);
  if (valid)
  {
    if (param.toString() == "A")
      _transType->setCurrentIndex(0);
    else if (param.toString() == "C")
      _transType->setCurrentIndex(1);
    else if (param.toString() == "CP")
      _transType->setCurrentIndex(2);
    else if (param.toString() == "R")
      _transType->setCurrentIndex(3);
    else if (param.toString() == "V")
      _transType->setCurrentIndex(4);
  }

  param = pParams.value("reforder", &valid);
  if (valid)
    _order->setText(param.toString());

  param = pParams.value("ordernum", &valid);
  if (valid)
    _transactionId->setText(param.toString());

  return NoError;
}

void externalCCTransaction::getResults(ParameterList &pParams)
{
  QVariant amount = pParams.value("amount");
  int amountIndex;
  for (amountIndex = 0; amountIndex < pParams.size(); amountIndex++)
    if (pParams.name(amountIndex) == "amount")
      break;

  switch (_approved->currentIndex())
  {
    case 0:
      pParams.append("approved", "APPROVED");
      if (_transType->currentIndex() == 0)
        pParams.append("status", "A");
      else if (_transType->currentIndex() == 4)
        pParams.append("status", "V");
      else
        pParams.append("status", "C");
      break;
    case 1:
      pParams.append("approved", "DECLINED");
      pParams.append("status",   "D");
      if (amountIndex < pParams.size())
        pParams.removeAt(amountIndex);
      pParams.append("amount",   "0");
      break;
    case 2:
      pParams.append("approved", "ERROR");
      pParams.append("status",   "X");
      if (amountIndex < pParams.size())
        pParams.removeAt(amountIndex);
      pParams.append("amount",   "0");
      break;
    case 3:
      pParams.append("approved", "HELDFORREVIEW");
      if (_transType->currentIndex() == 0)
        pParams.append("status", "A");
      else if (_transType->currentIndex() == 4)
        pParams.append("status", "V");
      else
        pParams.append("status", "C");
      break;
    default:
      pParams.append("status",  "X");
      if (amountIndex < pParams.size())
        pParams.removeAt(amountIndex);
      pParams.append("amount",  "0");
  }

  pParams.append("code",     _approvalCode->text());
  pParams.append("xactionid",_transactionId->text());
  pParams.append("avs",      _passedAVS->isChecked() ? tr("passed") :
                                                 tr("failed or not entered"));
  pParams.append("passedavs",QVariant(_passedAVS->isChecked()));
  pParams.append("passedcvv",QVariant(_passedCVV->isChecked()));
  //pParams.append("error",    );
  //pParams.append("shipping",    );
  //pParams.append("tax",     );
  //pParams.append("ref",     );
  //pParams.append("message", );
}
