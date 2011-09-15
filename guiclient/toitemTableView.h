/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef TOITEMTABLEVIEW_H
#define TOITEMTABLEVIEW_H

#include <QItemDelegate>
#include <QList>
#include <QModelIndex>
#include <QTableView>

class ToitemTableView : public QTableView
{
  Q_OBJECT

  public:
    ToitemTableView(QWidget* = 0);
    virtual void	setModel(QAbstractItemModel* model);

  protected slots:
    virtual void	currentChanged(const QModelIndex &, const QModelIndex &) ;
    virtual void	sHandleCloseEditor(QWidget *);

  private:
    QList<int> _hideList;
};

// TODO: move the ToitemTableDelegate implemention into the declaration
class ToitemTableDelegate : public QItemDelegate
{
  Q_OBJECT

  public:
    ToitemTableDelegate(QObject *parent = 0);

    virtual QWidget	*createEditor(QWidget *, const QStyleOptionViewItem &, const QModelIndex &) const;
    virtual void	setEditorData(QWidget *, const QModelIndex &) const;
    virtual void	setModelData(QWidget *, QAbstractItemModel *, const QModelIndex &) const;
    virtual void	updateEditorGeometry(QWidget *, const QStyleOptionViewItem &, const QModelIndex &) const;

};

#endif // TOITEMTABLEVIEW_H
