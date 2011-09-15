/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "failedPostList.h"

failedPostList::failedPostList(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);
}

failedPostList::~failedPostList()
{
    // no need to delete child widgets, Qt does it all for us
}

void failedPostList::languageChange()
{
    retranslateUi(this);
}

void failedPostList::setLabel(const QString& pText)
{
  _label->setText(pText);
}

void failedPostList::sSetList(QList<XTreeWidgetItem*> itemList, QTreeWidgetItem* hitem, QHeaderView* header)
{
  if (itemList.size() > 0)
  {
    for (int j = 0; j < hitem->columnCount(); j++)
    {
      _doclist->addColumn(hitem->text(j), header->sectionSize(j), hitem->textAlignment(j));

    }

    //_doclist->addTopLevelItems(itemList);
    for (int i = 0; i < itemList.size(); i++)
    {
      XTreeWidgetItem* tmp = (XTreeWidgetItem*)(itemList[i]->clone());
      _doclist->addTopLevelItem(tmp);
    }
  }
}
