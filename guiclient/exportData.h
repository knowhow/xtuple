/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef EXPORTDATA_H
#define EXPORTDATA_H

#include "xwidget.h"

#include <QMenu>

#include <parameteredit.h>

#include "ui_exportData.h"

#include "queryset.h"

class exportData : public XWidget, public Ui::exportData
{
  Q_OBJECT
  
  public:
    exportData(QWidget* = 0, const char * = 0, Qt::WindowFlags = 0);
    ~exportData();

    static bool userHasPriv(const int priv);
    Q_INVOKABLE ParameterEdit *getParameterEdit() const;

  public slots:

  protected slots:
    virtual void languageChange();
    virtual void sDeleteQuerySet();
    virtual void sEditQuerySet();
    virtual void sExport();
    virtual void sFillList();
    virtual void sHandleButtons();
    virtual void sNewQuerySet();

  protected:
    static QString exportFileDir;

           ParameterEdit *_paramedit;
           QuerySet      *_queryset;
};

#endif
