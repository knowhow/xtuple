/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef xtupleplugin_h
#define xtupleplugin_h

#include "widgets.h"

#include <QtDesigner/QtDesigner>

class xTuplePlugin : public QObject, public QDesignerCustomWidgetCollectionInterface
{
  Q_OBJECT
  Q_INTERFACES(QDesignerCustomWidgetCollectionInterface)

  public:
    xTuplePlugin(QObject *parent = 0);

    virtual QList<QDesignerCustomWidgetInterface*> customWidgets() const;

  private:
    QList<QDesignerCustomWidgetInterface*> m_plugins;
};

void XTUPLEWIDGETS_EXPORT initializePlugin(Preferences *, Metrics *, Privileges *, QString, QWorkspace *);

#endif
