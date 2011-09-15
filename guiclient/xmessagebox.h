/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __XMESSAGEBOX_H__
#define __XMESSAGEBOX_H__

#include <QMessageBox>

class XMessageBox {
  public:
    static int message( QWidget * parent, QMessageBox::Icon severity,
                        const QString & caption, const QString & text,
                        const QString & button0Text = QString::null,
                        const QString & button1Text = QString::null,
                        bool snooze = true,
                        int defaultButtonNumber = 0, int escapeButtonNumber = -1 );

  private:
    XMessageBox();
};

#endif

