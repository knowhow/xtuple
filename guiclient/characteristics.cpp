/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "characteristics.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <parameter.h>
#include <openreports.h>

#include "characteristic.h"
#include "storedProcErrorLookup.h"

characteristics::characteristics(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_char, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *, int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit,   SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_new,    SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_print,  SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_view,   SIGNAL(clicked()), this, SLOT(sView()));

  if (_privileges->check("MaintainCharacteristics"))
  {
    connect(_char, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
    connect(_char, SIGNAL(valid(bool)),     _delete, SLOT(setEnabled(bool)));
    connect(_char, SIGNAL(valid(bool)),       _edit, SLOT(setEnabled(bool)));
    connect(_char, SIGNAL(valid(bool)),       _view, SLOT(setEnabled(bool)));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_char, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
    connect(_char, SIGNAL(valid(bool)),       _view, SLOT(setEnabled(bool)));
  }

  _char->addColumn(tr("Name"), _itemColumn, Qt::AlignLeft, true, "char_name");
  _char->addColumn(tr("Description"),   -1, Qt::AlignLeft, true, "char_notes");

  sFillList();
}

characteristics::~characteristics()
{
  // no need to delete child widgets, Qt does it all for us
}

void characteristics::languageChange()
{
  retranslateUi(this);
}

void characteristics::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  characteristic newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void characteristics::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("char_id", _char->id());

  characteristic newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void characteristics::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("char_id", _char->id());

  characteristic newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void characteristics::sDelete()
{
  q.prepare("SELECT deleteCharacteristic(:char_id) AS result;");
  q.bindValue(":char_id", _char->id());
  q.exec();
  if (q.first())
  {
    int returnVal = q.value("result").toInt();
    if (returnVal < 0)
    {
      QMessageBox::critical( this, tr("Cannot Delete Characteristic"),
                             storedProcErrorLookup("deleteCharacteristic",
						   returnVal));
      return;
    }

    sFillList();
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void characteristics::sPrint()
{
  orReport report("CharacteristicsMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void characteristics::sFillList()
{
  q.prepare("SELECT char_id, char_name, char_notes "
            "FROM char "
            "ORDER BY char_name;");
  q.exec();
  _char->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void characteristics::sPopulateMenu(QMenu *)
{
}
