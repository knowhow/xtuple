/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "financialLayoutItem.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>

#define cIncome		0
#define cBalance	1
#define cCash		2
#define cAdHoc		3

/*
 *  Constructs a financialLayoutItem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
financialLayoutItem::financialLayoutItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_showBeginning, SIGNAL(toggled(bool)), _showBeginningPrcnt, SLOT(setEnabled(bool)));
  connect(_showBudget, SIGNAL(toggled(bool)), _showBudgetPrcnt, SLOT(setEnabled(bool)));
  connect(_showDB, SIGNAL(toggled(bool)), _showDBPrcnt, SLOT(setEnabled(bool)));
  connect(_showEnding, SIGNAL(toggled(bool)), _showEndingPrcnt, SLOT(setEnabled(bool)));
  connect(_showDiff, SIGNAL(toggled(bool)), _showDiffPrcnt, SLOT(setEnabled(bool)));
  connect(_showPrcnt, SIGNAL(toggled(bool)), this, SLOT(sToggleShowPrcnt()));
  connect(_selectAccount, SIGNAL(toggled(bool)), this, SLOT(sToggleSegment()));
  connect(_selectSegment, SIGNAL(toggled(bool)), this, SLOT(sToggleAccount()));
  connect(_type, SIGNAL(activated(int)), this, SLOT(populateSubTypes()));

  _flheadid = -1;
  _flitemid = -1;

  _account->setShowExternal(true);
  _company->setType(XComboBox::Companies);
  _company->append(-1,tr("All"),"All");
  _company->setId(-1);
  _profit->setType(XComboBox::ProfitCenters);
  _profit->append(-1,tr("All"),"All");
  _profit->setId(-1);
  _type->setCurrentIndex(5);
  _sub->setType(XComboBox::Subaccounts);
  _sub->append(-1,tr("All"),"All");
  _sub->setId(-1);

  q.prepare( "SELECT DISTINCT accnt_id, accnt_number, accnt_number "
                  "FROM ONLY accnt "
                  "ORDER BY accnt_number;" );
  q.exec();
  _number->populate(q);
  _number->append(-1,tr("All"),"All");
  _number->setId(-1);

  _subType->setAllowNull(FALSE);
  populateSubTypes();

  if (_metrics->value("GLCompanySize").toInt() == 0)
  {
    _company->hide();
    _sep1Lit->hide();
  }

  if (_metrics->value("GLProfitSize").toInt() == 0)
  {
    _profit->hide();
    _sep2Lit->hide();
  }

  if (_metrics->value("GLSubaccountSize").toInt() == 0)
  {
    _sub->hide();
    _sep3Lit->hide();
  }
}

/*
 *  Destroys the object and frees any allocated resources
 */
financialLayoutItem::~financialLayoutItem()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void financialLayoutItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse financialLayoutItem::set(const ParameterList &pParams)
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

  param = pParams.value("flitem_id", &valid);
  if (valid)
  {
    _flitemid = param.toInt();
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
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
      _buttonBox->setFocus();
      _selectAccount->setEnabled(FALSE);
      _selectSegment->setEnabled(FALSE);
      _operationGroup->setEnabled(FALSE);
      _showColumns->setEnabled(FALSE);
      _showPrcnt->setEnabled(FALSE);
      _group->setEnabled(FALSE);
      _showCustom->setEnabled(FALSE);
      _buttonBox->setFocus();
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
			_showColumns->setHidden(TRUE);
			_showCustom->setHidden(TRUE);
		}
		
        if (param.toString() == "income")
        {
        	_rpttype = cIncome;	
   			_showBeginning->setChecked(FALSE);
			_showEnding->setChecked(FALSE);
			_showDB->setChecked(FALSE);
			_showBudget->setChecked(TRUE);
			_showDiff->setChecked(TRUE);
			_showCustom->setChecked(FALSE);
        }
        else if (param.toString() == "balance")
        {
            _rpttype = cBalance;
   			_showBeginning->setChecked(FALSE);
			_showEnding->setChecked(TRUE);
			_showDB->setChecked(FALSE);
			_showBudget->setChecked(TRUE);
			_showDiff->setChecked(TRUE);
			_showCustom->setChecked(FALSE);
        }
        else if (param.toString() == "cash")
        {
			_rpttype = cCash;
   			_showBeginning->setChecked(FALSE);
			_showEnding->setChecked(FALSE);
			_showDB->setChecked(TRUE);
			_showBudget->setChecked(TRUE);
			_showDiff->setChecked(TRUE);
			_showCustom->setChecked(FALSE);
		}
    }

  return NoError;
}

