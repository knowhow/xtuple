/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "getGLDistDate.h"

getGLDistDate::getGLDistDate(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);
  _date->setDate(QDate::currentDate());
}

getGLDistDate::~getGLDistDate()
{
  // no need to delete child widgets, Qt does it all for us
}

void getGLDistDate::languageChange()
{
  retranslateUi(this);
}

QDate getGLDistDate::date() const
{
  if (_default->isChecked())
    return QDate();
  else
    return _date->date();
}

void getGLDistDate::sSetDefaultLit(const QString & pStr)
{
  _default->setText(pStr);
}
