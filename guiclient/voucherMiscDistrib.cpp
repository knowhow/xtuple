/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "voucherMiscDistrib.h"

#include <QVariant>
#include <QMessageBox>
#include <QValidator>
/*
 *  Constructs a voucherMiscDistrib as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
voucherMiscDistrib::voucherMiscDistrib(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_taxCode, SIGNAL(newID(int)), this, SLOT(sCheck()));
  
  _account->setType(GLCluster::cRevenue | GLCluster::cExpense |
                    GLCluster::cAsset | GLCluster::cLiability);

  adjustSize();
}

/*
 *  Destroys the object and frees any allocated resources
 */
voucherMiscDistrib::~voucherMiscDistrib()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void voucherMiscDistrib::languageChange()
{
  retranslateUi(this);
}

enum SetResponse voucherMiscDistrib::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;


  param = pParams.value("vohead_id", &valid);
  if (valid)
    _voheadid = param.toInt();
 
  _miscvoucher = pParams.inList("voucher_type");
 
  param = pParams.value("curr_id", &valid);
  if (valid)
    _amount->setId(param.toInt());

  param = pParams.value("curr_effective", &valid);
  if (valid)
    _amount->setEffective(param.toDate());

  param = pParams.value("vodist_id", &valid);
  if (valid)
  {
    _vodistid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      param = pParams.value("vend_id", &valid);
      if (valid)
        sPopulateVendorInfo(param.toInt());
      param = pParams.value("amount", &valid);
      if (valid)
        _amount->setLocalValue(param.toDouble());
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _amount->setFocus();
      sCheck();
    }
  }

  return NoError;
}

void voucherMiscDistrib::populate()
{
  q.prepare( "SELECT * "
             "FROM vodist "
             "WHERE (vodist_id=:vodist_id);" ) ;
  q.bindValue(":vodist_id", _vodistid);
  q.exec();
  if (q.first())
  {
    _account->setId(q.value("vodist_accnt_id").toInt());
    _amount->setLocalValue(q.value("vodist_amount").toDouble());
    _discountable->setChecked(q.value("vodist_discountable").toBool());
    _notes->setText(q.value("vodist_notes").toString());
    if(q.value("vodist_expcat_id").toInt() != -1)
    {
      _expcatSelected->setChecked(TRUE);
      _expcat->setId(q.value("vodist_expcat_id").toInt());
    }
    if(q.value("vodist_tax_id").toInt() != -1)
    {
	   _taxSelected->setChecked(TRUE);
	   _taxCode->setId(q.value("vodist_tax_id").toInt());
    }
  }
}

void voucherMiscDistrib::sSave()
{
  if (_accountSelected->isChecked() && !_account->isValid())
  {
    QMessageBox::warning( this, tr("Select Account"),
                          tr("You must select an Account to post this Miscellaneous Distribution to.") );
    _account->setFocus();
    return;
  }

  if (_expcatSelected->isChecked() && !_expcat->isValid())
  {
    QMessageBox::warning( this, tr("Select Expense Category"),
                          tr("You must select an Expense Category to post this Miscellaneous Distribution to.") );
    _account->setFocus();
    return;
  }

  if (_taxSelected->isChecked() && !_taxCode->isValid())
  {
    QMessageBox::warning( this, tr("Select Tax Code"),
                          tr("You must select a Tax Code to post this Miscellaneous Distribution to.") );
    _account->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('vodist_vodist_id_seq') AS _vodistid;");
    if (q.first())
      _vodistid = q.value("_vodistid").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }

    q.prepare( "INSERT INTO vodist "
               "( vodist_id, vodist_vohead_id, vodist_poitem_id,"
               "  vodist_costelem_id, vodist_accnt_id, vodist_amount, vodist_discountable,"
               "  vodist_expcat_id, vodist_tax_id, vodist_notes ) "
               "VALUES "
               "( :vodist_id, :vodist_vohead_id, -1,"
               "  -1, :vodist_accnt_id, :vodist_amount, :vodist_discountable,"
               "  :vodist_expcat_id, :vodist_tax_id, :vodist_notes );" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE vodist "
               "SET vodist_accnt_id=:vodist_accnt_id,"
               "    vodist_amount=:vodist_amount,"
               "    vodist_discountable=:vodist_discountable,"
               "    vodist_expcat_id=:vodist_expcat_id, "
               "    vodist_tax_id = :vodist_tax_id, "
			   "    vodist_notes=:vodist_notes "
               "WHERE (vodist_id=:vodist_id);" );
  
  q.bindValue(":vodist_id", _vodistid);
  q.bindValue(":vodist_vohead_id", _voheadid);
  q.bindValue(":vodist_amount", _amount->localValue());
  q.bindValue(":vodist_discountable", QVariant(_discountable->isChecked()));
  q.bindValue(":vodist_notes", _notes->toPlainText().trimmed());
  if(_accountSelected->isChecked())
  {
    q.bindValue(":vodist_accnt_id", _account->id());
    q.bindValue(":vodist_expcat_id", -1);
    q.bindValue(":vodist_tax_id", -1);
  }
  else if (_expcatSelected->isChecked())
  {
    q.bindValue(":vodist_accnt_id", -1);
    q.bindValue(":vodist_expcat_id", _expcat->id());
    q.bindValue(":vodist_tax_id", -1);
  }
  else
  {
    q.bindValue(":vodist_accnt_id", -1);
    q.bindValue(":vodist_expcat_id", -1);
    q.bindValue(":vodist_tax_id", _taxCode->id()); 
  }
  q.exec();

  done(_vodistid);
}

