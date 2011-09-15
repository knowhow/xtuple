/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "states.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "state.h"

states::states(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_country,SIGNAL(newID(int)), this, SLOT(sFillList()));
  connect(_delete,  SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit,    SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_new,     SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_view,    SIGNAL(clicked()), this, SLOT(sView()));

  _state->addColumn(tr("Abbreviation"),_itemColumn, Qt::AlignLeft, true, "state_abbr");
  _state->addColumn(tr("Name"),                 -1, Qt::AlignLeft, true, "state_name");
  _state->addColumn(tr("Country"),              -1, Qt::AlignLeft, true, "country_name");

  if (_privileges->check("MaintainStates"))
  {
    connect(_state, SIGNAL(valid(bool)),       _edit, SLOT(setEnabled(bool)));
    connect(_state, SIGNAL(valid(bool)),     _delete, SLOT(setEnabled(bool)));
    connect(_state, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_state, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  sFillList();
}

states::~states()
{
  // no need to delete child widgets, Qt does it all for us
}

void states::languageChange()
{
  retranslateUi(this);
}

void states::sFillList()
{
  MetaSQLQuery mql("SELECT state_id, state_abbr, state_name,"
                   "       country_name"
                   "  FROM state JOIN country ON (state_country_id=country_id) "
                   "<? if exists(\"country_id\") ?>"
                   " WHERE (state_country_id=<? value(\"country_id\") ?>) "
                   "<? endif ?>"
                   "ORDER BY country_name, state_name;");
  ParameterList params;
  if (_country->id() >= 0)
    params.append("country_id", _country->id());
  q = mql.toQuery(params);
  _state->populate(q);
}

void states::sDelete()
{
  q.prepare("SELECT EXISTS(SELECT 1"
            "                FROM addr JOIN state ON (addr_state=state_abbr)"
            "               WHERE ((addr_country=:country_name)"
            "                  AND (state_id=:state_id))) AS isused;");
  q.bindValue(":state_id",     _state->id());
  q.bindValue(":country_name", _state->currentItem()->text("country_name"));
  q.exec();
  if (q.first() && q.value("isused").toBool() &&
        QMessageBox::question(this, tr("Delete State?"),
                              tr("<p>The state %1 is used in addresses. "
                                 "Are you sure you want to delete it?")
                                .arg(_state->currentItem()->text("state_name")),
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    return;
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else if (QMessageBox::question(this, tr("Delete State?"),
                                 tr("<p>Are you sure you want to delete this state?"),
                            QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    return;

  q.prepare( "DELETE FROM state "
             "WHERE (state_id=:state_id);" );
  q.bindValue(":state_id", _state->id());
  q.exec();

  sFillList();
}

void states::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  if (_country->isValid())
  {
    //if (DEBUG)
      qDebug("setting country_id to %d", _country->id());
    params.append("country_id", _country->id());
  }

  state newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
  sFillList();
}

void states::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("state_id", _state->id());

  state newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
  sFillList();
}

void states::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("state_id", _state->id());

  state newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

