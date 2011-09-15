/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef IMPORTDATA_H
#define IMPORTDATA_H

#include <QDomDocument>
#include "xwidget.h"
#include <QMenu>

#include "ui_importData.h"

class importData : public XWidget, public Ui::importData
{
  Q_OBJECT
  
  public:
    importData(QWidget* = 0, const char * = 0, Qt::WindowFlags = 0);
    ~importData();

    static bool userHasPriv();

  public slots:
    virtual void setVisible(bool);

  protected slots:
    virtual void languageChange();
    virtual void sAdd();
    virtual void sClearStatus();
    virtual void sDelete();
    virtual void sFillList();
    virtual void sHandleAutoUpdate(const bool);
    virtual void sImportAll();
    virtual void sImportSelected();
    virtual void sPopulateMenu(QMenu*, QTreeWidgetItem*);

  private:
    QString	_defaultDir;
    bool	importOne(const QString &, const int pType = -1);
};

#endif
