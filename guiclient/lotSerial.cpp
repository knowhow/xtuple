/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "lotSerial.h"
#include "characteristicAssignment.h"
#include "lotSerialRegistration.h"

#include <QSqlError>
#include <QVariant>
#include <QMessageBox>

#include <openreports.h>
#include <comments.h>

lotSerial::lotSerial(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    _print = _buttonBox->addButton(tr("Print Label"),QDialogButtonBox::ActionRole);

    connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
    connect(_lotSerial, SIGNAL(valid(bool)), this, SLOT(populate()));
    connect(_notes, SIGNAL(textChanged()), this, SLOT(sChanged()));
    connect(_deleteChar,  SIGNAL(clicked()), this, SLOT(sDeleteCharass()));
    connect(_editChar,    SIGNAL(clicked()), this, SLOT(sEditCharass()));
    connect(_newChar,     SIGNAL(clicked()), this, SLOT(sNewCharass()));
    connect(_deleteReg,  SIGNAL(clicked()), this, SLOT(sDeleteReg()));
    connect(_editReg,    SIGNAL(clicked()), this, SLOT(sEditReg()));
    connect(_newReg,     SIGNAL(clicked()), this, SLOT(sNewReg()));
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));

    
    _charass->addColumn(tr("Characteristic"), _itemColumn, Qt::AlignLeft, true, "char_name" );
    _charass->addColumn(tr("Value"),          -1,          Qt::AlignLeft, true, "charass_value" );
    
    _reg->addColumn(tr("Number")      ,        _orderColumn,  Qt::AlignLeft, true, "lsreg_number" );
    _reg->addColumn(tr("CRM Account#"),        _itemColumn,  Qt::AlignLeft, true, "crmacct_number" );
    _reg->addColumn(tr("Name"        ),	      -1,   Qt::AlignLeft, true, "crmacct_name" );
    _reg->addColumn(tr("First Name"  ),        80,  Qt::AlignLeft, true, "cntct_first_name" );
    _reg->addColumn(tr("Last Name"   ),        80,  Qt::AlignLeft, true, "cntct_last_name" );
    _reg->addColumn(tr("Phone"       ),        80,  Qt::AlignLeft, true, "cntct_phone" );
  
    _changed=false;
}

lotSerial::~lotSerial()
{
    // no need to delete child widgets, Qt does it all for us
}

void lotSerial::languageChange()
{
    retranslateUi(this);
}

void lotSerial::populate()
{
  if (_changed)
  {
    if (QMessageBox::question(this, tr("Save changes?"),
                              tr("<p>Notes were changed without saving. "
                                 "If you continue your changes will be lost. "
                                 "Would you like an opportunity to save your changes first?"),
                              QMessageBox::Yes | QMessageBox::Default,
                              QMessageBox::No) == QMessageBox::Yes)
    {
      disconnect(_lotSerial, SIGNAL(valid(bool)), this, SLOT(populate()));
      _item->setId(_itemidCache);
      _lotSerial->setId(_lsidCache);
      connect(_lotSerial, SIGNAL(valid(bool)), this, SLOT(populate()));
      return;
    }
  }

  q.prepare( "SELECT ls_item_id,ls_notes "
             "FROM ls "
             "WHERE (ls_id=:ls_id );" );
  q.bindValue(":ls_id", _lotSerial->id());
  q.exec();
  if (q.first())
  {
    _lsidCache=_lotSerial->id();
    if (_item->id() == -1)
      _item->setId(q.value("ls_item_id").toInt());
    _itemidCache=_item->id();
    _notes->setText(q.value("ls_notes").toString());
    _changed=false;
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillList();
}

void lotSerial::sSave()
{
  q.prepare("UPDATE ls SET"
            " ls_notes=:notes "
            "WHERE (ls_id=:ls_id);");
  q.bindValue(":notes",_notes->toPlainText());
  q.bindValue(":ls_id", _lotSerial->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  _notes->clear();
  _changed=false;
  _item->setId(-1);
}

void lotSerial::sChanged()
{
  _changed=true;
}

void lotSerial::sNewCharass()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("ls_id", _lotSerial->id());

  characteristicAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void lotSerial::sEditCharass()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("charass_id", _charass->id());

  characteristicAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void lotSerial::sDeleteCharass()
{
  q.prepare( "DELETE FROM charass "
             "WHERE (charass_id=:charass_id);" );
  q.bindValue(":charass_id", _charass->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void lotSerial::sNewReg()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("item_id", _item->id());
  params.append("ls_id", _lotSerial->id());

  lotSerialRegistration newdlg(this, "", TRUE);
  newdlg.set(params);
  
  newdlg.exec();
  sFillList();
}

void lotSerial::sEditReg()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("lsreg_id", _reg->id());

  lotSerialRegistration newdlg(this, "", TRUE);
  newdlg.set(params);
  
  newdlg.exec();
  sFillList();
}

void lotSerial::sDeleteReg()
{
  q.prepare( "DELETE FROM lsreg "
             "WHERE (lsreg_id=:lsreg_id);"
             "DELETE FROM charass "
             "WHERE ((charass_target_type='LSR') "
             "AND (charass_target_id=:lsreg_id))" );
  q.bindValue(":lsreg_id", _reg->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}


void lotSerial::sFillList()
{
  q.prepare( "SELECT charass_id, char_name, "
             " CASE WHEN char_type < 2 THEN "
             "   charass_value "
             " ELSE "
             "   formatDate(charass_value::date) "
             "END AS charass_value "
             "FROM charass, char "
             "WHERE ((charass_target_type='LS')"
             " AND   (charass_char_id=char_id)"
             " AND   (charass_target_id=:ls_id) ) "
             "ORDER BY char_order, char_name;" );
  q.bindValue(":ls_id", _lotSerial->id());
  q.exec();
  _charass->clear();
  _charass->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  
  q.prepare( "SELECT lsreg_id,lsreg_number,crmacct_number,crmacct_name,"
             "  cntct_first_name,cntct_last_name,cntct_phone "
             "FROM lsreg "
             "  LEFT OUTER JOIN crmacct ON (lsreg_crmacct_id=crmacct_id), "
             "  cntct "
             "WHERE ((lsreg_cntct_id=cntct_id) "
             "AND (lsreg_ls_id=:ls_id));");
  q.bindValue(":ls_id", _lotSerial->id());
  q.exec();
  _reg->clear();
  _reg->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void lotSerial::sPrint()
{
  QString lot(tr("Lot#:"));
  QString serial(tr("Serial#:"));
  ParameterList params;
  params.append("ls_id", _lotSerial->id());

  // Try to infer whether this requires a lot or serial number label
  q.prepare("SELECT DISTINCT itemsite_controlmethod "
            "FROM itemsite "
            "WHERE ((itemsite_item_id=:item_id)"
            " AND (itemsite_controlmethod IN ('L','S')));");
  q.bindValue(":item_id", _item->id());
  q.exec();
  if (q.size() == 1) {
    q.first();
    if (q.value("itemsite_controlmethod").toString() == "L")
      params.append("label", lot);
    else
      params.append("label", serial);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else {
    QString msg(tr("Is this a Lot or a Serial number?"));
    QMessageBox msgbox;
    msgbox.setText(msg);
    msgbox.addButton(tr("Lot"),QMessageBox::AcceptRole);
    msgbox.addButton(tr("Serial"),QMessageBox::RejectRole);
    if (msgbox.exec() == QDialog::Accepted)
      params.append("label", serial);
    else
      params.append("label", lot);
  }

  orReport report("LotSerialLabel",params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}
