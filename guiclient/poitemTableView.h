/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef POITEMTABLEVIEW_H
#define POITEMTABLEVIEW_H

#include <QItemDelegate>
#include <QList>
#include <QModelIndex>
#include <QTableView>

// this has to be in the .h or MOC won't find it
class PoitemTableDelegate : public QItemDelegate
{
  Q_OBJECT

  public:
    PoitemTableDelegate(QObject *parent = 0);

    virtual QWidget *createEditor(QWidget *, const QStyleOptionViewItem &, const QModelIndex &) const;
    virtual void    setEditorData(QWidget *, const QModelIndex &) const;
    virtual void    setModelData(QWidget *, QAbstractItemModel *, const QModelIndex &) const;
    virtual void    updateEditorGeometry(QWidget *, const QStyleOptionViewItem &, const QModelIndex &) const;

};

class PoitemTableView : public QTableView
{
  Q_OBJECT

  public:
    PoitemTableView(QWidget* = 0);
    virtual void setModel(QAbstractItemModel* model);

  public slots:
    virtual void sHeaderDataChanged(Qt::Orientation, int, int);

  protected slots:
    virtual void currentChanged(const QModelIndex &, const QModelIndex &) ;
    virtual void closeEditor(QWidget *, QAbstractItemDelegate::EndEditHint);

  private:
    QList<int> _hideList;
};

#endif // POITEMTABLEVIEW_H
