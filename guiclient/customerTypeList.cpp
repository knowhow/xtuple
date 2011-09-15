/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "customerTypeList.h"

#include <QVariant>

customerTypeList::customerTypeList(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_select, SIGNAL(clicked()), this, SLOT(sSelect()));
  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_clear, SIGNAL(clicked()), this, SLOT(sClear()));

  _custtype->addColumn(tr("Code"), _itemColumn, Qt::AlignCenter,true, "custtype_code");
  _custtype->addColumn(tr("Description"),   -1, Qt::AlignLeft,  true, "custtype_descrip");
}

customerTypeList::~customerTypeList()
{
  // no need to delete child widgets, Qt does it all for us
}

void customerTypeList::languageChange()
{
  retranslateUi(this);
}

enum SetResponse customerTypeList::set(ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("custtype_id", &valid);
  if (valid)
    _custtypeid = param.toInt();
  else
    _custtypeid = -1;

  sFillList();

  return NoError;
}

void customerTypeList::sClose()
{
  done(_custtypeid);
}

void customerTypeList::sClear()
{
  done(-1);
}

void customerTypeList::sSelect()
{
  done(_custtype->id());
}

void customerTypeList::sFillList()
{
  _custtype->populate( "SELECT custtype_id, custtype_code, custtype_descrip "
                       "FROM custtype "
                       "ORDER BY custtype_code;", _custtypeid );
}
