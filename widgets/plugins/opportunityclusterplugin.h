/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __OPPORTUNITYCLUSTERPLUGIN_H__
#define __OPPORTUNITYCLUSTERPLUGIN_H__

#include "opportunitycluster.h"

#include <QDesignerCustomWidgetInterface>
#include <QtPlugin>

class OpportunityClusterPlugin : public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

  public:
    OpportunityClusterPlugin(QObject *parent = 0) : QObject(parent), initialized(false) {}

    bool isContainer() const { return false; }
    bool isInitialized() const { return initialized; }
    QIcon icon() const { return QIcon(); }
    QString domXml() const
    {
      return "<widget class=\"OpportunityCluster\" name=\"opportunityCluster\">\n"
             "</widget>\n";
    }
    QString group() const { return "xTuple Custom Widgets"; }
    QString includeFile() const { return "opportunitycluster.h"; }
    QString name() const { return "OpportunityCluster"; }
    QString toolTip() const { return ""; }
    QString whatsThis() const { return ""; }
    QWidget *createWidget(QWidget *parent) { return new OpportunityCluster(parent); }
    void initialize(QDesignerFormEditorInterface *) { initialized = true; }

  private:
    bool initialized;
};

#endif
