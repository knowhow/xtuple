/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "financialLayoutGroup.h"

#include <QVariant>

#define cIncome   0
#define cBalance  1
#define cCash     2
#define cAdHoc    3

/*
 *  Constructs a financialLayoutGroup as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
financialLayoutGroup::financialLayoutGroup(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_showSubtotal, SIGNAL(toggled(bool)), this, SLOT(sToggled()));
  connect(_summarize, SIGNAL(toggled(bool)), this, SLOT(sToggled()));
  connect(_summarize, SIGNAL(toggled(bool)), _showSubtotal, SLOT(setDisabled(bool)));
  connect(_showBudget, SIGNAL(toggled(bool)), _showBudgetPrcnt, SLOT(setEnabled(bool)));
  connect(_showDelta, SIGNAL(toggled(bool)), _showDeltaPrcnt, SLOT(setEnabled(bool)));
  connect(_showEnd, SIGNAL(toggled(bool)), _showEndPrcnt, SLOT(setEnabled(bool)));
  connect(_showStart, SIGNAL(toggled(bool)), _showStartPrcnt, SLOT(setEnabled(bool)));
  connect(_showDiff, SIGNAL(toggled(bool)), _showDiffPrcnt, SLOT(setEnabled(bool)));
  connect(_showCustom, SIGNAL(toggled(bool)), _showCustomPrcnt, SLOT(setEnabled(bool)));
  connect(_showPrcnt, SIGNAL(toggled(bool)), this, SLOT(sToggleShowPrcnt()));
}

/*
 *  Destroys the object and frees any allocated resources
 */
financialLayoutGroup::~financialLayoutGroup()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void financialLayoutGroup::languageChange()
{
  retranslateUi(this);
}

enum SetResponse financialLayoutGroup::set(const ParameterList &pParams)
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

  param = pParams.value("flgrp_flgrp_id", &valid);
  if (valid)
    _flgrp_flgrpid = param.toInt();

  param = pParams.value("flgrp_id", &valid);
  if (valid)
  {
    _flgrpid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      
      _name->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      
      _buttonBox->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
      _buttonBox->setFocus();
      _name->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _subSummGroup->setEnabled(FALSE);
      _operationGroup->setEnabled(FALSE);
    }
  }
  param = pParams.value("type", &valid);
    if (valid)
    {
        if (param.toString() == "adHoc")
        {
			_rpttype = cAdHoc;
			_showPrcnt->setHidden(TRUE);
		}
        else
        {
        
          _showStart->setHidden(TRUE);
		  _showEnd->setHidden(TRUE);
          _showDelta->setHidden(TRUE);
          _showBudget->setHidden(TRUE);
          _showDiff->setHidden(TRUE);
          _showCustom->setHidden(TRUE);
          _showStartPrcnt->setHidden(TRUE);
          _showEndPrcnt->setHidden(TRUE);
          _showDeltaPrcnt->setHidden(TRUE);
          _showBudgetPrcnt->setHidden(TRUE);
          _showDiffPrcnt->setHidden(TRUE);
          _showCustomPrcnt->setHidden(TRUE);
    
        }
        if (param.toString() == "income")
        {
          _rpttype = cIncome;
          _showPrcnt->setHidden(FALSE);
		  _showStart->setChecked(FALSE);
          _showEnd->setChecked(FALSE);
          _showDelta->setChecked(FALSE);
          _showBudget->setChecked(TRUE);
          _showDiff->setChecked(TRUE);
      
        }
        else if (param.toString() == "balance")
        {
        _rpttype = cBalance;
      _showPrcnt->setHidden(FALSE);
      _showStart->setChecked(FALSE);
      _showEnd->setChecked(TRUE);
      _showDelta->setChecked(FALSE);
      _showBudget->setChecked(TRUE);
      _showDiff->setChecked(FALSE);
        }
        else if (param.toString() == "cash")
        {
         _rpttype = cCash;
      _showPrcnt->setHidden(FALSE);
      _showStart->setChecked(FALSE);
      _showEnd->setChecked(FALSE);
      _showDelta->setChecked(TRUE);
      _showBudget->setChecked(TRUE);
      _showDiff->setChecked(TRUE);
      }
    }

  return NoError;
}

