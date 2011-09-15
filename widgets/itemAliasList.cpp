/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#include "itemAliasList.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QVariant>

#include <parameter.h>
#include <xsqlquery.h>

#include "itemcluster.h"
#include "xtreewidget.h"

QString buildItemLineEditQuery(const QString, const QStringList, const QString, const unsigned int);
QString buildItemLineEditTitle(const unsigned int, const QString);

itemAliasList::itemAliasList(QWidget* parent, const char* name, bool modal, Qt::WFlags fl) :
  QDialog(parent, fl)
{
  setObjectName(name ? name : "itemAliasList");
  setModal(modal);

  setWindowTitle(tr( "Item Aliases"));

  _itemType = ItemLineEdit::cUndefined;

  if ( !name )
   setObjectName( "itemAliasList" );

  QVBoxLayout *_mainLayout = new QVBoxLayout(this);
  QHBoxLayout *_itemLayout = new QHBoxLayout(0);
  QHBoxLayout *_line1Layout = new QHBoxLayout(0);
  QVBoxLayout *_enterLayout = new QVBoxLayout(0);
  QHBoxLayout *_topLayout = new QHBoxLayout( 0);
  QVBoxLayout *_buttonsLayout= new QVBoxLayout(0);
  QVBoxLayout *_listLayout = new QVBoxLayout(0);

  _mainLayout->setContentsMargins(5, 5, 5, 5); 
  _itemLayout->setContentsMargins(0, 0, 0, 0); 
  _line1Layout->setContentsMargins(0, 0, 0, 0); 
  _enterLayout->setContentsMargins(0, 0, 0, 0); 
  _topLayout->setContentsMargins(0, 0, 0, 0); 
  _buttonsLayout->setContentsMargins(0, 0, 0, 0); 
  _listLayout->setContentsMargins(0, 0, 0, 0); 

  _mainLayout->setSpacing(5); 
  _itemLayout->setSpacing(5); 
  _line1Layout->setSpacing(7); 
  _enterLayout->setSpacing(5); 
  _topLayout->setSpacing(7); 
  _buttonsLayout->setSpacing(5); 
  _listLayout->setSpacing(0); 

  QLabel *_aliasLit = new QLabel(tr("&Alias:"), this);
  _aliasLit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _aliasLit->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  _itemLayout->addWidget(_aliasLit);

  _alias = new QLineEdit(this);
  _alias->setObjectName("_alias");
  _alias->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  _aliasLit->setBuddy(_alias);
  _itemLayout->addWidget(_alias);
  _line1Layout->addLayout(_itemLayout);

  _showInactive = new QCheckBox(tr("Show &Inactive Items"), this);
  _line1Layout->addWidget(_showInactive);
  _enterLayout->addLayout(_line1Layout);

  QSpacerItem* spacer = new QSpacerItem( 20, 32, QSizePolicy::Minimum, QSizePolicy::Fixed );
  _enterLayout->addItem( spacer );
  _topLayout->addLayout(_enterLayout);

  _close = new QPushButton(tr("&Cancel"), this);
  _buttonsLayout->addWidget(_close);

  _select = new QPushButton(tr("&Select"), this);
  _select->setEnabled( FALSE );
  _select->setAutoDefault( TRUE );
  _select->setDefault( TRUE );
  _buttonsLayout->addWidget(_select);

  _topLayout->addLayout(_buttonsLayout);
  _mainLayout->addLayout(_topLayout);

  QLabel *_itemsLit = new QLabel(tr("&Items:"), this);
  _listLayout->addWidget(_itemsLit);

  _item = new XTreeWidget(this);
  _item->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  _itemsLit->setBuddy(_item);
  _listLayout->addWidget(_item);
  _mainLayout->addLayout(_listLayout);

  connect( _item, SIGNAL( itemSelected(int) ), this, SLOT( sSelect() ) );
  connect( _select, SIGNAL( clicked() ), this, SLOT( sSelect() ) );
  connect( _alias, SIGNAL( lostFocus() ), this, SLOT( sFillList() ) );
  connect( _item, SIGNAL( valid(bool) ), _select, SLOT( setEnabled(bool) ) );
  connect( _close, SIGNAL( clicked() ), this, SLOT( reject() ) );

  _useQuery = FALSE;

  _item->addColumn(tr("Alias Number"),100, Qt::AlignLeft, true, "itemalias_number");
  _item->addColumn(tr("Item Number"), 100, Qt::AlignLeft, true, "item_number");
  _item->addColumn(tr("Description"),  -1, Qt::AlignLeft, true, "item_descrip");
}

void itemAliasList::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("itemType", &valid);
  if (valid)
  {
    _itemType = param.toUInt();
    setWindowTitle(buildItemLineEditTitle(_itemType, tr("Item Aliases")));
  }
  else
  {
    _itemType = ItemLineEdit::cUndefined;
    setWindowTitle(tr("Item Aliases"));
  }

  param = pParams.value("extraClauses", &valid);
  if (valid)
    _extraClauses = param.toStringList();

  _showInactive->setChecked(FALSE);
  _showInactive->setEnabled(!(_itemType & ItemLineEdit::cActive));

  param = pParams.value("sql", &valid);
  if (valid)
    _sql = param.toString();
}

void itemAliasList::sSelect()
{
  done(_item->altId());
}

void itemAliasList::sFillList()
{
  _item->clear();

  if (_alias->text().trimmed().length() == 0)
    return;

  QString pre( "SELECT item_id, itemalias_id, itemalias_number, item_number, (item_descrip1 || ' ' || item_descrip2) AS item_descrip "
               "FROM (SELECT DISTINCT item_id, item_number, item_descrip1, item_descrip2");
  QString post(") AS data, itemalias "
               "WHERE ( (itemalias_item_id=item_id)"
               " AND (UPPER(itemalias_number)~UPPER(:searchString)) )" );

  if(_x_preferences && _x_preferences->boolean("ListNumericItemNumbersFirst"))
    post += " ORDER BY toNumeric(item_number, 999999999999999), item_number";
  else
    post += " ORDER BY item_number";

  QStringList clauses;
  clauses = _extraClauses;
  if ( !(_itemType & ItemLineEdit::cActive) && !_showInactive->isChecked())
    clauses << "(item_active)";

  XSqlQuery alias;
  alias.prepare(buildItemLineEditQuery(pre, clauses, post, _itemType));
  alias.bindValue(":searchString", _alias->text().trimmed());
  alias.exec();
  if (alias.first())
  {
    if (_useQuery)
    {
      XSqlQuery item(_sql);
      if (item.first())
      {
          XTreeWidgetItem *last = NULL;
          do
          {
            if (item.findFirst("item_id", alias.value("item_id").toInt()) != -1)
              last = new XTreeWidgetItem( _item, last, alias.value("item_id").toInt(), alias.value("itemalias_id").toInt(),
                                        alias.value("itemalias_number").toString(), alias.value("item_number").toString(), alias.value("item_descrip").toString() );
          }
          while (alias.next());
      }
    }
    else
      _item->populate(alias, TRUE);
  }
}
