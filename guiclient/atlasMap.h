/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ATLASMAP_H
#define ATLASMAP_H

#include "guiclient.h"
#include "xdialog.h"
#include "ui_atlasMap.h"

#include <parameter.h>

class QAbstractMessageHandler;

class atlasMap : public XDialog, public Ui::atlasMap
{
    Q_OBJECT

  public:
    atlasMap(QWidget* parent = 0, const char * = 0, Qt::WFlags fl = 0);
    ~atlasMap();

    static bool userHasPriv();

  public slots:
    virtual int              exec();
    virtual enum SetResponse set(const ParameterList&);
    virtual void             sPopulate();

  protected slots:
    virtual void languageChange();

    virtual void sHandleAtlas();
    virtual void sSave();

  private:
    int		_mode;
    int		_atlasmapId;
    QString	_defaultDir;
    QAbstractMessageHandler *_msghandler;

};

#endif // ATLASMAP_H