void financialLayoutGroup::sCheck()
{
}

void financialLayoutGroup::sSave()
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
    q.bindValue(":flgrp_id", _flgrp_flgrpid);
    q.bindValue(":flhead_id", _flheadid);
    q.exec();

    if(q.first())
      order = q.value("neworder").toInt();
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('flgrp_flgrp_id_seq') AS flgrp_id;");
    if (q.first())
      _flgrpid = q.value("flgrp_id").toInt();
    
    q.prepare( "INSERT INTO flgrp "
               "( flgrp_id, flgrp_flhead_id, flgrp_flgrp_id, flgrp_name, flgrp_descrip, flgrp_order, flgrp_subtotal,"
               "  flgrp_summarize, flgrp_subtract, flgrp_showstart, flgrp_showend, flgrp_showdelta, flgrp_showbudget, flgrp_showdiff, flgrp_showcustom,"
               "  flgrp_showstartprcnt, flgrp_showendprcnt, flgrp_showdeltaprcnt, flgrp_showbudgetprcnt, flgrp_showdiffprcnt, flgrp_showcustomprcnt,"
               "  flgrp_prcnt_flgrp_id, flgrp_usealtsubtotal, flgrp_altsubtotal ) "
               "VALUES "
               "( :flgrp_id, :flgrp_flhead_id, :flgrp_flgrp_id, :flgrp_name, :flgrp_descrip, :flgrp_order, :flgrp_subtotal,"
               "  :flgrp_summarize, :flgrp_subtract, :flgrp_showstart, :flgrp_showend, :flgrp_showdelta, :flgrp_showbudget, :flgrp_showdiff, :flgrp_showcustom,"
               "  :flgrp_showstartprcnt, :flgrp_showendprcnt, :flgrp_showdeltaprcnt, :flgrp_showbudgetprcnt, :flgrp_showdiffprcnt, :flgrp_showcustomprcnt,"
               "  :flgrp_prcnt_flgrp_id, :flgrp_usealtsubtotal, :flgrp_altsubtotal );" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE flgrp "
               "SET flgrp_name=:flgrp_name, flgrp_descrip=:flgrp_descrip,"
               "    flgrp_subtotal=:flgrp_subtotal,"
               "    flgrp_summarize=:flgrp_summarize,"
               "    flgrp_subtract=:flgrp_subtract,"
               "    flgrp_showstart=:flgrp_showstart,"
               "    flgrp_showend=:flgrp_showend,"
               "    flgrp_showdelta=:flgrp_showdelta,"
               "    flgrp_showbudget=:flgrp_showbudget,"
               "    flgrp_showdiff=:flgrp_showdiff,"
               "    flgrp_showcustom=:flgrp_showcustom,"
               "    flgrp_showstartprcnt=:flgrp_showstartprcnt,"
               "    flgrp_showendprcnt=:flgrp_showendprcnt,"
               "    flgrp_showdeltaprcnt=:flgrp_showdeltaprcnt,"
               "    flgrp_showbudgetprcnt=:flgrp_showbudgetprcnt,"
               "    flgrp_showdiffprcnt=:flgrp_showdiffprcnt,"
               "    flgrp_showcustomprcnt=:flgrp_showcustomprcnt,"
               "    flgrp_prcnt_flgrp_id=:flgrp_prcnt_flgrp_id,"
               "    flgrp_usealtsubtotal=:flgrp_usealtsubtotal,"
               "    flgrp_altsubtotal=:flgrp_altsubtotal "
               "WHERE (flgrp_id=:flgrp_id);" );
    
  q.bindValue(":flgrp_id", _flgrpid);
  q.bindValue(":flgrp_flgrp_id", _flgrp_flgrpid);
  q.bindValue(":flgrp_flhead_id", _flheadid);
  q.bindValue(":flgrp_name", _name->text());
  q.bindValue(":flgrp_descrip", _description->text());
  q.bindValue(":flgrp_order", order);
  q.bindValue(":flgrp_subtotal",  QVariant(_showSubtotal->isChecked()));
  q.bindValue(":flgrp_summarize", QVariant(_summarize->isChecked()));
  q.bindValue(":flgrp_subtract",  QVariant(_subtract->isChecked()));
  q.bindValue(":flgrp_showstart", QVariant(((_showSubtotal->isChecked() || _summarize->isChecked()) && _showStart->isChecked())));
  q.bindValue(":flgrp_showend",   QVariant(((_showSubtotal->isChecked() || _summarize->isChecked()) && _showEnd->isChecked())));
  q.bindValue(":flgrp_showdelta", QVariant(((_showSubtotal->isChecked() || _summarize->isChecked()) && _showDelta->isChecked())));
  q.bindValue(":flgrp_showbudget", QVariant(((_showSubtotal->isChecked() || _summarize->isChecked()) && _showBudget->isChecked())));
  q.bindValue(":flgrp_showdiff",  QVariant(((_showSubtotal->isChecked() || _summarize->isChecked()) && _showDiff->isChecked())));
  q.bindValue(":flgrp_showcustom", QVariant(((_showSubtotal->isChecked() || _summarize->isChecked()) && _showCustom->isChecked())));
  q.bindValue(":flgrp_showstartprcnt", QVariant(((_showSubtotal->isChecked() || _summarize->isChecked()) && _showStart->isChecked() && _showStartPrcnt->isChecked())));
  q.bindValue(":flgrp_showendprcnt", QVariant(((_showSubtotal->isChecked() || _summarize->isChecked()) && _showEnd->isChecked() && _showEndPrcnt->isChecked())));
  q.bindValue(":flgrp_showdeltaprcnt", QVariant(((_showSubtotal->isChecked() || _summarize->isChecked()) && _showDelta->isChecked() && _showDeltaPrcnt->isChecked())));
  q.bindValue(":flgrp_showbudgetprcnt", QVariant(((_showSubtotal->isChecked() || _summarize->isChecked()) && _showBudget->isChecked() && _showBudgetPrcnt->isChecked())));
  q.bindValue(":flgrp_showdiffprcnt", QVariant(((_showSubtotal->isChecked() || _summarize->isChecked()) && _showDiff->isChecked() && _showDiffPrcnt->isChecked())));
  q.bindValue(":flgrp_showcustomprcnt", QVariant(((_showSubtotal->isChecked() || _summarize->isChecked()) && _showCustom->isChecked() && _showCustomPrcnt->isChecked())));
  q.bindValue(":flgrp_prcnt_flgrp_id", _group->id());
  q.bindValue(":flgrp_usealtsubtotal", QVariant(_altSubtotal->isChecked()));
  q.bindValue(":flgrp_altsubtotal", _altSubtotalLabel->text());

  q.exec();
  
  done(_flgrpid);
}

