/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xuiloader.h"

#include <QPluginLoader>
#include <QStackedWidget>
#include <QTabWidget>
#include <QDialog>
#include <QToolBox>

XUiLoader::XUiLoader(QObject* parent)
  : QUiLoader(parent)
{
  QObjectList instances = QPluginLoader::staticInstances();
  for (int i = 0; i < instances.count(); ++i)
  {
    // step 1) try with a normal plugin
    QDesignerCustomWidgetInterface *iface = qobject_cast<QDesignerCustomWidgetInterface *>(instances.at(i));
    if (iface != 0) {
      m_customWidgets.insert(iface->name(), iface);
      continue;
    }

    // step 2) try with a collection of plugins
    QDesignerCustomWidgetCollectionInterface *c = qobject_cast<QDesignerCustomWidgetCollectionInterface *>(instances.at(i));
    if (c != 0) {
      foreach (QDesignerCustomWidgetInterface *iface, c->customWidgets()) {
        m_customWidgets.insert(iface->name(), iface);
      }
    }
  }
}

XUiLoader::~XUiLoader()
{
}

QWidget * XUiLoader::createWidget ( const QString & className, QWidget * parent, const QString & name )
{
  QWidget *w = 0;

  if (qobject_cast<QTabWidget*>(parent)
        || qobject_cast<QStackedWidget*>(parent)
        || qobject_cast<QToolBox*>(parent))
    parent = 0;

  QDesignerCustomWidgetInterface *factory = m_customWidgets.value(className);
  if (factory != 0)
    w = factory->createWidget(parent);

  if(w)
  {
    w->setObjectName(name);

    if (qobject_cast<QDialog *>(w))
      w->setParent(parent);

    return w;
  }

  return QUiLoader::createWidget(className, parent, name);
}