void voucherMiscDistrib::sCheck()     
{
  
 if(_taxSelected->isChecked() && _mode == cEdit)
 {
  q.prepare( "SELECT vodist_id, vodist_vohead_id, vodist_amount "
              "FROM vodist " 
              "WHERE (vodist_id=:vodist_id) "
			  "   AND (vodist_poitem_id=-1)"
              "   AND (vodist_tax_id=:tax_id);" );
  
  q.bindValue(":vodist_id", _vodistid);
  q.bindValue(":tax_id", _taxCode->id());
  q.exec();
  if (q.first())
  {
    _amount->setFocus();
	_taxSelected->setEnabled(false);
	_taxCode->setEnabled(false);
	_expcatSelected->setEnabled(false);
	_accountSelected->setEnabled(false);
  }
 }
else if(_taxSelected->isChecked() && _mode == cNew)
{
   q.prepare( "SELECT vodist_id, vodist_amount "
              "FROM vodist " 
              "WHERE (vodist_vohead_id=:vodist_vohead_id) "
			  "   AND (vodist_poitem_id=-1)"
              "   AND (vodist_tax_id=:tax_id);" );
  
  q.bindValue(":vodist_vohead_id", _voheadid);
  q.bindValue(":tax_id", _taxCode->id());
  q.exec();
  if (q.first())
  {
    _vodistid=q.value("vodist_id").toInt();
    _amount->setLocalValue(q.value("vodist_amount").toDouble());
	_amount->setFocus();
	_mode=cEdit;
	_taxSelected->setEnabled(false);
	_taxCode->setEnabled(false);
	_expcatSelected->setEnabled(false);
	_accountSelected->setEnabled(false);
  }
 }
}

void voucherMiscDistrib::sPopulateVendorInfo(int pVendid)
{
  q.prepare( "SELECT vend_accnt_id, vend_expcat_id, vend_tax_id "
             "FROM vendinfo "
             "WHERE (vend_id=:vend_id);" );
  q.bindValue(":vend_id", pVendid);
  q.exec();
  if (q.first())
  {
    if(q.value("vend_accnt_id").toInt() != -1)
    {
      _accountSelected->setChecked(TRUE);
      _account->setId(q.value("vend_accnt_id").toInt());
      _amount->setFocus();
    }
    if(q.value("vend_expcat_id").toInt() != -1)
    {
      _expcatSelected->setChecked(TRUE);
      _expcat->setId(q.value("vend_expcat_id").toInt());
      _amount->setFocus();
    }
    if(q.value("vend_tax_id").toInt() != -1)
    {
      _taxSelected->setChecked(TRUE);
      _taxCode->setId(q.value("vend_tax_id").toInt());
      _amount->setFocus();
    }
  }
}

