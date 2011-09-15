/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "opportunityType.h"

#include <QVariant>
#include <QMessageBox>

/*
 *  Constructs a opportunityType as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
opportunityType::opportunityType(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_name, SIGNAL(lostFocus()), this, SLOT(sCheck()));
}

/*
 *  Destroys the object and frees any allocated resources
 */
opportunityType::~opportunityType()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void opportunityType::languageChange()
{
  retranslateUi(this);
}

enum SetResponse opportunityType::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("optype_id", &valid);
  if (valid)
  {
    _optypeid = param.toInt();
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
      _name->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _name->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
      _buttonBox->setFocus();
    }
  }

  return NoError;
}

void opportunityType::sCheck()
{
  _name->setText(_name->text().trimmed());
  if ((_mode == cNew) && (_name->text().length() != 0))
  {
    q.prepare( "SELECT optype_id "
               "FROM optype "
               "WHERE (UPPER(optype_name)=UPPER(:optype_name));" );
    q.bindValue(":optype_name", _name->text());
    q.exec();
    if (q.first())
    {
      _optypeid = q.value("optype_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(FALSE);
    }
  }
}

void opportunityType::sSave()
{
  _name->setText(_name->text().trimmed().toUpper());
  if (_name->text().length() == 0)
  {
    QMessageBox::information( this, tr("Invalid Name"),
                              tr("You must enter a valid Name for this Opportunity Type.") );
    _name->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('optype_optype_id_seq') AS optype_id");
    if (q.first())
      _optypeid = q.value("optype_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }

    q.prepare( "INSERT INTO optype "
               "( optype_id, optype_name, optype_descrip)"
               "VALUES "
               "( :optype_id, :optype_name, :optype_descrip );" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE optype "
               "   SET optype_name=:optype_name,"
               "       optype_descrip=:optype_descrip "
               " WHERE(optype_id=:optype_id);" );

  q.bindValue(":optype_id", _optypeid);
  q.bindValue(":optype_name", _name->text());
  q.bindValue(":optype_descrip", _description->text().trimmed());
  q.exec();

  done(_optypeid);
}

void opportunityType::populate()
{
  q.prepare( "SELECT optype_name, optype_descrip "
             "  FROM optype "
             " WHERE(optype_id=:optype_id);" );
  q.bindValue(":optype_id", _optypeid);
  q.exec();
  if (q.first())
  {
    _name->setText(q.value("optype_name"));
    _description->setText(q.value("optype_descrip"));
  }
} 

