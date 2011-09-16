/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "prospect.h"

#include <QAction>
#include <QCloseEvent>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>

#include "printQuote.h"
#include "salesOrder.h"
#include "storedProcErrorLookup.h"

prospect::prospect(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_deleteQuote,SIGNAL(clicked()),	this,	SLOT(sDeleteQuote()));
  connect(_editQuote,	SIGNAL(clicked()),	this,	SLOT(sEditQuote()));
  connect(_newQuote,	SIGNAL(clicked()),	this,	SLOT(sNewQuote()));
  connect(_number,	SIGNAL(lostFocus()),	this,	SLOT(sCheckNumber()));
  connect(_printQuote,SIGNAL(clicked()),	this,	SLOT(sPrintQuote()));
  connect(_quotes,	SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)),	this,	SLOT(sPopulateQuotesMenu(QMenu*)));
  connect(_save,	SIGNAL(clicked()),	this,	SLOT(sSave()));
  connect(_viewQuote,	SIGNAL(clicked()),	this,	SLOT(sViewQuote()));
  connect(omfgThis,	SIGNAL(quotesUpdated(int, bool)), this, SLOT(sFillQuotesList()));

  if (_privileges->check("MaintainProspectMasters"))
    connect(_quotes, SIGNAL(itemSelected(int)), _editQuote, SLOT(animateClick()));
  else
    connect(_quotes, SIGNAL(itemSelected(int)), _viewQuote, SLOT(animateClick()));

  _prospectid = -1;
  _crmacct->setId(-1);

  _taxzone->setAllowNull(true);
  _taxzone->setType(XComboBox::TaxZones);

  _quotes->addColumn(tr("Quote #"),          _orderColumn, Qt::AlignLeft, true, "quhead_number" );
  _quotes->addColumn(tr("Quote Date"),       _dateColumn,  Qt::AlignLeft, true, "quhead_quotedate" );

  _NumberGen = -1;
}

prospect::~prospect()
{
  // no need to delete child widgets, Qt does it all for us
}

void prospect::languageChange()
{
  retranslateUi(this);
}

enum SetResponse prospect::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("crmacct_name", &valid);
  if (valid)
    _name->setText(param.toString());

  param = pParams.value("crmacct_number", &valid);
  if (valid)
    _number->setText(param.toString());

  param = pParams.value("crmacct_id", &valid);
  if (valid)
  {
    _crmacct->setId(param.toInt());
  }

  param = pParams.value("prospect_id", &valid);
  if (valid)
  {
    _prospectid = param.toInt();
    populate();
    emit newId(_prospectid);
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      // prospects and customers share an id sequence
      q.exec("SELECT NEXTVAL('cust_cust_id_seq') AS prospect_id;");
      if (q.first())
      {
        _prospectid = q.value("prospect_id").toInt();
        emit newId(_prospectid);
	_number->setFocus();
      }
      else
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return UndefinedError;
      }

      if(((_metrics->value("CRMAccountNumberGeneration") == "A") ||
          (_metrics->value("CRMAccountNumberGeneration") == "O"))
       && _number->text().isEmpty() )
      {
        q.exec("SELECT fetchCRMAccountNumber() AS number;");
        if (q.first())
        {
          _number->setText(q.value("number"));
          _NumberGen = q.value("number").toInt();
        }


      }

    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _number->setEnabled(FALSE);
      _name->setEnabled(FALSE);
      _active->setEnabled(FALSE);
      _contact->setEnabled(FALSE);
      _taxzone->setEnabled(FALSE);
      _notes->setReadOnly(TRUE);
      _newQuote->setEnabled(FALSE);
      _save->hide();

      disconnect(_quotes, SIGNAL(itemSelected(int)), _editQuote, SLOT(animateClick()));
      disconnect(_quotes, SIGNAL(valid(bool)),       _editQuote, SLOT(setEnabled(bool)));
      disconnect(_quotes, SIGNAL(valid(bool)),       _editQuote, SLOT(setEnabled(bool)));
      disconnect(_quotes, SIGNAL(valid(bool)),     _deleteQuote, SLOT(setEnabled(bool)));

      connect(_quotes, SIGNAL(itemSelected(int)), _viewQuote, SLOT(animateClick()));

      _close->setText(tr("&Close"));
      _close->setFocus();
    }
  }

  if(_metrics->value("CRMAccountNumberGeneration") == "A")
    _number->setEnabled(FALSE);
  
  return NoError;
}

