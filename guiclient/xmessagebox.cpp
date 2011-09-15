/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xmessagebox.h"

#include <QApplication>
#include <QPushButton>

/** \brief XMessageBox should be used for cases where the user might
           be in a loop that has a lot of dialogs and the application
           needs to get the user's attention.

           For example, if there is a loop where the user needs to enter
           a series of values into individual dialogs and an error occurs,
           the XMessageBox might make sense to display the error. That's because
           the default button causes the XMessageBox to repeat the error dialog.

           Other good uses are places where the user is scanning a series of barcodes.
  */

int XMessageBox::message( QWidget * parent, QMessageBox::Icon severity,
                          const QString & caption, const QString & text,
                          const QString & button0Text,
                          const QString & button1Text,
                          bool snooze, int defaultButtonNumber, int escapeButtonNumber )
{
  QPushButton *button0   = 0;
  QPushButton *button1   = 0;
  QPushButton *buttonsnooze = 0;

  QMessageBox *mb = new QMessageBox(severity, caption, text, QMessageBox::NoButton, parent);
  mb->setObjectName("xtuple_msgbox_snooze");

  Q_CHECK_PTR( mb );

  if (button0Text.isEmpty())
  {
    button0 = mb->addButton(QMessageBox::Ok);
    defaultButtonNumber = snooze ? -1 : 0;
  }
  else
    button0 = mb->addButton(button0Text,
                            defaultButtonNumber == 0 ? QMessageBox::AcceptRole : QMessageBox::RejectRole);

  if (!button1Text.isEmpty())
    button1 = mb->addButton(button1Text,
                            defaultButtonNumber == 1 ? QMessageBox::AcceptRole : QMessageBox::RejectRole);

  if ( snooze )
    buttonsnooze = mb->addButton(QObject::tr("Snooze"), QMessageBox::RejectRole);

  if (defaultButtonNumber == 0)
    mb->setDefaultButton(button0);
  else if (defaultButtonNumber == 1)
    mb->setDefaultButton(button1);
  else if (snooze)
    mb->setDefaultButton(buttonsnooze);

  if (escapeButtonNumber == 0)
    mb->setEscapeButton(button0);
  else if (escapeButtonNumber == 1)
    mb->setEscapeButton(button1);
  else if (snooze)
    mb->setEscapeButton(buttonsnooze);

#ifndef QT_NO_CURSOR
  mb->setCursor(QCursor(Qt::ArrowCursor));
#endif
  int result = -1;
  do {
    QApplication::beep();
    result = mb->exec();
  } while (snooze && mb->clickedButton() == buttonsnooze);

  delete mb;

  return result - 1;
}

