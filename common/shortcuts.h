/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#ifndef shortcuts_h
#define shortcuts_h

#include <QObject>
#include <QWidget>
#include <QTabWidget>

class TabWidgetNavigtor : public QObject
{
  Q_OBJECT

public:
  TabWidgetNavigtor(QTabWidget* tabWidget, QObject* parent = 0);

public slots:
  void pageDown();
  void pageUp();

private:
  QTabWidget* _tab;
};

class shortcuts
{
public:
  static void setStandardKeys(QWidget* widget);
};


#endif

