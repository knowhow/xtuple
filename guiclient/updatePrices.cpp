/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "updatePrices.h"

#include <QCloseEvent>
#include <QVariant>
#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <metasql.h>
#include <parameter.h>
#include "mqlutil.h"
#include "guiclient.h"
#include "xdoublevalidator.h"

updatePrices::updatePrices(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_byItem,             SIGNAL(toggled(bool)), this, SLOT(sHandleBy(bool)));
  connect(_byItemGroup,        SIGNAL(toggled(bool)), this, SLOT(sHandleBy(bool)));
  connect(_byProductCategory,  SIGNAL(toggled(bool)), this, SLOT(sHandleBy(bool)));
  connect(_close,              SIGNAL(clicked()),     this, SLOT(close()));
  connect(_add,                SIGNAL(clicked()),     this, SLOT(sAdd()));
  connect(_addAll,             SIGNAL(clicked()),     this, SLOT(sAddAll()));
  connect(_remove,             SIGNAL(clicked()),     this, SLOT(sRemove()));
  connect(_removeAll,          SIGNAL(clicked()),     this, SLOT(sRemoveAll()));
  connect(_update,             SIGNAL(clicked()),     this, SLOT(sUpdate()));
  connect(_showEffective,      SIGNAL(clicked()),     this, SLOT(populate()));
  connect(_showExpired,        SIGNAL(clicked()),     this, SLOT(populate()));
  connect(_showCurrent,        SIGNAL(clicked()),     this, SLOT(populate()));
  connect(_value,              SIGNAL(clicked()),     this, SLOT(sHandleCharPrice()));
  connect(_percent,            SIGNAL(clicked()),     this, SLOT(sHandleCharPrice()));
  connect(_avail,              SIGNAL(itemSelected(int)), _add,    SLOT(animateClick()));
  connect(_sel,                SIGNAL(itemSelected(int)), _remove, SLOT(animateClick()));

  _updateBy->setValidator(new XDoubleValidator(-100, 9999, decimalPlaces("curr"), _updateBy));

  MetaSQLQuery mql = mqlLoad("updateprices", "createselsched");
  ParameterList params;
  q = mql.toQuery(params);
  if (q.lastError().type() != QSqlError::NoError)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

  _avail->addColumn(tr("Schedule"),      -1,          Qt::AlignLeft,  true,  "ipshead_name");
  _avail->addColumn(tr("Description"),   -1,          Qt::AlignLeft,  true,  "ipshead_descrip");
  _avail->addColumn(tr("Effective"),     -1,          Qt::AlignLeft,  true,  "ipshead_effective");
  _avail->addColumn(tr("Expires"),       -1,          Qt::AlignLeft,  true,  "ipshead_expires");
  _avail->setSelectionMode(QAbstractItemView::ExtendedSelection);

  _sel->addColumn(tr("Schedule"),        -1,          Qt::AlignLeft,  true,  "ipshead_name");
  _sel->addColumn(tr("Description"),     -1,          Qt::AlignLeft,  true,  "ipshead_descrip");

  _group->hide();
  //	_value->setChecked(true);

  populate();
}

updatePrices::~updatePrices()
{
  // no need to delete child widgets, Qt does it all for us
}

void updatePrices::languageChange()
{
  retranslateUi(this);
}

void updatePrices::closeEvent(QCloseEvent * /*pEvent*/)
{
  MetaSQLQuery mql = mqlLoad("updateprices", "dropselsched");
  ParameterList params;
  q = mql.toQuery(params);
  if (q.lastError().type() != QSqlError::NoError)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
}

void updatePrices::sUpdate()
{
  if (_byItem->isChecked() && !_item->isValid())
  {
    QMessageBox::critical( this, tr("Incomplete Data"),
                           tr("You must select an Item to continue.") );
    _item->setFocus();
    return;
  }

  if (!_sel->topLevelItemCount())
  {
    QMessageBox::critical( this, tr("Incomplete Data"),
                           tr("You must select a Pricing Schedule to continue.") );
    return;
  }

  if (_updateBy->toDouble() == 0.0)
  {
    QMessageBox::critical( this, tr("Incomplete Data"),
                           tr("You must provide a Value to continue.") );
    _updateBy->setFocus();
    return;
  }

  ParameterList params;
  
  if (_byItem->isChecked())
    params.append("item_id", _item->id());
  else
    _paramGroup->appendValue(params);
  params.append("updateBy", _updateBy->toDouble());
  if (_value->isChecked())
    params.append("updateByValue", true);
  else
    params.append("updateByPercent", true);

  MetaSQLQuery mql = mqlLoad("updateprices", "update");
  q = mql.toQuery(params);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_updateCharPrices->isChecked())
  {
    MetaSQLQuery mql2 = mqlLoad("updateprices", "updatechar");
    q = mql2.toQuery(params);
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  QMessageBox::information( this, tr("Success"),
                            tr("Update Completed.") );
  _updateBy->clear();
}

void updatePrices::populate()
{
  ParameterList params;
  if (_showEffective->isChecked())
    params.append("showEffective", true);
  if (_showExpired->isChecked())
    params.append("showExpired", true);
  if (_showCurrent->isChecked())
    params.append("showCurrent", true);

  MetaSQLQuery mql = mqlLoad("updateprices", "availsched");
  q = mql.toQuery(params);
  if (q.lastError().type() != QSqlError::NoError)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
  _avail->populate(q);

  MetaSQLQuery mql2 = mqlLoad("updateprices", "selsched");
  q = mql2.toQuery(params);
  if (q.lastError().type() != QSqlError::NoError)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
  _sel->populate(q);
}

void updatePrices::sAdd()
{
  QList<XTreeWidgetItem*> selected = _avail->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    MetaSQLQuery mql = mqlLoad("updateprices", "add");
    ParameterList params;
    params.append("ipshead_id", ((XTreeWidgetItem*)(selected[i]))->id());
    q = mql.toQuery(params);
    if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
  }
  populate();
}

void updatePrices::sAddAll()
{
  ParameterList params;
  if (_showEffective->isChecked())
    params.append("showEffective", true);
  if (_showExpired->isChecked())
    params.append("showExpired", true);
  if (_showCurrent->isChecked())
    params.append("showCurrent", true);

  MetaSQLQuery mql = mqlLoad("updateprices", "add");
  q = mql.toQuery(params);
  if (q.lastError().type() != QSqlError::NoError)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
  populate();
}

void updatePrices::sRemove()
{
  MetaSQLQuery mql = mqlLoad("updateprices", "remove");
  ParameterList params;
  params.append("ipshead_id", _sel->id());
  q = mql.toQuery(params);
  if (q.lastError().type() != QSqlError::NoError)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
  populate();
}

void updatePrices::sRemoveAll()
{
  MetaSQLQuery mql = mqlLoad("updateprices", "remove");
  ParameterList params;
  q = mql.toQuery(params);
  if (q.lastError().type() != QSqlError::NoError)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
  populate();
}

void updatePrices::sHandleBy(bool toggled)
{
  if (!toggled)
    return;
  if (_byItem->isChecked())
  {
    _paramGroup->hide();
    _group->show();
  }
  else
  {
    _group->hide();
    _paramGroup->show();
    if (_byItemGroup->isChecked())
      _paramGroup->setType(ParameterGroup::ItemGroup);
    else
      _paramGroup->setType(ParameterGroup::ProductCategory);
  }
}

void updatePrices::sHandleCharPrice()
{
  // Only enable update char prices for percentage updates.
  _updateCharPrices->setEnabled( _percent->isChecked() );
}
