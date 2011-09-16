/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "itemGroups.h"

#include <parameter.h>
#include "itemGroup.h"
#include "guiclient.h"

itemGroups::itemGroups(QWidget* parent, const char* name, Qt::WFlags fl)
  : XWidget(parent, name, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_itemgrp, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));

  _itemgrp->addColumn(tr("Name"),        _itemColumn, Qt::AlignLeft, true, "itemgrp_name" );
  _itemgrp->addColumn(tr("Description"), -1,          Qt::AlignLeft, true, "itemgrp_descrip" );
  
  if (_privileges->check("MaintainItemGroups"))
  {
    connect(_itemgrp, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_itemgrp, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_itemgrp, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_itemgrp, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  connect(omfgThis, SIGNAL(itemGroupsUpdated(int, bool)), this, SLOT(sFillList(int)));

  sFillList(-1);
}

itemGroups::~itemGroups()
{
  // no need to delete child widgets, Qt does it all for us
}

void itemGroups::languageChange()
{
  retranslateUi(this);
}

void itemGroups::sDelete()
{
  q.prepare( "DELETE FROM itemgrpitem "
             "WHERE (itemgrpitem_itemgrp_id=:itemgrp_id);"

             "DELETE FROM itemgrp "
             "WHERE (itemgrp_id=:itemgrp_id);" );
  q.bindValue(":itemgrp_id", _itemgrp->id());
  q.exec();

  sFillList(-1);
}


void itemGroups::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  itemGroup *newdlg = new itemGroup();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void itemGroups::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("itemgrp_id", _itemgrp->id());

  itemGroup *newdlg = new itemGroup();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void itemGroups::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("itemgrp_id", _itemgrp->id());

  itemGroup *newdlg = new itemGroup();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void itemGroups::sFillList(int)
{
  _itemgrp->populate( "SELECT itemgrp_id, itemgrp_name, itemgrp_descrip "
                      "FROM itemgrp "
                      "ORDER BY itemgrp_name;" );
}

