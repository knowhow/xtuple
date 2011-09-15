/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "taxCode.h"

#include <QAction>
#include <QCloseEvent>
#include <QDoubleValidator>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include "taxCodeRate.h"

taxCode::taxCode(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_code, SIGNAL(lostFocus()), this, SLOT(sCheck())); 
  connect(_taxClass, SIGNAL(newID(int)), this, SLOT(populateBasis()));
  connect(_taxitems, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew())); 
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit())); 
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_expire, SIGNAL(clicked()), this, SLOT(sExpire())); 
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  
  _taxitems->addColumn(tr("Effective"), _dateColumn,    Qt::AlignLeft,  true, "effective" );
  _taxitems->addColumn(tr("Expires"),   _dateColumn,    Qt::AlignLeft,  true, "expires" );
  _taxitems->addColumn(tr("Percent"),   _prcntColumn,   Qt::AlignRight, true, "taxrate_percent" );
  _taxitems->addColumn(tr("Amount"),    _moneyColumn,   Qt::AlignRight, true, "taxrate_amount" );
  _taxitems->addColumn(tr("Currency"),  -1,             Qt::AlignLeft,  true, "curr_name" );
  sFillList(); 

  _account->setType(GLCluster::cRevenue | GLCluster::cLiability | GLCluster::cExpense);
}

void taxCode::populateBasis()
{
  _basis->clear();

  QString sql(" SELECT tax_id, tax_code || '-' || tax_descrip, tax_code" 
   			  " FROM tax"
			  " WHERE ( tax_taxclass_id = "
			  " <? value(\"taxclass_id\") ?>);"); 

  MetaSQLQuery mql(sql);
  ParameterList params;
  params.append("taxclass_id", _taxClass->id()); 
  XSqlQuery taxbasis = mql.toQuery(params); 
  _basis->populate(taxbasis);
} 

void taxCode::sPopulateMenu(QMenu *menuThis)
{
  menuThis->addAction(tr("View"), this, SLOT(sView()));
  
  if ((_mode == cNew) || (_mode == cEdit))
  {
    menuThis->addAction(tr("Edit"), this, SLOT(sEdit()));
    menuThis->addAction(tr("Expire"), this, SLOT(sExpire()));
    menuThis->addAction(tr("Delete"), this, SLOT(sDelete()));
  }
}

taxCode::~taxCode()
{
  // no need to delete child widgets, Qt does it all for us
}

void taxCode::languageChange()
{
  retranslateUi(this);
}

