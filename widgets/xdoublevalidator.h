/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __XDOUBLEVALIDATOR_H__
#define __XDOUBLEVALIDATOR_H__

#include <QDoubleValidator>

class XDoubleValidator: public QDoubleValidator
{
  Q_OBJECT

  public:
    XDoubleValidator(QObject * parent);
    XDoubleValidator(double bottom, double top, int decimals, QObject * parent);

    QValidator::State validate(QString &, int &) const;
};

#endif