void prospect::sSave()
{
  struct {
    bool	condition;
    QString	msg;
    QWidget*	widget;
  } error[] = {
    { _number->text().trimmed().length() == 0,
      tr("You must enter a number for this Prospect before saving"),
      _number
    },

    { true, "", NULL }
  };

  int errIndex;
  for (errIndex = 0; ! error[errIndex].condition; errIndex++)
    ;
  if (! error[errIndex].msg.isEmpty())
  {
    QMessageBox::critical(this, tr("Cannot Save Prospect"),
			  QString("<p>") + error[errIndex].msg);
    error[errIndex].widget->setFocus();
    return;
  }

  if (_number->text().trimmed() != _cachedNumber)
  {
    // in anticipation of converting prospects to customers, disallow overlap of prospect and customer numbers
    q.prepare( "SELECT prospect_name AS name "
	       "FROM prospect "
	       "WHERE (UPPER(prospect_number)=UPPER(:prospect_number)) "
	       "  AND (prospect_id<>:prospect_id) "
	       "UNION "
	       "SELECT cust_name AS name "
	       "FROM cust "
	       "WHERE (UPPER(cust_number)=UPPER(:prospect_number));" );
    q.bindValue(":prospect_number", _number->text().trimmed());
    q.bindValue(":prospect_id", _prospectid);
    q.exec();
    if (q.first())
    {
      QMessageBox::critical( this, tr("Prospect Number Used"),
			     tr("<p>The newly entered Prospect Number cannot "
				"be used as it is currently in use by '%1'. "
				"Please enter a different Prospect Number." )
			     .arg(q.value("name").toString()) );
      _number->setFocus();
      return;
    }
  }

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  if (! q.exec("BEGIN;"))
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_mode == cEdit)
  {
    q.prepare( "UPDATE prospect SET "
               "       prospect_number=:prospect_number,"
               "       prospect_name=:prospect_name,"
               "       prospect_cntct_id=:prospect_cntct_id,"
               "       prospect_comments=:prospect_comments,"
               "       prospect_taxzone_id=:prospect_taxzone_id,"
               "       prospect_salesrep_id=:prospect_salesrep_id,"
               "       prospect_warehous_id=:prospect_warehous_id,"
               "       prospect_active=:prospect_active "
               "WHERE (prospect_id=:prospect_id);" );
  }
  else
  {
    q.prepare( "INSERT INTO prospect "
               "( prospect_id,	      prospect_number,	    prospect_name,"
               "  prospect_cntct_id,  prospect_taxzone_id,  prospect_comments,"
               "  prospect_salesrep_id, prospect_warehous_id, prospect_active) "
               " VALUES "
               "( :prospect_id,	      :prospect_number,	    :prospect_name,"
               "  :prospect_cntct_id, :prospect_taxzone_id, :prospect_comments,"
               "  :prospect_salesrep_id, :prospect_warehous_id, :prospect_active);");
  }

  q.bindValue(":prospect_id",		_prospectid);
  q.bindValue(":prospect_number",	_number->text().trimmed());
  q.bindValue(":prospect_name",		_name->text().trimmed());
  if (_contact->isValid())
    q.bindValue(":prospect_cntct_id",	_contact->id());	// else NULL
  if (_taxzone->isValid())
    q.bindValue(":prospect_taxzone_id",	_taxzone->id());	// else NULL
  if (_salesrep->isValid())
    q.bindValue(":prospect_salesrep_id", _salesrep->id());      // else NULL
  if (_site->isValid())
    q.bindValue(":prospect_warehous_id", _site->id());          // else NULL
  q.bindValue(":prospect_comments",	_notes->toPlainText());
  q.bindValue(":prospect_active",	QVariant(_active->isChecked()));

  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_crmacct->isValid())
  {
    q.prepare("UPDATE crmacct SET crmacct_prospect_id = :prospect_id "
	      "WHERE (crmacct_id=:crmacct_id);");
    q.bindValue(":prospect_id",	_prospectid);
    q.bindValue(":crmacct_id",	_crmacct->id());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
	rollback.exec();
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
    }
  }

  q.exec("COMMIT;");
  _NumberGen = -1;
  omfgThis->sProspectsUpdated();
  emit saved(_prospectid);
  if (_mode == cNew)
  {
    omfgThis->sCrmAccountsUpdated(_crmacct->id());
    emit newId(_prospectid);   // cluster listeners couldn't handle set()'s emit
  }

  close();
}

