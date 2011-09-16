/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef SETUP_H
#define SETUP_H

#include "guiclient.h"
#include "xdialog.h"
#include "xt.h"
#include <parameter.h>

#include "ui_setup.h"

void setupSetupApi(QScriptEngine* engine);

class setup : public XDialog, public Ui::setup
{
    Q_OBJECT

public:
    setup(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~setup();

    enum SetupTypes
    {
      Configure, AccountMapping, MasterInformation
    };

public slots:
    enum SetResponse set(const ParameterList & pParams );
    void insert(const QString &title,
                const QString &uiName,
                int type,
                int modules,
                bool enabled = true,
                int mode = 0,
                const QString &saveMethod = QString());
    void apply();
    void languageChange();
    int mode(const QString &editPriv, const QString &viewPriv = QString());
    void populate(bool first = true);
    void save(bool close = true);
    void setCurrentIndex(const QString &uiName);

signals:
    void saving();

private slots:
    void setCurrentIndex(XTreeWidgetItem* item);

private:
    struct ItemProps {
      QString title;
      QString uiName;
      int type;
      int modules;
      bool enabled;
      int mode;
      QString saveMethod;
      int       index;  // in the stack widget
      int       id;     // for the tree widget
      QObject  *implementation;
    };
    QMap<QString, ItemProps>    _itemMap;

    XTreeWidgetItem*            _configItem;
    XTreeWidgetItem*            _mapItem;
    XTreeWidgetItem*            _masterItem;
};

Q_DECLARE_METATYPE(enum setup::SetupTypes);

#endif // SETUP_H
