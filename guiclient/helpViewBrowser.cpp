/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QWidget>
#include <QVariant>

#include <QDebug>

#include "helpViewBrowser.h"
#include "xtHelp.h"

#define DEBUG false

helpViewBrowser::helpViewBrowser(QWidget *parent)
  : QTextBrowser(parent)
{
  setSource(xtHelp::getInstance()->homePage());
}

QVariant helpViewBrowser::loadResource(int type, const QUrl &name)
{
  if (DEBUG) qDebug() << "loadResource: type [" << type << "] name [" << name.toString() << "]";
  QByteArray data;
  if(type < 4)
  {
    QUrl url(name);
    if(name.isRelative())
    {
      url = source().resolved(url);
      if (DEBUG)
        qDebug() << "url was relative [" << url << "]";
    }
    data = xtHelp::getInstance()->fileData(url);
  }
  return data;
}

helpViewBrowser::~helpViewBrowser()
{
}
