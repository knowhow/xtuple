/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "filtersave.h"
#include "shortcuts.h"
#include <xsqlquery.h>
#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>
#include <QDialog>

filterSave::filterSave(QWidget* parent, const char* name)
    : QDialog(parent)
{
  if(name)
    setObjectName(name);

  setupUi(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(save()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  shortcuts::setStandardKeys(this);
}

void filterSave::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("filter", &valid);
  if (valid)
    _filter = param.toString();

  param = pParams.value("filtersetname", &valid);
  if (valid)
    _filterName->setText(param.toString());

  param = pParams.value("username", &valid);
  if (valid)
    _username = param.toString();

  param = pParams.value("classname", &valid);
  if (valid)
    _classname = param.toString();

  param = pParams.value("shared", &valid);
  if (valid)
    _shared->setChecked(true);
}

void filterSave::save()
{
  if ( _filterName->text().isEmpty() )
  {
    QMessageBox msgBox;
    msgBox.setText(tr("Please enter a name for this filter set before saving."));
    msgBox.exec();
    return;
  }

  int filterid = -1;
  QString user;
  QString query;
  XSqlQuery qry;

  if (_shared->isChecked())
    user = "null";
  else
    user = "getEffectiveXtUser()";

  //check to see if filter name exists for this screen
  QString filter_query = "select filter_id, filter_name, filter_username "
                         "from filter "
                         "where filter_name=:name "
                         " and COALESCE(filter_username,getEffectiveXtUser())=getEffectiveXtUser() "
                         " and filter_screen=:screen";
  qry.prepare(filter_query);
  qry.bindValue(":name", _filterName->text());
  qry.bindValue(":screen", _classname);

  qry.exec();

  //if the filter name is found, update it
  if (qry.first())
  {
    if (qry.value("filter_username").toString().isEmpty())
    {
      if ( QMessageBox::question( this, tr("Shared Filter Exists"),
                                  tr("<p>This will over-write a shared filter.\n"
                                     " Are you sure this is what you want to do?"),
                                  QMessageBox::No | QMessageBox::Default,
                                  QMessageBox::Yes) == QMessageBox::No )
        return;
    }

    filterid = qry.value("filter_id").toInt();
    query = QString("update filter set "
                    "  filter_value=:value, "
                    "  filter_username=%1 "
                    "where filter_id=:filter_id;").arg(user);
  }
  else
  {
    qry.prepare("SELECT nextval('filter_filter_id_seq') AS result");
    qry.exec();
    qry.first();
    filterid = qry.value("result").toInt();
    query = QString("insert into filter (filter_id, filter_screen, filter_name, filter_value, filter_username) "
            " values (:filter_id, :screen, :name, :value, %1); ").arg(user);
  }

  qry.prepare(query);
  qry.bindValue(":filter_id", filterid);
  qry.bindValue(":screen", _classname);
  qry.bindValue(":value", _filter);
  qry.bindValue(":name", _filterName->text() );

  if (qry.exec())
    done(filterid);
}

