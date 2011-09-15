/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef QUERYITEM_H
#define QUERYITEM_H

#include "widgets.h"

#include <mqledit.h>
#include <parameteredit.h>
#include <selectmql.h>

#include "ui_queryitem.h"

class QueryItem : public QWidget, public Ui::QueryItem
{
    Q_OBJECT

  public:
    QueryItem(QWidget* parent = 0, Qt::WindowFlags fl = 0);
    ~QueryItem();

    Q_INVOKABLE virtual int id()       const;
    Q_INVOKABLE virtual int parentId() const;

  public slots:
    virtual void clear();
    virtual void setId(int p);
    virtual void setParentId(int p);

  signals:
    void saved(int queryitemid);
    void closed(bool ok);

  protected slots:
    virtual void languageChange();
    virtual void sHandleButtons();
    virtual void sSave();
    virtual void sUpdateRelations();

  protected:
    virtual void closeEvent(QCloseEvent *event);

    MQLEdit   *_mqledit;
    int        _qryheadid;
    int        _qryitemid;
    SelectMQL *_selectmql;

};

#endif // QUERYITEM_H