void financialLayoutItem::sCheck()
{
}

void financialLayoutItem::sSave()
{ 
  QString sql;

  if (_selectAccount->isChecked())
  {
    q.prepare( "SELECT count(*) AS result"
             "  FROM flitem"
             " WHERE ((flitem_flhead_id=:flhead_id)"
             "   AND  (flitem_id != :flitem_id)"
             "   AND  (flitem_accnt_id=:accnt_id) ); ");
    q.bindValue(":flhead_id", _flheadid);
    q.bindValue(":flitem_id", _flitemid);
    q.bindValue(":accnt_id", _account->id());
    q.exec();
    if(q.first() && (q.value("result").toInt() > 0) )
    {
      if ( QMessageBox::warning( this, tr("Duplicate Account Number"),
           tr("The selected Account number is already being used in this Financial Report.\n"
              "Would you like to continue anyway?"),
           QMessageBox::Yes | QMessageBox::Default,
           QMessageBox::No  | QMessageBox::Escape,
           Qt::NoButton ) != QMessageBox::Yes)
        return;
 
    }
  }
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
    q.exec("SELECT NEXTVAL('flitem_flitem_id_seq') AS flitem_id;");
    if (q.first())
      _flitemid = q.value("flitem_id").toInt();
    
    sql = ( "INSERT INTO flitem "
               "( flitem_id, flitem_flhead_id, flitem_flgrp_id, flitem_order,"
               "  flitem_accnt_id, flitem_showstart, flitem_showend, flitem_showdelta, flitem_showbudget, flitem_showdiff, flitem_showcustom,"
               "  flitem_subtract, flitem_showstartprcnt, flitem_showendprcnt, flitem_showdeltaprcnt, flitem_showcustomprcnt,"
               "  flitem_showbudgetprcnt, flitem_showdiffprcnt, flitem_prcnt_flgrp_id, flitem_custom_source, "
               "  flitem_company, flitem_profit, flitem_number, flitem_sub, flitem_type, flitem_subaccnttype_code) "
               "VALUES "
               "( :flitem_id, :flitem_flhead_id, :flitem_flgrp_id, :flitem_order,"
               "  :flitem_accnt_id, :flitem_showstart, :flitem_showend, :flitem_showdelta, :flitem_showbudget, :flitem_showdiff, :flitem_showcustom,"
               "  :flitem_subtract, :flitem_showstartprcnt, :flitem_showendprcnt, :flitem_showdeltaprcnt, :flitem_showcustomprcnt,"
               "  :flitem_showbudgetprcnt, :flitem_showdiffprcnt, :flitem_prcnt_flgrp_id, :flitem_custom_source,"
               "  :flitem_company, :flitem_profit, :flitem_number, :flitem_sub, :flitem_type, ");
    if (_selectAccount->isChecked())
      sql += "	   '' );";
    else
      sql += "    (COALESCE((SELECT subaccnttype_code FROM subaccnttype WHERE subaccnttype_id=:subaccnttype_id),'All')));" ;
  }
  else if (_mode == cEdit)
  {
    sql =  "UPDATE flitem "
               "SET flitem_accnt_id=:flitem_accnt_id, flitem_showstart=:flitem_showstart,"
               "    flitem_showend=:flitem_showend, flitem_showdelta=:flitem_showdelta,"
               "    flitem_showbudget=:flitem_showbudget, flitem_subtract=:flitem_subtract,"
               "    flitem_showdiff=:flitem_showdiff, flitem_showcustom=:flitem_showcustom,"
               "    flitem_showstartprcnt=:flitem_showstartprcnt, flitem_showendprcnt=:flitem_showendprcnt,"
               "    flitem_showdeltaprcnt=:flitem_showdeltaprcnt, flitem_showbudgetprcnt=:flitem_showbudgetprcnt,"
               "    flitem_showdiffprcnt=:flitem_showdiffprcnt, flitem_showcustomprcnt=:flitem_showcustomprcnt,"
               "    flitem_prcnt_flgrp_id=:flitem_prcnt_flgrp_id, flitem_custom_source=:flitem_custom_source, "
	       "    flitem_company=:flitem_company, flitem_profit=:flitem_profit, "
  	       "    flitem_number=:flitem_number, flitem_sub=:flitem_sub, "
	       "    flitem_type=:flitem_type, ";
    if (_selectAccount->isChecked())
      sql += "	    flitem_subaccnttype_code='' ";
    else
      sql += "    flitem_subaccnttype_code=(COALESCE((SELECT subaccnttype_code FROM subaccnttype WHERE subaccnttype_id=:subaccnttype_id),'All'))" ;

    sql +=   " WHERE (flitem_id=:flitem_id);";
  }

  q.prepare(sql);
   
  q.bindValue(":flitem_flhead_id", _flheadid);
  q.bindValue(":flitem_flgrp_id", _flgrpid);
  q.bindValue(":flitem_order", order);
  q.bindValue(":flitem_accnt_id", _account->id());
  q.bindValue(":flitem_showstart", QVariant(_showBeginning->isChecked()));
  q.bindValue(":flitem_showend", QVariant(_showEnding->isChecked()));
  q.bindValue(":flitem_showdelta", QVariant(_showDB->isChecked()));
  q.bindValue(":flitem_showbudget", QVariant(_showBudget->isChecked()));
  q.bindValue(":flitem_showdiff", QVariant(_showDiff->isChecked()));
  q.bindValue(":flitem_showcustom", QVariant(_showCustom->isChecked()));
  q.bindValue(":flitem_subtract", QVariant(_subtract->isChecked()));
  q.bindValue(":flitem_showstartprcnt", QVariant(_showBeginning->isChecked() && _showBeginningPrcnt->isChecked()));
  q.bindValue(":flitem_showendprcnt", QVariant(_showEnding->isChecked() && _showEndingPrcnt->isChecked()));
  q.bindValue(":flitem_showdeltaprcnt", QVariant(_showDB->isChecked() && _showDBPrcnt->isChecked()));
  q.bindValue(":flitem_showbudgetprcnt", QVariant(_showBudget->isChecked() && _showBudgetPrcnt->isChecked()));
  q.bindValue(":flitem_showdiffprcnt", QVariant(_showDiff->isChecked() && _showDiffPrcnt->isChecked()));
  q.bindValue(":flitem_showcustomprcnt", QVariant(_showCustom->isChecked() && _showCustomPrcnt->isChecked()));
  q.bindValue(":flitem_id", _flitemid);
  q.bindValue(":flitem_prcnt_flgrp_id", _group->id());

  if(_customUseBeginning->isChecked())
    q.bindValue(":flitem_custom_source", "S");
  else if(_customUseEnding->isChecked())
    q.bindValue(":flitem_custom_source", "E");
  else if(_customUseDebits->isChecked())
    q.bindValue(":flitem_custom_source", "D");
  else if(_customUseCredits->isChecked())
    q.bindValue(":flitem_custom_source", "C");
  else if(_customUseBudget->isChecked())
    q.bindValue(":flitem_custom_source", "B");
  else if(_customUseDiff->isChecked())
    q.bindValue(":flitem_custom_source", "F");

  if ( _selectSegment->isChecked() )
  {
    q.bindValue(":flitem_accnt_id", -1);
    q.bindValue(":flitem_company", _company->code());
    q.bindValue(":flitem_profit", _profit->code());
    q.bindValue(":flitem_number", _number->code());
    q.bindValue(":flitem_sub", _sub->code());
    q.bindValue(":subaccnttype_id", _subType->id());

    if (_type->currentIndex() == 0)
      q.bindValue(":flitem_type", "A");
    else if (_type->currentIndex() == 1)
      q.bindValue(":flitem_type", "L");
    else if (_type->currentIndex() == 2)
      q.bindValue(":flitem_type", "E");
    else if (_type->currentIndex() == 3)
      q.bindValue(":flitem_type", "R");
    else if (_type->currentIndex() == 4)
      q.bindValue(":flitem_type", "Q");
    else
      q.bindValue(":flitem_type", "");
  }
  else
  {
    q.bindValue(":flitem_accnt_id", _account->id());
    q.bindValue(":flitem_company", "");
    q.bindValue(":flitem_profit", "");
    q.bindValue(":flitem_number", "");
    q.bindValue(":flitem_sub", "");
    q.bindValue(":flitem_subaccnttype_code", "");
    q.bindValue(":flitem_type", "");
  }
  
  q.exec();

  done(_flitemid);
}

