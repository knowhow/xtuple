/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#include <QAction>
#include <QDialogButtonBox>
#include <QPushButton>

#include "shortcuts.h"

TabWidgetNavigtor::TabWidgetNavigtor(QTabWidget* tabWidget, QObject* parent)  :
    QObject(parent)
{
  _tab = tabWidget;
}

void TabWidgetNavigtor::pageDown()
{
  int curridx = _tab->currentIndex();
  if (curridx < _tab->count())
    _tab->setCurrentIndex(curridx + 1);
}

void TabWidgetNavigtor::pageUp()
{
  int curridx = _tab->currentIndex();
  if (curridx)
    _tab->setCurrentIndex(curridx - 1);
}

////////////////////////////////

void shortcuts::setStandardKeys(QWidget* widget)
{
  if (!widget)
    return;

  // Add standard shortcuts to applicable buttons
  bool hasShortcut = false;
  QPushButton* button;

  // For Save
  button = widget->findChild<QPushButton*>("_save");
  if (button)
  {
    button->setShortcut(QKeySequence::Save);
    button->setToolTip(button->text().remove("&")  + " " +
                       button->shortcut().toString(QKeySequence::NativeText));
    hasShortcut = true;
  }
  if (!hasShortcut) // Because some screens have both
  {
    // For Post
    button = widget->findChild<QPushButton*>("_post");
    if (button)
    {
      button->setShortcut(QKeySequence::Save);
      button->setToolTip(button->text().remove("&")  + " " +
                         button->shortcut().toString(QKeySequence::NativeText));
    }
  }
  if (!hasShortcut)
  {
    QDialogButtonBox* bb = widget->findChild<QDialogButtonBox*>();
    if (bb)
    {
      QList<QAbstractButton*> buttons =  bb->buttons();
      for (int i = 0; i < buttons.size(); ++i)
      {
        QAbstractButton *bbutton = buttons.at(i);
        QDialogButtonBox::ButtonRole btnrole = bb->buttonRole(buttons.at(i));
        if (btnrole == QDialogButtonBox::AcceptRole)
        {
          bbutton->setShortcut(QKeySequence::Save);
          bbutton->setToolTip(bbutton->text().remove("&")  + " " +
                              bbutton->shortcut().toString(QKeySequence::NativeText));

        }
        else if (btnrole == QDialogButtonBox::RejectRole)
        {
          bbutton->setShortcut(QKeySequence::Close);
          bbutton->setToolTip(bbutton->text().remove("&")  + " " +
                              bbutton->shortcut().toString(QKeySequence::NativeText));

        }
      }
    }
  }

  // For Close
  hasShortcut = false;
  button = widget->findChild<QPushButton*>("_close");
  if (button)
  {
    button->setShortcut(QKeySequence::Close);
    button->setToolTip(button->text().remove("&")  + " " +
                       button->shortcut().toString(QKeySequence::NativeText));
    hasShortcut = true;
  }
  if (!hasShortcut) // Because some screens have both
  {
    // For Post
    button = widget->findChild<QPushButton*>("_cancel");
    if (button)
    {
      button->setShortcut(QKeySequence::Close);
      button->setToolTip(button->text().remove("&")  + " " +
                         button->shortcut().toString(QKeySequence::NativeText));
    }
  }

  // For New
  button = widget->findChild<QPushButton*>("_new");
  if (button)
  {
    button->setShortcut(QKeySequence::New);
    button->setToolTip(button->text().remove("&")  + " " +
                       button->shortcut().toString(QKeySequence::NativeText));
    hasShortcut = true;
  }

  // For Print
  button = widget->findChild<QPushButton*>("_print");
  if (button)
  {
    button->setShortcut(QKeySequence::Print);
    button->setToolTip(button->text().remove("&")  + " " +
                       button->shortcut().toString(QKeySequence::NativeText));
    hasShortcut = true;
  }

  // For Query
  button = widget->findChild<QPushButton*>("_query");
  if (button)
  {
    button->setShortcut(QKeySequence::Refresh);
    button->setToolTip(button->text().remove("&")  + " " +
                       button->shortcut().toString(QKeySequence::NativeText));
    hasShortcut = true;
  }

  // Page up/down for tab widgets
  QTabWidget* tab = widget->findChild<QTabWidget*>();
  if (tab)
  {
    TabWidgetNavigtor* tabNav = new TabWidgetNavigtor(tab, widget);
    QAction* pagedownAct = new QAction(tab);
    pagedownAct->setShortcut(QKeySequence::MoveToNextPage);
    pagedownAct->connect(pagedownAct, SIGNAL(triggered()), tabNav, SLOT(pageDown()));
    tab->addAction(pagedownAct);

    QAction* pageupAct = new QAction(tab);
    pageupAct->setShortcut(QKeySequence::MoveToPreviousPage);
    pageupAct->connect(pageupAct, SIGNAL(triggered()), tabNav, SLOT(pageUp()));
    tab->addAction(pageupAct);
  }
}
