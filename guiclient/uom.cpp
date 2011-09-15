/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "uom.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>
#include <parameter.h>
#include "storedProcErrorLookup.h"
#include "uomConv.h"

/*
 *  Constructs a uom as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
uom::uom(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_name, SIGNAL(lostFocus()), this, SLOT(sCheck()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_uomconv, SIGNAL(itemSelectionChanged()), this, SLOT(sSelected()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));

  _uomconv->addColumn(tr("UOM/UOM"),    -1,        Qt::AlignLeft,   true,  "uomuom");
  _uomconv->addColumn(tr("From Value"), -1,        Qt::AlignRight,  true,  "uomconv_from_value");
  _uomconv->addColumn(tr("To Value"),   -1,        Qt::AlignRight,  true,  "uomconv_to_value");
  _uomconv->addColumn(tr("Fractional"), _ynColumn, Qt::AlignCenter, true,  "uomconv_fractional");
}

/*
 *  Destroys the object and frees any allocated resources
 */
uom::~uom()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void uom::languageChange()
{
  retranslateUi(this);
}

enum SetResponse uom::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("uom_id", &valid);
  if (valid)
  {
    _uomid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
      _mode = cNew;
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _buttonBox->setFocus();
      _new->setEnabled(true);
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _name->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _weightUom->setEnabled(FALSE);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
      _buttonBox->setFocus();

      _edit->setText(tr("&View"));
    }
  }

  return NoError;
}

void uom::sSave()
{
  if (_name->text().length() == 0)
  {
    QMessageBox::information( this, tr("No UOM Name Entered"),
                              tr("You must enter a valid UOM name before saving this UOM.") );
    _name->setFocus();
    return;
  }
  
  if (_weightUom->isChecked())
  {
    if (_mode == cNew)
      q.exec("SELECT uom_id FROM uom WHERE (uom_item_weight);");
    else
    {
      q.prepare("SELECT uom_id FROM uom WHERE ((uom_item_weight) AND (uom_id<>:uom_id));");
      q.bindValue(":uom_id", _uomid);
      q.exec();
    }
    if (q.first())
    {
      int response = QMessageBox::warning (this, tr("Set Item Weight?"),
                                                 tr("The Item Weight UOM has already been set. "
                                                    "Are you sure you want to clear the existing entry and set "
                                                    "%1 to be the Item Weight UOM?")
                                          .arg(_name->text()),
                                          QMessageBox::Yes | QMessageBox::Escape,
                                          QMessageBox::No | QMessageBox::Default);
      if (response == QMessageBox::Yes)
        q.exec("UPDATE uom SET uom_item_weight=FALSE;");
      else
        return;
    }
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('uom_uom_id_seq') AS uom_id;");
    if (q.first())
      _uomid = q.value("uom_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }
 
    q.prepare( "INSERT INTO uom "
               "( uom_id, uom_name, uom_descrip, uom_item_weight ) "
               "VALUES "
               "( :uom_id, :uom_name, :uom_descrip, :uom_item_weight );" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE uom "
               "SET uom_name=:uom_name, uom_descrip=:uom_descrip,"
               "    uom_item_weight=:uom_item_weight "
               "WHERE (uom_id=:uom_id);" );

  q.bindValue(":uom_id", _uomid);
  q.bindValue(":uom_name", _name->text());
  q.bindValue(":uom_descrip", _description->text());
  q.bindValue(":uom_item_weight", QVariant(_weightUom->isChecked()));
  q.exec();

  done(_uomid);
}

void uom::sCheck()
{
  _name->setText(_name->text().trimmed().toUpper());
  if ( (_mode == cNew) && (_name->text().length()) )
  {
    q.prepare( "SELECT uom_id "
               "FROM uom "
               "WHERE (UPPER(uom_name)=UPPER(:uom_name));" );
    q.bindValue(":uom_name", _name->text());
    q.exec();
    if (q.first())
    {
      _uomid = q.value("uom_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(FALSE);
    }
  }
}

void uom::populate()
{
  q.prepare( "SELECT uom_name, uom_descrip, uom_item_weight "
             "  FROM uom "
             " WHERE(uom_id=:uom_id);" );
  q.bindValue(":uom_id", _uomid);
  q.exec();
  if (q.first())
  {
    _name->setText(q.value("uom_name").toString());
    _description->setText(q.value("uom_descrip").toString());
    _weightUom->setChecked(q.value("uom_item_weight").toBool());

    sFillList();
  }
}

void uom::sFillList()
{
  q.prepare("SELECT uomconv_id,"
            "       (nuom.uom_name||'/'||duom.uom_name) AS uomuom,"
            "       uomconv_from_value, uomconv_to_value,"
            "       uomconv_fractional,"
            "       'uomratio' AS uomconv_from_value_xtnumericrole,"
            "       'uomratio' AS uomconv_to_value_xtnumericrole "
            "  FROM uomconv"
            "  JOIN uom AS nuom ON (uomconv_from_uom_id=nuom.uom_id)"
            "  JOIN uom AS duom ON (uomconv_to_uom_id=duom.uom_id)"
            " WHERE((uomconv_from_uom_id=:uom_id)"
            "    OR (uomconv_to_uom_id=:uom_id));");
  q.bindValue(":uom_id", _uomid);
  q.exec();
  _uomconv->populate(q);
}

void uom::sSelected()
{
  if(_uomconv->id() != -1)
  {
    _edit->setEnabled(true);
    _edit->setText((_mode==cView?tr("&View"):tr("&Edit")));
    _delete->setEnabled(!(_mode==cView));
  }
  else
  {
    _edit->setEnabled(false);
    _delete->setEnabled(false);
  }
}

void uom::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("from_uom_id", _uomid);

  uomConv newdlg(this, "", TRUE);
  newdlg.set(params);
  if(newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void uom::sEdit()
{
  ParameterList params;
  if(_mode == cView)
    params.append("mode", "view");
  else
    params.append("mode", "edit");
  params.append("uomconv_id", _uomconv->id());

  uomConv newdlg(this, "", TRUE);
  newdlg.set(params);
  if(newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void uom::sDelete()
{
  q.prepare( "SELECT deleteUOMConv(:uomconv_id) AS result;" );
  q.bindValue(":uomconv_id", _uomconv->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteUOMConv", result), __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

