/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "itemSourceList.h"

#include <qvariant.h>

/*
 *  Constructs a itemSourceList as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
itemSourceList::itemSourceList(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_itemsrc, SIGNAL(valid(bool)), _select, SLOT(setEnabled(bool)));
    connect(_itemsrc, SIGNAL(itemSelected(int)), _select, SLOT(animateClick()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_select, SIGNAL(clicked()), this, SLOT(sSelect()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
itemSourceList::~itemSourceList()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void itemSourceList::languageChange()
{
    retranslateUi(this);
}


void itemSourceList::init()
{
  _itemsrc->addColumn(tr("Ranking"),      _orderColumn, Qt::AlignRight, true, "itemsrc_ranking" );
  _itemsrc->addColumn(tr("Vendor"),       -1,           Qt::AlignLeft,  true, "vend_name");
  _itemsrc->addColumn(tr("Vend Item#"),   _itemColumn, Qt::AlignRight, true, "itemsrc_vend_item_number" );
  _itemsrc->addColumn(tr("Manufacturer"), _itemColumn, Qt::AlignLeft,  true, "itemsrc_manuf_name");
  _itemsrc->addColumn(tr("Manuf. Item#"), _itemColumn, Qt::AlignRight, true, "itemsrc_manuf_item_number" );
  _itemsrc->addColumn(tr("Default"),      _itemColumn, Qt::AlignLeft, true, "default" );
}

enum SetResponse itemSourceList::set(ParameterList &pParams)
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
  q.prepare( "SELECT itemsrc_id, itemsrc_ranking, vend_name, "
             "  itemsrc_vend_item_number, itemsrc_manuf_name, "
             "  itemsrc_manuf_item_number, "
			 "  CASE WHEN itemsrc_default = 'TRUE' THEN 'Yes' "
			 "    ELSE 'No' "
			 "  END AS default "
             "FROM itemsrc, vend "
             "WHERE ( (itemsrc_vend_id=vend_id)"
             " AND (itemsrc_item_id=:item_id) ) "
             "ORDER BY itemsrc_ranking, vend_name, "
             "  itemsrc_vend_item_number, itemsrc_manuf_name, "
             "  itemsrc_manuf_item_number;" );
  q.bindValue(":item_id", _item->id());
  q.exec();
  _itemsrc->populate(q);
}

