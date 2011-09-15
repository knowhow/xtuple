/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef UPDATEPRICES_H
#define UPDATEPRICES_H

#include "xdialog.h"
#include "ui_updatePrices.h"

class updatePrices : public XDialog, public Ui::updatePrices
{
  Q_OBJECT

public:
  updatePrices(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
  ~updatePrices();

public slots:
  virtual void sUpdate();
  virtual void populate();
  virtual void sAdd();
  virtual void sAddAll();
  virtual void sRemove();
  virtual void sRemoveAll();
  virtual void sHandleBy(bool toggled);
  virtual void sHandleCharPrice();
  virtual void closeEvent( QCloseEvent * pEvent );

protected slots:
  virtual void languageChange();

};

#endif // UPDATEPRICES_H
