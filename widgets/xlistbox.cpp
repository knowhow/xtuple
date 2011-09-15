/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xlistbox.h"

XListBoxText::XListBoxText(QListWidget* listbox, const QString &text, int id)
  : QListWidgetItem(text, listbox)
{
  _id = id;
}

XListBoxText::XListBoxText(const QString & text, int id)
  : QListWidgetItem(text)
{
  _id = id;
}

XListBoxText::XListBoxText(QListWidget* listbox, const QString &text, QListWidgetItem *after)
  : QListWidgetItem(text, listbox)
{
  _id = -1;
  listbox->insertItem(listbox->row(after) + 1, this);
}

XListBoxText::XListBoxText(QListWidget* listbox, const QString & text, int id, QListWidgetItem *after)
  : QListWidgetItem(text, listbox)
{
  _id = id;
  listbox->insertItem(listbox->row(after) + 1, this);
}

XListBoxText::~XListBoxText()
{
}

void XListBoxText::setId(int id)
{
  _id = id;
}

int XListBoxText::RTTI = 8152005;
int XListBoxText::rtti() const
{
  return RTTI;
}
