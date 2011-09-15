/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "getLotInfo.h"

#include <QVariant>
#include <QMessageBox>

getLotInfo::getLotInfo(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_assign, SIGNAL(clicked()), this, SLOT(sAssign()));
}

getLotInfo::~getLotInfo()
{
  // no need to delete child widgets, Qt does it all for us
}

void getLotInfo::languageChange()
{
  retranslateUi(this);
}

QString getLotInfo::lot()
{
  return _lot->text();
}

QDate getLotInfo::expiration()
{
  if(_expiration->isEnabled())
    return _expiration->date();
  else
    return omfgThis->endOfTime();
}

QDate getLotInfo::warranty()
{
  if(_warranty->isEnabled())
    return _warranty->date();
  else
    return QDate();
}

void getLotInfo::enableExpiration( bool yes )
{
  _expiration->setEnabled(yes);
}

void getLotInfo::enableWarranty( bool yes )
{
  _warranty->setEnabled(yes);
}

void getLotInfo::sAssign()
{
  if (_lot->text().isEmpty())
  {
    QMessageBox::critical( this, tr("Enter A Lot Number"),
                           tr("You must specifiy a Lot Number.") );
    _lot->setFocus();
    return;
  }
  
  if ( (_expiration->isEnabled()) && (!_expiration->isValid()) )
  {
    QMessageBox::critical( this, tr("Enter Expiration Date"),
                           tr("You must enter an expiration date to this Perishable Lot/Serial number.") );
    _expiration->setFocus();
    return;
  }
  
  if ( (_warranty->isEnabled()) && (!_warranty->isValid()) )
  {
    QMessageBox::critical( this, tr("Enter Warranty Date"),
                           tr("You must enter an warranty date to this Lot/Serial number.") );
    _expiration->setFocus();
    return;
  }
  
  accept();
}

