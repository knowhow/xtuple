/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "configureWO.h"

#include <QValidator>
#include <QVariant>

#include "guiclient.h"

configureWO::configureWO(QWidget* parent, const char* name, bool /*modal*/, Qt::WFlags fl)
    : XAbstractConfigure(parent, fl)
{
  setupUi(this);

  if (name)
    setObjectName(name);

  connect(_autoExplode, SIGNAL(toggled(bool)), _WOExplosionGroup, SLOT(setDisabled(bool)));
  connect(_autoExplode, SIGNAL(toggled(bool)), _multiLevel, SLOT(setChecked(bool)));

  _nextWoNumber->setValidator(omfgThis->orderVal());

  q.exec( "SELECT orderseq_number "
          "FROM orderseq "
          "WHERE (orderseq_name='WoNumber')" );
  if (q.first())
    _nextWoNumber->setText(q.value("orderseq_number").toString());

  _autoExplode->setChecked(_metrics->boolean("AutoExplodeWO"));
  _workOrderChangeLog->setChecked(_metrics->boolean("WorkOrderChangeLog"));

  _woNumGeneration->append(0, tr("Automatic"),                  "A");
  _woNumGeneration->append(1, tr("Manual"),                     "M");
  _woNumGeneration->append(2, tr("Automatic, Allow Override"),  "O");

  _woNumGeneration->setCode(_metrics->value("WONumberGeneration"));

  if (_metrics->value("ExplodeWOEffective") == "E")
    _explodeDateEffective->setChecked(TRUE);
  else
    _startDateEffective->setChecked(TRUE);

  if (_metrics->value("WOExplosionLevel") == "S")
    _singleLevel->setChecked(TRUE);
  else if (_metrics->value("WOExplosionLevel") == "M")
    _multiLevel->setChecked(TRUE);

  _materialVariances->setChecked(_metrics->boolean("PostMaterialVariances"));
  
  if (_metrics->value("JobItemCosDefault") == "P")
    _proportional->setChecked(TRUE);
  else
    _todate->setChecked(TRUE);
    
  this->setWindowTitle("Manufacture Configuration");
}

configureWO::~configureWO()
{
  // no need to delete child widgets, Qt does it all for us
}

void configureWO::languageChange()
{
  retranslateUi(this);
}

bool configureWO::sSave()
{
  emit saving();

  q.prepare("SELECT setNextWoNumber(:woNumber) AS result;");
  q.bindValue(":woNumber", _nextWoNumber->text().toInt());
  q.exec();

  _metrics->set("AutoExplodeWO", _autoExplode->isChecked());
  _metrics->set("WorkOrderChangeLog", _workOrderChangeLog->isChecked());

  _metrics->set("WONumberGeneration", _woNumGeneration->code());

  _metrics->set("ExplodeWOEffective", ((_explodeDateEffective->isChecked()) ? QString("E") : QString("S")));
  _metrics->set("WOExplosionLevel", ((_singleLevel->isChecked()) ? QString("S") : QString("M")));
  _metrics->set("PostMaterialVariances", _materialVariances->isChecked());

  if (_todate->isChecked())
    _metrics->set("JobItemCosDefault", QString("D"));
  else 
    _metrics->set("JobItemCosDefault", QString("P"));

  return true;
}

