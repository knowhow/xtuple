/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xitemdelegate.h"

#include <QCheckBox>
#include <QDesktopWidget>
#include <QDialog>
#include <QMainWindow>
#include <QStandardItemEditorCreator>
#include <QSqlField>

#include "datecluster.h"
#include "xsqltablemodel.h"

#define DEBUG true

XItemDelegate::XItemDelegate(QObject *parent)
  : QItemDelegate(parent)
{
  QItemEditorFactory *editorfactory = new QItemEditorFactory();
  if (editorfactory)
  {
    editorfactory->registerEditor(QVariant::Date,
                                  new QStandardItemEditorCreator<DLineEdit>());
    setItemEditorFactory(editorfactory);

    editorfactory->registerEditor(QVariant::Bool,
                                  new QStandardItemEditorCreator<QCheckBox>());
    setItemEditorFactory(editorfactory);
  }
}

XItemDelegate::~XItemDelegate()
{
}