void financialLayoutItem::populate()
{
  q.prepare( "SELECT * "
             "FROM flitem "
	     "LEFT OUTER JOIN subaccnttype ON flitem_subaccnttype_code=subaccnttype_code "
             "WHERE (flitem_id=:flitem_id);" );
  q.bindValue(":flitem_id", _flitemid);
  q.exec();
  if (q.first())
  {
    if ( q.value("flitem_accnt_id").toInt() == -1 )
    {
      _selectSegment->setChecked(TRUE);

      if (_metrics->value("GLCompanySize").toInt())
        _company->setCode(q.value("flitem_company").toString());
      if (_metrics->value("GLProfitSize").toInt())
        _profit->setCode(q.value("flitem_profit").toString());
      _number->setCode(q.value("flitem_number").toString());
      if (_metrics->value("GLSubaccountSize").toInt())
        _sub->setCode(q.value("flitem_sub").toString());

      if (q.value("flitem_type").toString() == "A")
        _type->setCurrentIndex(0);
      else if (q.value("flitem_type").toString() == "L")
        _type->setCurrentIndex(1);
      else if (q.value("flitem_type").toString() == "E")
        _type->setCurrentIndex(2);
      else if (q.value("flitem_type").toString() == "R")
        _type->setCurrentIndex(3);
      else if (q.value("flitem_type").toString() == "Q")
        _type->setCurrentIndex(4);
      else
	  _type->setCurrentIndex(5);

      _subType->setId(q.value("subaccnttype_id").toInt());
    }
    else
    {
      _selectAccount->setChecked(TRUE);
      _account->setId(q.value("flitem_accnt_id").toInt());
    }
    _showBeginning->setChecked(q.value("flitem_showstart").toBool());
    _showEnding->setChecked(q.value("flitem_showend").toBool());
    _showDB->setChecked(q.value("flitem_showdelta").toBool());
    _showBudget->setChecked(q.value("flitem_showbudget").toBool());
    _showDiff->setChecked(q.value("flitem_showdiff").toBool());
    _showCustom->setChecked(q.value("flitem_showcustom").toBool());
    _showBeginningPrcnt->setChecked(q.value("flitem_showstartprcnt").toBool());
    _showEndingPrcnt->setChecked(q.value("flitem_showendprcnt").toBool());
    _showDBPrcnt->setChecked(q.value("flitem_showdeltaprcnt").toBool());
    _showBudgetPrcnt->setChecked(q.value("flitem_showbudgetprcnt").toBool());
    _showDiffPrcnt->setChecked(q.value("flitem_showdiffprcnt").toBool());
    _showCustomPrcnt->setChecked(q.value("flitem_showcustomprcnt").toBool());
    
    if ((_rpttype != cAdHoc) & ((_showDiffPrcnt->isChecked()) || (_showEndingPrcnt->isChecked())))
		_showPrcnt->setChecked(TRUE);

    QString src = q.value("flitem_custom_source").toString();
    if("S" == src)
      _customUseBeginning->setChecked(TRUE);
    else if("E" == src)
      _customUseEnding->setChecked(TRUE);
    else if("D" == src)
      _customUseDebits->setChecked(TRUE);
    else if("C" == src)
      _customUseCredits->setChecked(TRUE);
    else if("B" == src)
      _customUseBudget->setChecked(TRUE);
    else if("F" == src)
      _customUseDiff->setChecked(TRUE);

    if(q.value("flitem_subtract").toBool())
      _subtract->setChecked(TRUE);
    else
      _add->setChecked(TRUE);

    _flheadid = q.value("flitem_flhead_id").toInt();

    int grpid = q.value("flitem_prcnt_flgrp_id").toInt();
    sFillGroupList();
    _group->setId(grpid);
    
    sToggleShowPrcnt();
  }
}

