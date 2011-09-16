/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "configureMS.h"

#include <QVariant>
#include <QValidator>

configureMS::configureMS(QWidget* parent, const char* name, bool /*modal*/, Qt::WFlags fl)
    : XAbstractConfigure(parent, fl)
{
  setupUi(this);

  if (name)
    setObjectName(name);

  _nextPlanNumber->setValidator(omfgThis->orderVal());

  q.exec("SELECT currentPlanNumber() AS result;");
  if (q.first())
    _nextPlanNumber->setText(q.value("result").toString());

  int cid = _metrics->value("DefaultMSCalendar").toInt();
  if(cid > 0)
    _calendar->setId(cid);
    
  this->setWindowTitle("Schedule Configuration");
}

configureMS::~configureMS()
{
  // no need to delete child widgets, Qt does it all for us
}

void configureMS::languageChange()
{
  retranslateUi(this);
}

bool configureMS::sSave()
{
  emit saving();

  XSqlQuery configq;
  configq.prepare("SELECT setNextPlanNumber(:planord_number) AS result;");
  configq.bindValue(":planord_number", _nextPlanNumber->text().toInt());
  configq.exec();

  _metrics->set("DefaultMSCalendar", _calendar->id());
  return true;
}
