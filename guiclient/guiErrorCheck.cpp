/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "guiErrorCheck.h"

#include <QList>
#include <QMessageBox>
#include <QWidget>

GuiErrorCheck::GuiErrorCheck(bool phasError, QWidget *pwidget, QString pmsg) :
  hasError(phasError),
  widget(pwidget),
  msg(pmsg)
{
  if (! widget)
  {
    msg = "Bug: GuiErrorCheck created with empty widget"; // no tr
    hasError = true;
  }
}

GuiErrorCheck::~GuiErrorCheck()
{
  // no need to delete child widgets, Qt does it all for us
}

bool GuiErrorCheck::reportErrors(QWidget *parent, QString title,
                                 QList<GuiErrorCheck> list)
{
  for (int i = 0; i < list.length(); i++)
  {
    if (list[i].hasError)
    {
      QMessageBox::critical(parent, title, QString("<p>%1").arg(list[i].msg));
      if (list[i].widget)
        list[i].widget->setFocus();
      return true;
    }
  }

  return false;
}
