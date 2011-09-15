/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "idleShutdown.h"

#include <QVariant>
#include <QTimerEvent>

idleShutdown::idleShutdown(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _secsRemaining = 60;
  _countMessage = _count->text();
  _count->setText(_countMessage.arg(_secsRemaining));

  startTimer(1000);
}

idleShutdown::~idleShutdown()
{
  // no need to delete child widgets, Qt does it all for us
}

void idleShutdown::languageChange()
{
  retranslateUi(this);
}

enum SetResponse idleShutdown::set(ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("minutes", &valid);
  if (valid)
    _message->setText(_message->text().arg(param.toInt()));

  return NoError;
}

void idleShutdown::timerEvent(QTimerEvent*)
{
  _secsRemaining--;

  if (_secsRemaining > 1)
    _count->setText(_countMessage.arg(_secsRemaining));
  else
    accept();
}

