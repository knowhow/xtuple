/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "itemSourceList.h"

#include <metasql.h>
#include <QVariant>
#include "mqlutil.h"

itemSourceList::itemSourceList(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_itemsrc, SIGNAL(valid(bool)), _select, SLOT(setEnabled(bool)));
  connect(_itemsrc, SIGNAL(itemSelected(int)), _select, SLOT(animateClick()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_select, SIGNAL(clicked()), this, SLOT(sSelect()));

  _itemsrc->addColumn(tr("Ranking"),      _orderColumn, Qt::AlignRight,  true, "itemsrc_ranking" );
  _itemsrc->addColumn(tr("Vendor"),       -1,           Qt::AlignLeft,   true, "vend_name");
  _itemsrc->addColumn(tr("Vend Item#"),   _itemColumn,  Qt::AlignRight,  true, "itemsrc_vend_item_number" );
  _itemsrc->addColumn(tr("Manufacturer"), _itemColumn,  Qt::AlignLeft,   true, "itemsrc_manuf_name");
  _itemsrc->addColumn(tr("Manuf. Item#"), _itemColumn,  Qt::AlignRight,  true, "itemsrc_manuf_item_number" );
  _itemsrc->addColumn(tr("Default"),      _itemColumn,  Qt::AlignLeft,   true, "itemsrc_default" );
}

itemSourceList::~itemSourceList()
{
  // no need to delete child widgets, Qt does it all for us
}

void itemSourceList::languageChange()
{
  retranslateUi(this);
}

enum SetResponse itemSourceList::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _item->setId(param.toInt());
    _item->setEnabled(FALSE);
  }

  param = pParams.value("qty", &valid);

  sFillList();

  return NoError;
}

void itemSourceList::sSelect()
{
  done(_itemsrc->id());
}

void itemSourceList::sFillList()
{
  MetaSQLQuery mql = mqlLoad("itemSources", "detail");

  ParameterList params;
  params.append("item_id", _item->id());
  params.append("onlyShowActive", true);
  q = mql.toQuery(params);
  _itemsrc->populate(q);
}