void financialLayoutGroup::populate()
{
  q.prepare( "SELECT flgrp_name, flgrp_descrip, flgrp_subtotal,"
             "       flgrp_summarize, flgrp_subtract,"
             "       flgrp_showstart, flgrp_showend,"
             "       flgrp_showdelta, flgrp_showbudget, flgrp_showdiff, flgrp_showcustom,"
             "       flgrp_showstartprcnt, flgrp_showendprcnt,"
             "       flgrp_showdeltaprcnt, flgrp_showbudgetprcnt, flgrp_showdiffprcnt, flgrp_showcustomprcnt,"
             "       flgrp_flhead_id, flgrp_prcnt_flgrp_id, flgrp_usealtsubtotal, flgrp_altsubtotal "
             "FROM flgrp "
             "WHERE (flgrp_id=:flgrp_id);" );
  q.bindValue(":flgrp_id", _flgrpid);
  q.exec();
  if (q.first())
  {
    _flheadid = q.value("flgrp_flhead_id").toInt();

    _name->setText(q.value("flgrp_name").toString());
    _description->setText(q.value("flgrp_descrip").toString());
    _showSubtotal->setChecked(q.value("flgrp_subtotal").toBool());
    _summarize->setChecked(q.value("flgrp_summarize").toBool());
    _altSubtotal->setChecked(q.value("flgrp_usealtsubtotal").toBool());
    _altSubtotalLabel->setText(q.value("flgrp_altsubtotal").toString());

    if(_summarize->isChecked() || _showSubtotal->isChecked())
    {
      _showStart->setChecked(q.value("flgrp_showstart").toBool());
      _showEnd->setChecked(q.value("flgrp_showend").toBool());
      _showDelta->setChecked(q.value("flgrp_showdelta").toBool());
      _showBudget->setChecked(q.value("flgrp_showbudget").toBool());
      _showDiff->setChecked(q.value("flgrp_showdiff").toBool());
      _showCustom->setChecked(q.value("flgrp_showcustom").toBool());
      _showStartPrcnt->setChecked(q.value("flgrp_showstartprcnt").toBool());
      _showEndPrcnt->setChecked(q.value("flgrp_showendprcnt").toBool());
      _showDeltaPrcnt->setChecked(q.value("flgrp_showdeltaprcnt").toBool());
      _showBudgetPrcnt->setChecked(q.value("flgrp_showbudgetprcnt").toBool());
      _showDiffPrcnt->setChecked(q.value("flgrp_showdiffprcnt").toBool());
      _showCustomPrcnt->setChecked(q.value("flgrp_showcustomprcnt").toBool());
    }

    if(q.value("flgrp_subtract").toBool())
      _subtract->setChecked(true);
    else
      _add->setChecked(true);

  if ((_rpttype != cAdHoc) && ((_showEndPrcnt->isChecked()) || (_showDiffPrcnt->isChecked())))
    _showPrcnt->setChecked(TRUE);

    int grpid = q.value("flgrp_prcnt_flgrp_id").toInt();
    sFillGroupList();
    _group->setId(grpid);
  }
}

