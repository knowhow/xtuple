/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "bankAdjustmentType.h"

#include <QMessageBox>
#include <QVariant>

bankAdjustmentType::bankAdjustmentType(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_name, SIGNAL(lostFocus()), this, SLOT(sCheck()));
}

bankAdjustmentType::~bankAdjustmentType()
{
  // no need to delete child widgets, Qt does it all for us
}

void bankAdjustmentType::languageChange()
{
  retranslateUi(this);
}

enum SetResponse bankAdjustmentType::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("bankadjtype_id", &valid);
  if (valid)
  {
    _bankadjtypeid = param.toInt();
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
      _description->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _name->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _accnt->setEnabled(FALSE);
      _senseGroup->setEnabled(FALSE);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
      _buttonBox->setFocus();
    }
  }

  return NoError;
}

void bankAdjustmentType::sSave()
{
  if (_name->text().length() == 0)
  {
    QMessageBox::information( this, tr("Cannot Save Adjustment Type"),
                              tr("You must enter a valid name for this Adjustment Type.") );
    _name->setFocus();
    return;
  }

  if (_accnt->id() == -1)
  {
    QMessageBox::information( this, tr("Cannot Save Adjustment Type"),
                              tr("You must select a valid account for this Adjustment Type.") );
    return;
  }
  
  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('bankadjtype_bankadjtype_id_seq') AS bankadjtype_id");
    if (q.first())
      _bankadjtypeid = q.value("bankadjtype_id").toInt();

    q.prepare( "INSERT INTO bankadjtype "
               "(bankadjtype_id, bankadjtype_name, bankadjtype_descrip, bankadjtype_accnt_id, bankadjtype_iscredit) "
               "VALUES "
               "(:bankadjtype_id, :bankadjtype_name, :bankadjtype_descrip, :bankadjtype_accnt_id, :bankadjtype_iscredit);" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE bankadjtype "
               "SET bankadjtype_name=:bankadjtype_name,"
               "    bankadjtype_descrip=:bankadjtype_descrip, "
               "    bankadjtype_accnt_id=:bankadjtype_accnt_id,"
               "    bankadjtype_iscredit=:bankadjtype_iscredit "
               "WHERE (bankadjtype_id=:bankadjtype_id);" );

  q.bindValue(":bankadjtype_id", _bankadjtypeid);
  q.bindValue(":bankadjtype_name", _name->text());
  q.bindValue(":bankadjtype_descrip", _description->text().trimmed());
  q.bindValue(":bankadjtype_accnt_id", _accnt->id());
  q.bindValue(":bankadjtype_iscredit", _credit->isChecked());
  q.exec();

  done(_bankadjtypeid);
}

void bankAdjustmentType::sCheck()
{
  _name->setText(_name->text().trimmed());
  if ((_mode == cNew) && (_name->text().length()))
  {
    q.prepare( "SELECT bankadjtype_id "
               "FROM bankadjtype "
               "WHERE (UPPER(bankadjtype_name)=UPPER(:bankadjtype_name));" );
    q.bindValue(":bankadjtype_name", _name->text());
    q.exec();
    if (q.first())
    {
      _bankadjtypeid = q.value("bankadjtype_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(FALSE);
    }
  }
}

void bankAdjustmentType::populate()
{
  q.prepare( "SELECT bankadjtype_name, bankadjtype_descrip,"
             "       bankadjtype_accnt_id, bankadjtype_iscredit "
             "FROM bankadjtype "
             "WHERE (bankadjtype_id=:bankadjtype_id);" );
  q.bindValue(":bankadjtype_id", _bankadjtypeid);
  q.exec();
  if (q.first())
  {
    _name->setText(q.value("bankadjtype_name"));
    _description->setText(q.value("bankadjtype_descrip"));
    _accnt->setId(q.value("bankadjtype_accnt_id").toInt());
    if(q.value("bankadjtype_iscredit").toBool())
      _credit->setChecked(true);
  }
} 