void financialLayoutItem::sFillGroupList()
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

void financialLayoutItem::sToggleShowPrcnt()
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
			_showEndingPrcnt->setChecked(TRUE);
		}
		else
		{
			_showBudgetPrcnt->setChecked(FALSE);
			_showEndingPrcnt->setChecked(FALSE);
		}
	}
	else if (_rpttype == cCash)
	{
		if (_showPrcnt->isChecked())
		{
			_showDBPrcnt->setChecked(TRUE);
			_showBudgetPrcnt->setChecked(TRUE);
			_showDiffPrcnt->setChecked(TRUE);
		}
		else
		{
			_showDBPrcnt->setChecked(FALSE);
			_showBudgetPrcnt->setChecked(FALSE);
			_showDiffPrcnt->setChecked(FALSE);
		}
	}
}

void financialLayoutItem::sToggleSegment()
{
    if (_selectAccount->isChecked())
      _selectSegment->setChecked(FALSE);
    else
      _selectSegment->setChecked(TRUE);
}

void financialLayoutItem::sToggleAccount()
{
    if (_selectSegment->isChecked())
      _selectAccount->setChecked(FALSE);
    else
      _selectAccount->setChecked(TRUE);
}

void financialLayoutItem::populateSubTypes()
{
  QString query;
  XSqlQuery sub;
  query = ("SELECT subaccnttype_id, (subaccnttype_code||'-'||subaccnttype_descrip) "
              "FROM subaccnttype ");
              
  if (_type->currentIndex() != 5)
       query += "WHERE (subaccnttype_accnt_type=:subaccnttype_accnt_type) ";
       
  query +=  "ORDER BY subaccnttype_code; ";
  sub.prepare(query);
  if  (_type->currentIndex() != 5)
  {
	if (_type->currentIndex() == 0)
		sub.bindValue(":subaccnttype_accnt_type", "A");
	else if (_type->currentIndex() == 1)
		sub.bindValue(":subaccnttype_accnt_type", "L");
	else if (_type->currentIndex() == 2)
		sub.bindValue(":subaccnttype_accnt_type", "E");
	else if (_type->currentIndex() == 3)
		sub.bindValue(":subaccnttype_accnt_type", "R");
	else if (_type->currentIndex() == 4)
		sub.bindValue(":subaccnttype_accnt_type", "Q");
  }
  sub.exec();
  
  _subType->populate(sub);
  _subType->append(-1,tr("All"));
  _subType->setText(tr("All"));
}