void financialLayoutGroup::sToggled()
{
  bool on = (_showSubtotal->isChecked() || _summarize->isChecked());
  if(_summarize->isChecked())
  {
    _altSubtotal->setChecked(false);
    _altSubtotal->setEnabled(false);
  }
  else
    _altSubtotal->setEnabled(_showSubtotal->isEnabled());
  _showStart->setEnabled(on);
  _showEnd->setEnabled(on);
  _showDelta->setEnabled(on);
  _showBudget->setEnabled(on);
  _showDiff->setEnabled(on);
  _showCustom->setEnabled(on);
  _showStartPrcnt->setEnabled(on  && _showStart->isChecked());
  _showEndPrcnt->setEnabled(on    && _showEnd->isChecked());
  _showDeltaPrcnt->setEnabled(on  && _showDelta->isChecked());
  _showBudgetPrcnt->setEnabled(on && _showBudget->isChecked());
  _showDiffPrcnt->setEnabled(on   && _showDiff->isChecked());
  _showCustomPrcnt->setEnabled(on && _showCustom->isChecked());
}

void financialLayoutGroup::sToggleShowPrcnt()
{
  if (_rpttype == cIncome)
  {
    if (_showPrcnt->isChecked())
    {
      _showBudgetPrcnt->setChecked(TRUE);
      _showDiffPrcnt->setChecked(TRUE);
    }
    else
    {
      _showBudgetPrcnt->setChecked(FALSE);
      _showDiffPrcnt->setChecked(FALSE);
    }
  }
  else if (_rpttype == cBalance)
  {
    if (_showPrcnt->isChecked())
    {
      _showBudgetPrcnt->setChecked(TRUE);
      _showEndPrcnt->setChecked(TRUE);
    }
    else
    {
      _showBudgetPrcnt->setChecked(FALSE);
      _showEndPrcnt->setChecked(FALSE);
    }
  }
  else if (_rpttype == cCash)
  {
    if (_showPrcnt->isChecked())
    {
      _showDeltaPrcnt->setChecked(TRUE);
      _showBudgetPrcnt->setChecked(TRUE);
      _showDiffPrcnt->setChecked(TRUE);
    }
    else
    {
      _showDeltaPrcnt->setChecked(FALSE);
      _showBudgetPrcnt->setChecked(FALSE);
      _showDiffPrcnt->setChecked(FALSE);
    }
  }
}

void financialLayoutGroup::sFillGroupList()
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