void prospect::sCheckNumber()
{
  _number->setText(_number->text().trimmed().toUpper());

  if (_mode == cNew)
  {
    if(cNew == _mode && -1 != _NumberGen && _number->text().toInt() != _NumberGen)
    {
      XSqlQuery query;
      query.prepare( "SELECT releaseCRMAccountNumber(:Number);" );
      query.bindValue(":Number", _NumberGen);
      query.exec();
      _NumberGen = -1;
    }

    q.prepare( "SELECT prospect_id "
               "FROM prospect "
               "WHERE (prospect_number=:prospect);" );
    q.bindValue(":prospect", _number->text());
    q.exec();
    if (q.first())
    {
      _prospectid = q.value("prospect_id").toInt();
      _mode = cEdit;
      populate();
      emit newId(_prospectid);
      _name->setFocus();
    }
  }
}

void prospect::sPrintQuote()
{
  ParameterList params;
  params.append("quhead_id", _quotes->id());

  printQuote newdlg(this, "printQuote", true);
  newdlg.set(params);
  newdlg.exec();
}

void prospect::sNewQuote()
{
  ParameterList params;
  params.append("mode", "newQuote");
  params.append("cust_id", _prospectid);

  salesOrder *newdlg = new salesOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void prospect::sEditQuote()
{
  ParameterList params;
  params.append("mode", "editQuote");
  params.append("quhead_id", _quotes->id());
    
  salesOrder *newdlg = new salesOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void prospect::sViewQuote()
{
  ParameterList params;
  params.append("mode", "viewQuote");
  params.append("quhead_id", _quotes->id());
    
  salesOrder *newdlg = new salesOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void prospect::sDeleteQuote()
{
  if ( QMessageBox::warning( this, tr("Delete Selected Quote"),
                             tr("Are you sure that you want to delete the selected Quote?" ),
                             tr("&Yes"), tr("&No"), QString::null, 0, 1 ) == 0)
  {
    q.prepare("SELECT deleteQuote(:quhead_id) AS result;");
    q.bindValue(":quhead_id", _quotes->id());
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
	systemError(this, storedProcErrorLookup("deleteQuote", result),
		    __FILE__, __LINE__);
	return;
      }
      else
	omfgThis->sQuotesUpdated(-1);
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void prospect::sFillQuotesList()
{
  q.prepare("SELECT DISTINCT quhead_id, quhead_number, "
	    "                quhead_quotedate "
	    "FROM quhead "
	    "WHERE (quhead_cust_id=:prospect_id) "
	    "ORDER BY quhead_number;");
  q.bindValue(":prospect_id", _prospectid);
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  _quotes->populate(q);
}

void prospect::sPopulateQuotesMenu(QMenu *menuThis)
{
  menuThis->addAction(tr("Edit..."),   this, SLOT(sEditQuote()));
  menuThis->addAction(tr("View..."),   this, SLOT(sViewQuote()));
  menuThis->addAction(tr("Delete..."), this, SLOT(sDeleteQuote()));
  menuThis->addAction(tr("Print..."),  this, SLOT(sPrintQuote()));
}

void prospect::populate()
{
  XSqlQuery prospect;
  prospect.prepare("SELECT prospect.*, crmacct_id "
                "FROM prospect, crmacct "
                "WHERE ((prospect_id=:prospect_id)"
		"  AND  (prospect_id=crmacct_prospect_id));" );
  prospect.bindValue(":prospect_id", _prospectid);
  prospect.exec();
  if (prospect.first())
  {
    _crmacct->setId(prospect.value("crmacct_id").toInt());

    _number->setText(prospect.value("prospect_number"));
    _cachedNumber = prospect.value("prospect_number").toString();
    _name->setText(prospect.value("prospect_name").toString());
    _contact->setId(prospect.value("prospect_cntct_id").toInt());
    _taxzone->setId(prospect.value("prospect_taxzone_id").toInt());
    _salesrep->setId(prospect.value("prospect_salesrep_id").toInt());
    _site->setId(prospect.value("prospect_warehous_id").toInt());
    _notes->setText(prospect.value("prospect_comments").toString());
    _active->setChecked(prospect.value("prospect_active").toBool());
  }
  else if (prospect.lastError().type() != QSqlError::NoError)
  {
    systemError(this, prospect.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillQuotesList();
  emit populated();
}

void prospect::closeEvent(QCloseEvent *pEvent)
{
  if(cNew == _mode && -1 != _NumberGen)
  {
    XSqlQuery query;
    query.prepare( "SELECT releaseCRMAccountNumber(:Number);" );
    query.bindValue(":Number", _NumberGen);
    query.exec();
    _NumberGen = -1;
  }
  XWidget::closeEvent(pEvent);
}

