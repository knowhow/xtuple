/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xdateinputdialog.h"

#include <QMessageBox>
#include <QValidator>
#include <QVariant>

XDateInputDialog::XDateInputDialog(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    _date->setDate(QDate::currentDate());
}

XDateInputDialog::~XDateInputDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

void XDateInputDialog::languageChange()
{
    retranslateUi(this);
}

enum SetResponse XDateInputDialog::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("label", &valid);
  if (valid)
    _label->setText(param.toString());

  param = pParams.value("default", &valid);
  if (valid)
    _date->setDate(param.toDate());

  return NoError;
}

QDate XDateInputDialog::getDate()
{
  return _date->date();
}