void taxCode::sNew() 
{
  
  ParameterList params;
  params.append("mode", "new");
  params.append("tax_id", _taxid);
  taxCodeRate newdlg(this, "", TRUE); 
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void taxCode::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("tax_id", _taxid); 
  params.append("taxrate_id", _taxitems->id());

  taxCodeRate newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void taxCode::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("taxrate_id", _taxitems->id());

  taxCodeRate newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void taxCode::sDelete()
{
  if (QMessageBox::question(this, tr("Delete Tax Code Rate?"),
			      tr("<p>Are you sure you want to delete this "
				 "Tax Code Rate ?"),
				  QMessageBox::Yes,
				  QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    q.prepare( " DELETE FROM taxrate "
               " WHERE (taxrate_id=:taxrate_id);");
                
    q.bindValue(":taxrate_id", _taxitems->id());
    q.exec();

    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    sFillList();
  }
}


void taxCode::sFillList()
{
  _taxitems->clear();


  MetaSQLQuery mql( " SELECT taxrate.*, curr_name, "
                    "        CASE WHEN (taxrate_effective = startOfTime()) THEN NULL "
                    "             ELSE taxrate_effective END AS effective, "
                    "        CASE WHEN (taxrate_expires = endOfTime()) THEN NULL "
                    "             ELSE taxrate_expires END AS expires, "
                    "        <? value(\"always\") ?> AS effective_xtnullrole, "
                    "        <? value(\"never\") ?>  AS expires_xtnullrole, "
                    "       CASE WHEN (taxrate_expires < CURRENT_DATE) THEN 'error'"
                    "	         WHEN (taxrate_effective >= CURRENT_DATE) THEN 'emphasis'"
                    "       END AS qtforegroundrole, "
                    "      'percent' AS taxrate_percent_xtnumericrole,"
                    "      'currency' AS taxrate_amount_xtnumericrole "
                    " FROM taxrate"
                    "      LEFT OUTER JOIN curr_symbol ON (taxrate_curr_id = curr_id) "
                    " WHERE taxrate_tax_id = <? value(\"tax_id\") ?> "
                    " ORDER BY taxrate_id, taxrate_effective, taxrate_expires, "
                    "          taxrate_percent, taxrate_amount; " );

  ParameterList params;
  setParams(params);
  q = mql.toQuery(params);
   
  _taxitems->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}


void taxCode::sExpire()
{
  q.prepare( "select taxrate_id "
             "FROM taxrate  "
             "WHERE (taxrate_id=:taxitems_id AND (taxrate_expires <= CURRENT_DATE "
			 " OR taxrate_effective > CURRENT_DATE));" );
  q.bindValue(":taxitems_id", _taxitems->id());
  q.exec();
  if(q.first())  
  {
    QMessageBox::information(this, tr("Expired Tax Rate"),
    tr("Cannot expire this Tax Code. It is already expired or is a Future Rate.") );
 	return;
  }
  else
  {
    q.prepare( "UPDATE taxrate "
               "SET taxrate_expires=CURRENT_DATE "
               "WHERE (taxrate_id=:taxitems_id);" );
    q.bindValue(":taxitems_id", _taxitems->id());
    q.exec();
    sFillList();
  }
}

enum SetResponse taxCode::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("tax_id", &valid); 
  if (valid) 
  {
    _taxid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    connect(_taxitems, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
    if ( (param.toString() == "new") || (param.toString() == "edit") )
    {
      connect(_taxitems, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_taxitems, SIGNAL(valid(bool)), _expire, SLOT(setEnabled(bool)));
      connect(_taxitems, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      connect(_taxitems, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
    }
  
    if (param.toString() == "new") 
    {
      _mode = cNew;
      _code->setFocus();
	  initTaxCode(); 
	  populateBasis();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _description->setFocus();

    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _code->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _account->setReadOnly(TRUE); 
      _taxClass->setEnabled(FALSE);
      _taxauth->setEnabled(FALSE);
      _basis->setEnabled(FALSE);
      _new->setEnabled(FALSE);
      _edit->setEnabled(FALSE);
      _expire->setEnabled(FALSE);
      _delete->setEnabled(FALSE);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
      _buttonBox->setFocus();
    }
  }  
  return NoError;
}

void taxCode::sSave()
{
  if(_code->text().trimmed().isEmpty())
  {
    QMessageBox::warning( this, tr("No Tax Code"),
                          tr("You must specify a Code for this Tax.") );
    _code->setFocus();
    return;
  }
  if (!_account->isValid())
   {
     QMessageBox::warning( this, tr("Select G/L Accout"),
                            tr("You must select a G/L Account for this Tax.") );
     _account->setFocus();
      return;
   }
  q.prepare("SELECT tax_id"
            "  FROM tax"
            " WHERE((tax_id!= :tax_id)"
		    " AND (tax_code=:tax_code));");
  q.bindValue(":tax_id", _taxid); 
  q.bindValue(":tax_code", _code->text().trimmed());
  q.exec();
  if(q.first())
  {
    QMessageBox::critical(this, tr("Duplicate Tax Code"),
      tr("A Tax Code already exists for the parameters specified.") );
    _code->setFocus();
    return;
  }

  q.prepare( "UPDATE tax "
             "SET tax_code=:tax_code, tax_descrip=:tax_descrip,"
             "    tax_sales_accnt_id=:tax_sales_accnt_id,"
             "    tax_taxclass_id=:tax_taxclass_id,"
             "    tax_taxauth_id=:tax_taxauth_id,"
             "    tax_basis_tax_id=:tax_basis_tax_id "
             "WHERE (tax_id=:tax_id);" );
  
  q.bindValue(":tax_code", _code->text().trimmed());
  q.bindValue(":tax_descrip", _description->text());
  if(_account->isValid())
     q.bindValue(":tax_sales_accnt_id", _account->id());
  if(_taxauth->isValid())
    q.bindValue(":tax_taxauth_id", _taxauth->id());
  if(_taxClass->isValid())
    q.bindValue(":tax_taxclass_id", _taxClass->id());
  if(_basis->isValid())
    q.bindValue(":tax_basis_tax_id", _basis->id());
  q.bindValue(":tax_id", _taxid); 
  q.exec();
  done (_taxid);
}

void taxCode::sCheck()
{
  _code->setText(_code->text().trimmed());
  if ((_mode == cNew) && (_code->text().length()))
  {
    q.prepare( "SELECT tax_id "
               "FROM tax "
               "WHERE (UPPER(tax_code)=UPPER(:tax_code));" );
    q.bindValue(":tax_code", _code->text());
    q.exec();
    if (q.first())
    {
      // delete placeholder
      XSqlQuery ph;
      ph.prepare( " DELETE FROM taxrate "
                  " WHERE (taxrate_tax_id=:tax_id);"
                  " DELETE FROM tax "
                  " WHERE (tax_id=:tax_id);");
                
      ph.bindValue(":tax_id", _taxid);
      ph.exec();
      if (ph.lastError().type() != QSqlError::NoError)
        systemError(this, ph.lastError().databaseText(), __FILE__, __LINE__);
        
      _taxid = q.value("tax_id").toInt();
      _mode = cEdit;
      populate();

      _code->setEnabled(FALSE);
    }
  }
}

void taxCode::populate() 
{
 
  q.prepare( "SELECT tax_code, tax_descrip,"
             "       tax_sales_accnt_id,"
             "       tax_taxclass_id,"
             "       tax_taxauth_id,"
             "       tax_basis_tax_id "
             "FROM tax "
             "WHERE (tax_id=:tax_id);" );
  q.bindValue(":tax_id", _taxid);
  q.exec();
  if (q.first())
  {
    _code->setText(q.value("tax_code").toString());
    _description->setText(q.value("tax_descrip").toString());
    _account->setId(q.value("tax_sales_accnt_id").toInt());
	_taxClass->setId(q.value("tax_taxclass_id").toInt());
	_taxauth->setId(q.value("tax_taxauth_id").toInt()); 
    _basis->setId(q.value("tax_basis_tax_id").toInt());
  }
  
  sFillList();
}

void taxCode::initTaxCode()
{
  q.exec("SELECT NEXTVAL('tax_tax_id_seq') AS tax_id;");
  if (q.first())
    _taxid = q.value("tax_id").toInt();
  else
  {
    systemError(this, tr("A System Error occurred at %1::%2.")
                  .arg(__FILE__)
                  .arg(__LINE__) );
    return;
  }

  q.prepare( "INSERT INTO tax( tax_id) VALUES ( :tax_id );");
  q.bindValue(":tax_id", _taxid);
  q.exec();
}

void taxCode::closeEvent(QCloseEvent *pEvent)
{
  if ((_mode == cNew) && (_taxid != -1))
  {
    if (_taxitems->topLevelItemCount() > 0 &&
        QMessageBox::question(this, tr("Delete Tax Code?"),
			      tr("<p>Are you sure you want to delete this "
				 "Tax Code and all of its associated "
				 "Tax Rates?"),
				  QMessageBox::Yes,
				  QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
    {
      pEvent->ignore();
      return;
    }

    q.prepare( " DELETE FROM taxrate "
               " WHERE (taxrate_tax_id=:tax_id);"
               " DELETE FROM tax "
               " WHERE (tax_id=:tax_id);");
                
    q.bindValue(":tax_id", _taxid);
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
  }

  XDialog::closeEvent(pEvent);
}

bool taxCode::setParams(ParameterList &pParams)
{
  pParams.append("tax_id",      _taxid);
  pParams.append("always",      tr("Always"));
  pParams.append("never",       tr("Never"));

  return true;
}
