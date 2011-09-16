/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "filterManager.h"
#include "parameterwidget.h"
#include "shortcuts.h"
#include <xtreewidget.h>
#include <xsqlquery.h>
#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>
#include <QDialog>
#include <QtDebug>

filterManager::filterManager(QWidget* parent, const char* name)
    : QDialog(parent)
{
  if(name)
    setObjectName(name);

  setupUi(this);

  _filterSet->addColumn(tr("Filter Set Name"), -1, Qt::AlignLeft, true, "filter_name" );
  _filterSet->addColumn(tr("Shared"), _ynColumn, Qt::AlignLeft, true, "shared");

  connect(_filterSet, SIGNAL(valid(bool)), this, SLOT( handleButtons(bool) ));
  connect(_share, SIGNAL(clicked()), this, SLOT( shareFilter() ));
  connect(_unshare, SIGNAL(clicked()), this, SLOT( unshareFilter() ));
  connect(_delete, SIGNAL(clicked()), this, SLOT( deleteFilter() ) );
  connect(this, SIGNAL(filterDeleted()), parent, SLOT(setSavedFilters()) );

  shortcuts::setStandardKeys(this);
}

void filterManager::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("screen", &valid);
  if (valid)
  {
    _screen = param.toString();
    populate();
  }
}

void filterManager::populate()
{
  if (parent())
  {
    if (parent()->parent())
    {
      XSqlQuery qry;
      qry.prepare("select filter_id, filter_name, "
                  " case when filter_username is null then "
                  "  true "
                  " else false end as shared "
                  "from filter "
                  "where COALESCE(filter_username,getEffectiveXtUser())=getEffectiveXtUser() "
                  " and filter_screen=:screen "
                  "order by filter_name;");
      qry.bindValue(":screen", parent()->parent()->objectName());
      qry.exec();

      _filterSet->populate(qry, false);
    }
  }
}
void filterManager::unshareFilter()
{
	XSqlQuery qry;
	if ( QMessageBox::question( this, tr("Unshare Filter"),
                                tr("<p>This will un-share the selected filter and assign it to you.\n"
                                   " Are you sure this is what you want to do?"),
                                QMessageBox::No | QMessageBox::Default,
                                QMessageBox::Yes) == QMessageBox::No )
      return;
	else
	{
		qry.prepare("UPDATE filter SET filter_username=getEffectiveXtUser() WHERE (filter_id=:filter_id);");
		qry.bindValue(":filter_id", _filterSet->id());
		qry.exec();
	}

	populate();

}
void filterManager::shareFilter()
{
  XSqlQuery qry;

  // See if there is already a shared filter by this name
  qry.prepare("SELECT shr.filter_id AS filter_id "
              "FROM filter prv "
              " JOIN filter shr ON ((prv.filter_name=shr.filter_name) "
              "                 AND (prv.filter_screen=shr.filter_screen) "
              "                 AND (shr.filter_id != prv.filter_id)) "
              "WHERE ((shr.filter_username IS NULL) "
              " AND (prv.filter_id=:filter_id)); ");
  qry.bindValue(":filter_id", _filterSet->id());
  qry.exec();
  if (qry.first())
  {
    if ( QMessageBox::question( this, tr("Shared Filter Exists"),
                                tr("<p>This will replace a shared filter with the same name.\n"
                                   " Are you sure this is what you want to do?"),
                                QMessageBox::No | QMessageBox::Default,
                                QMessageBox::Yes) == QMessageBox::No )
      return;
    else
    {
      int filterid = qry.value("filter_id").toInt();
      qry.prepare("UPDATE filter shr SET filter_value=prv.filter_value "
                  "FROM filter prv "
                  "WHERE ((shr.filter_id=:shr_filter_id) "
                  " AND (prv.filter_id=:prv_filter_id)); "
                  "DELETE FROM filter WHERE (filter_id=:prv_filter_id);");
      qry.bindValue(":shr_filter_id", filterid);
      qry.bindValue(":prv_filter_id", _filterSet->id());
      qry.exec();

      emit filterDeleted();
    }
  }
  else
  {
    qry.prepare("UPDATE filter SET filter_username=NULL WHERE (filter_id=:filter_id);");
    qry.bindValue(":filter_id", _filterSet->id());
    qry.exec();
  }

  populate();
}

void filterManager::deleteFilter()
{
  QString query = "delete from filter where filter_id=:filter_id";
  XSqlQuery qry;

  qry.prepare(query);
  qry.bindValue(":filter_id", _filterSet->id());
  qry.exec();

  this->populate();

  emit filterDeleted();
}

void filterManager::handleButtons(bool valid)
{
  _share->setEnabled(valid &&
                     !_filterSet->currentItem()->rawValue("shared").toBool());
  _delete->setEnabled(valid);

	_unshare->setEnabled(valid && _filterSet->currentItem()->rawValue("shared").toBool());
}
