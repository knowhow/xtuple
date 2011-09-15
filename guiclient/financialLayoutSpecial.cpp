/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "financialLayoutSpecial.h"

#include <QVariant>
#include <QMessageBox>

financialLayoutSpecial::financialLayoutSpecial(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_showBudget, SIGNAL(toggled(bool)), _showBudgetPrcnt, SLOT(setEnabled(bool)));
  connect(_showDB, SIGNAL(toggled(bool)), _showDBPrcnt, SLOT(setEnabled(bool)));
  connect(_showEnding, SIGNAL(toggled(bool)), _showEndingPrcnt, SLOT(setEnabled(bool)));
  connect(_showDiff, SIGNAL(toggled(bool)), _showDiffPrcnt, SLOT(setEnabled(bool)));
  connect(_showBeginning, SIGNAL(toggled(bool)), _showBeginningPrcnt, SLOT(setEnabled(bool)));

  _flheadid = -1;
  _flspecid = -1;
}

financialLayoutSpecial::~financialLayoutSpecial()
{
  // no need to delete child widgets, Qt does it all for us
}

void financialLayoutSpecial::languageChange()
{
  retranslateUi(this);
}

enum SetResponse financialLayoutSpecial::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("flhead_id", &valid);
  if (valid)
  {
    _flheadid = param.toInt();
    sFillGroupList();
  }

  param = pParams.value("flgrp_id", &valid);
  if (valid)
    _flgrpid = param.toInt();

  param = pParams.value("flspec_id", &valid);
  if (valid)
  {
    _flspecid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _buttonBox->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _buttonBox->setFocus();
    }
  }

  return NoError;
}

void financialLayoutSpecial::sCheck()
{
}

