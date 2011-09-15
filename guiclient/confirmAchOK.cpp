/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "confirmAchOK.h"

confirmAchOK::confirmAchOK(QWidget* parent, const char* name, Qt::WFlags fl)
    : XDialog(parent, name, fl)
{
  setupUi(this);
  QFont fixedFont("Courier");
  fixedFont.setStyleHint(QFont::TypeWriter);
  fixedFont.setPointSize(fixedFont.pointSize() - 2);
  _fileContents->setFont(fixedFont);
}

confirmAchOK::~confirmAchOK()
{
  // no need to delete child widgets, Qt does it all for us
}

void confirmAchOK::languageChange()
{
  retranslateUi(this);
}

bool confirmAchOK::askOK(QWidget *parent, QFile &pfile)
{
  confirmAchOK newdlg(parent, "confirmAchOK");
  newdlg._filename->setText(pfile.fileName());
  if (! pfile.open(QIODevice::ReadOnly))
    return false;
  newdlg._fileContents->setPlainText(QString(pfile.readAll()));
  pfile.close();
  return (newdlg.exec() == QDialog::Accepted);
}
