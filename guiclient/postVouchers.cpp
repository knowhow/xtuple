/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "postVouchers.h"

#include <QMessageBox>
#include <QSqlError>

#include <openreports.h>

postVouchers::postVouchers(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));

  if (_preferences->boolean("XCheckBox/forgetful"))
    _printJournal->setChecked(true);

  _post->setFocus();
}

postVouchers::~postVouchers()
{
    // no need to delete child widgets, Qt does it all for us
}

void postVouchers::languageChange()
{
    retranslateUi(this);
}

void postVouchers::sPost()
{
  q.prepare("SELECT count(*) AS unposted FROM vohead WHERE (NOT vohead_posted)");
  q.exec();
  if(q.first() && q.value("unposted").toInt()==0)
  {
    QMessageBox::critical( this, tr("No Vouchers to Post"),
                           tr("There are no Vouchers to post.") );
    return;
  }

  q.prepare("SELECT postVouchers(FALSE) AS result;");
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result == -5)
    {
      QMessageBox::critical( this, tr("Cannot Post Voucher"),
                             tr( "The Cost Category(s) for one or more Item Sites for the Purchase Order covered by one or more\n"
                                 "of the Vouchers that you are trying to post is not configured with Purchase Price Variance or\n"
                                 "P/O Liability Clearing Account Numbers or the Vendor of these Vouchers is not configured with an\n"
                                 "A/P Account Number.  Because of this, G/L Transactions cannot be posted for these Vouchers.\n"
                                 "You must contact your Systems Administrator to have this corrected before you may\n"
                                 "post Vouchers." ) );
      return;
    }
    else if (result < 0)
    {
      systemError( this, tr("A System Error occurred at %1::%2, Error #%3.")
                         .arg(__FILE__)
                         .arg(__LINE__)
                         .arg(result) );
      return;
    }

    omfgThis->sVouchersUpdated();
 
    if (_printJournal->isChecked())
    {
      ParameterList params;
      params.append("journalNumber", result);

      orReport report("PayablesJournal", params);
      if (report.isValid())
        report.print();
      else
        report.reportError(this);
    }
  }
  else
  {
    systemError( this, tr("A System Error occurred at %1::%2.\n%3")
                       .arg(__FILE__)
                       .arg(__LINE__)
                       .arg(q.lastError().databaseText()) );
    return;
  }

  accept();
}