void financialLayoutSpecial::sSave()
{
  int order = 1;
  if (_mode == cNew)
  {
    q.prepare("SELECT COALESCE(MAX(ord),0) + 1 AS neworder"
              "  FROM (SELECT flgrp_order AS ord"
              "          FROM flgrp"
              "         WHERE ((flgrp_flgrp_id=:flgrp_id)"
              "           AND  (flgrp_flhead_id=:flhead_id))"
              "         UNION"
              "        SELECT flitem_order AS ord"
              "          FROM flitem"
              "         WHERE ((flitem_flgrp_id=:flgrp_id)"
              "           AND  (flitem_flhead_id=:flhead_id))"
              "         UNION"
              "        SELECT flspec_order AS ord"
              "          FROM flspec"
              "         WHERE ((flspec_flgrp_id=:flgrp_id)"
              "           AND  (flspec_flhead_id=:flhead_id)) ) AS data;" );
    q.bindValue(":flgrp_id", _flgrpid);
    q.bindValue(":flhead_id", _flheadid);
    q.exec();

    if(q.first())
      order = q.value("neworder").toInt();
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('flspec_flspec_id_seq') AS flspec_id;");
    if (q.first())
      _flspecid = q.value("flspec_id").toInt();
    
    q.prepare( "INSERT INTO flspec "
               "( flspec_id, flspec_flhead_id, flspec_flgrp_id, flspec_order,"
               "  flspec_name, flspec_type, flspec_showstart, flspec_showend, flspec_showdelta,"
               "  flspec_showbudget, flspec_showdiff, flspec_showcustom,"
               "  flspec_subtract, flspec_showstartprcnt, flspec_showendprcnt, flspec_showdeltaprcnt,"
               "  flspec_showbudgetprcnt, flspec_showdiffprcnt, flspec_showcustomprcnt,"
               "  flspec_prcnt_flgrp_id, flspec_custom_source) "
               "VALUES "
               "( :flspec_id, :flspec_flhead_id, :flspec_flgrp_id, :flspec_order,"
               "  :flspec_name, :flspec_type, :flspec_showstart, :flspec_showend, :flspec_showdelta,"
               "  :flspec_showbudget, :flspec_showdiff, :flspec_showcustom,"
               "  :flspec_subtract, :flspec_showstartprcnt, :flspec_showendprcnt, :flspec_showdeltaprcnt,"
               "  :flspec_showbudgetprcnt, :flspec_showdiffprcnt, :flspec_showcustomprcnt,"
               "  :flspec_prcnt_flgrp_id, :flspec_custom_source);" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE flspec "
               "SET flspec_name=:flspec_name, flspec_type=:flspec_type,"
               "    flspec_showstart=:flspec_showstart,"
               "    flspec_showend=:flspec_showend, flspec_showdelta=:flspec_showdelta,"
               "    flspec_showbudget=:flspec_showbudget, flspec_subtract=:flspec_subtract,"
               "    flspec_showdiff=:flspec_showdiff, flspec_showcustom=:flspec_showcustom,"
               "    flspec_showstartprcnt=:flspec_showstartprcnt, flspec_showendprcnt=:flspec_showendprcnt,"
               "    flspec_showdeltaprcnt=:flspec_showdeltaprcnt, flspec_showbudgetprcnt=:flspec_showbudgetprcnt,"
               "    flspec_showdiffprcnt=:flspec_showdiffprcnt, flspec_showcustomprcnt=:flspec_showcustomprcnt,"
               "    flspec_prcnt_flgrp_id=:flspec_prcnt_flgrp_id, flspec_custom_source=:flspec_custom_source "
               "WHERE (flspec_id=:flspec_id);" );
  
  q.bindValue(":flspec_flhead_id", _flheadid);
  q.bindValue(":flspec_flgrp_id", _flgrpid);
  q.bindValue(":flspec_order", order);
  q.bindValue(":flspec_name", _name->text());
  q.bindValue(":flspec_showstart", QVariant(_showBeginning->isChecked()));
  q.bindValue(":flspec_showend", QVariant(_showEnding->isChecked()));
  q.bindValue(":flspec_showdelta", QVariant(_showDB->isChecked()));
  q.bindValue(":flspec_showbudget", QVariant(_showBudget->isChecked()));
  q.bindValue(":flspec_showdiff", QVariant(_showDiff->isChecked()));
  q.bindValue(":flspec_showcustom", QVariant(_showCustom->isChecked()));
  q.bindValue(":flspec_subtract", QVariant(_subtract->isChecked()));
  q.bindValue(":flspec_showstartprcnt", QVariant(_showBeginning->isChecked() && _showBeginningPrcnt->isChecked()));
  q.bindValue(":flspec_showendprcnt", QVariant(_showEnding->isChecked() && _showEndingPrcnt->isChecked()));
  q.bindValue(":flspec_showdeltaprcnt", QVariant(_showDB->isChecked() && _showDBPrcnt->isChecked()));
  q.bindValue(":flspec_showbudgetprcnt", QVariant(_showBudget->isChecked() && _showBudgetPrcnt->isChecked()));
  q.bindValue(":flspec_showdiffprcnt", QVariant(_showDiff->isChecked() && _showDiffPrcnt->isChecked()));
  q.bindValue(":flspec_showcustomprcnt", QVariant(_showCustom->isChecked() && _showCustomPrcnt->isChecked()));
  q.bindValue(":flspec_prcnt_flgrp_id", _group->id());
  q.bindValue(":flspec_id", _flspecid);

  if(_customUseBeginning->isChecked())
    q.bindValue(":flspec_custom_source", "S");
  else if(_customUseEnding->isChecked())
    q.bindValue(":flspec_custom_source", "E");
  else if(_customUseDebits->isChecked())
    q.bindValue(":flspec_custom_source", "D");
  else if(_customUseCredits->isChecked())
    q.bindValue(":flspec_custom_source", "C");
  else if(_customUseBudget->isChecked())
    q.bindValue(":flspec_custom_source", "B");
  else if(_customUseDiff->isChecked())
    q.bindValue(":flspec_custom_source", "F");

  switch(_type->currentIndex())
  {
    case 1:
      q.bindValue(":flspec_type", "OpenAP");
      break;
    case 0:
    default:
      q.bindValue(":flspec_type", "OpenAR");
  }

  q.exec();
  
  done(_flspecid);
}

void financialLayoutSpecial::populate()
{
  q.prepare( "SELECT * "
             "FROM flspec "
             "WHERE (flspec_id=:flspec_id);" );
  q.bindValue(":flspec_id", _flspecid);
  q.exec();
  if (q.first())
  {
    _name->setText(q.value("flspec_name").toString());
    _showBeginning->setChecked(q.value("flspec_showstart").toBool());
    _showEnding->setChecked(q.value("flspec_showend").toBool());
    _showDB->setChecked(q.value("flspec_showdelta").toBool());
    _showBudget->setChecked(q.value("flspec_showbudget").toBool());
    _showDiff->setChecked(q.value("flspec_showdiff").toBool());
    _showCustom->setChecked(q.value("flspec_showcustom").toBool());
    _showBeginningPrcnt->setChecked(q.value("flspec_showstartprcnt").toBool());
    _showEndingPrcnt->setChecked(q.value("flspec_showendprcnt").toBool());
    _showDBPrcnt->setChecked(q.value("flspec_showdeltaprcnt").toBool());
    _showBudgetPrcnt->setChecked(q.value("flspec_showbudgetprcnt").toBool());
    _showDiffPrcnt->setChecked(q.value("flspec_showdiffprcnt").toBool());
    _showCustomPrcnt->setChecked(q.value("flspec_showcustomprcnt").toBool());

    QString src = q.value("flspec_custom_source").toString();
    if("S" == src)
      _customUseBeginning->setChecked(true);
    else if("E" == src)
      _customUseEnding->setChecked(true);
    else if("D" == src)
      _customUseDebits->setChecked(true);
    else if("C" == src)
      _customUseCredits->setChecked(true);
    else if("B" == src)
      _customUseBudget->setChecked(true);
    else if("F" == src)
      _customUseDiff->setChecked(true);

    if(q.value("flspec_subtract").toBool())
      _subtract->setChecked(true);
    else
      _add->setChecked(true);

    if(q.value("flspec_type").toString() == "OpenAP")
      _type->setCurrentIndex(1);
    else //if(q.value("flspec_type").toString() == "OpenAR")
      _type->setCurrentIndex(0);

    _flheadid = q.value("flspec_flhead_id").toInt();

    int grpid = q.value("flspec_prcnt_flgrp_id").toInt();
    sFillGroupList();
    _group->setId(grpid);
  }
}

void financialLayoutSpecial::sFillGroupList()
{
  _group->clear();
  q.prepare("SELECT flgrp_id, flgrp_name"
            "  FROM flgrp"
            " WHERE (flgrp_flhead_id=:flhead_id)"
            " ORDER BY flgrp_name;");
  q.bindValue(":flhead_id", _flheadid);
  q.exec();
  _group->append(-1, tr("Parent"));
  while(q.next())
    _group->append(q.value("flgrp_id").toInt(), q.value("flgrp_name").toString());
}

