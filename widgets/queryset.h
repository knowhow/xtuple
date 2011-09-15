/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef QUERYSET_H
#define QUERYSET_H

#include "widgets.h"

#include <mqledit.h>
#include <parameteredit.h>
#include <selectmql.h>

#include "ui_queryset.h"

#include "queryitem.h"

class QuerySet : public QWidget, public Ui::QuerySet
{
    Q_OBJECT

  public:
    QuerySet(QWidget* parent = 0, Qt::WindowFlags fl = 0);
    ~QuerySet();

    Q_INVOKABLE virtual int id()       const;

  public slots:
    virtual void setId(int p);

  signals:
    void closed(bool ok);
    void saved(int id);

  protected slots:
    virtual void languageChange();
    virtual void sDelete();
    virtual void sEdit();
    virtual void sFillList();
    virtual void sHandleButtons();
    virtual void sNew();
    virtual bool sSave(bool done = true);

  protected:
    MQLEdit   *_mqledit;
    int        _qryheadid;
    QueryItem *_itemdlg;
    SelectMQL *_selectmql;

};

Q_DECLARE_METATYPE(QuerySet*);

void setupQuerySet(QScriptEngine *engine);

#endif // QUERYSET_H
