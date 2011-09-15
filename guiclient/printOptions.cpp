/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printOptions.h"
#include "xtsettings.h"

#include <QPrinterInfo>

printOptions::printOptions(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_printerGroup, SIGNAL(toggled(bool)), this, SLOT(sHandleCheckbox()));

  QPrinterInfo pinfo;
  QList<QPrinterInfo> plist = pinfo.availablePrinters();
  for (int i = 0; i < plist.size(); ++i)
     _printers->addItem(plist.at(i).printerName());
}

printOptions::~printOptions()
{
  // no need to delete child widgets, Qt does it all for us
}

void printOptions::languageChange()
{
  retranslateUi(this);
}

enum SetResponse printOptions::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("parentName", &valid);
  if (valid) {
    _parentName = param.toString();
    _autoPrint->setChecked(xtsettingsValue(QString("%1.autoPrint").arg(_parentName)).toBool());
    _printers->setCurrentIndex(_printers->findText(xtsettingsValue(QString("%1.defaultPrinter").arg(_parentName)).toString()));
    _printerGroup->setChecked(!_printers->currentText().isEmpty());
  }

  return NoError;
}

void printOptions::sHandleCheckbox()
{
  if (!_printerGroup->isChecked())
    _printers->clearEditText();
}

void printOptions::sSave()
{
  xtsettingsSetValue(QString("%1.autoPrint").arg(_parentName), (bool)_autoPrint->isChecked());
  xtsettingsSetValue(QString("%1.defaultPrinter").arg(_parentName), _printers->currentText());
  accept();
}


