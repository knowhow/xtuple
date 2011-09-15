/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __ITEMCHARACTERISTICDELEGATE_H__
#define __ITEMCHARACTERISTICDELEGATE_H__

#include <QItemDelegate>
#include <QModelIndex>
#include <QObject>
#include <QDate>

class ItemCharacteristicDelegate : public QItemDelegate
{
  Q_OBJECT

  public:
    ItemCharacteristicDelegate(QObject * parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const;
    
  private:
    enum {
      CHAR  = 0,
      VALUE = 1,
      PRICE = 2
    };
         
    enum {
      ITEM_ID   = 0,
      CUST_ID   = 1,
      SHIPTO_ID = 2,
      QTY       = 3,
      CURR_ID   = 4,
      EFFECTIVE = 5
    };
  
};

#endif

