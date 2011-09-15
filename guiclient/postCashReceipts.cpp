/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "postCashReceipts.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>
#include <QStatusBar>
#include <openreports.h>
#include <parameter.h>
#include "guiclient.h"

/*
 *  Constructs a postCashReceipts as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
postCashReceipts::postCashReceipts(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));

  _post->setFocus();
}

/*
 *  Destroys the object and frees any allocated resources
 */
postCashReceipts::~postCashReceipts()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void postCashReceipts::languageChange()
{
    retranslateUi(this);
}

void postCashReceipts::sPost()
{
  int journalNumber;

  q.exec("SELECT fetchJournalNumber('C/R') AS journalnumber;");
  if (q.first())
    journalNumber = q.value("journalnumber").toInt();
  else
  {
    systemError(this, tr("A System Error occurred at %1::%2.")
                      .arg(__FILE__)
                      .arg(__LINE__) );
    return;
  }

  q.exec( "SELECT cashrcpt_id, cust_number "
          "FROM cashrcpt, cust "
          "WHERE ( (NOT cashrcpt_posted)"
          "  AND   (NOT cashrcpt_void)"
          "  AND   (cashrcpt_cust_id=cust_id) );" );
  if (q.first())
  {
    int counter = 0;

    XSqlQuery post;
    post.prepare("SELECT postCashReceipt(:cashrcpt_id, :journalNumber) AS result;");
    
    do
    {
      message( tr("Posting Cash Receipt #%1...")
               .arg(q.value("cust_number").toString()) );

      post.bindValue(":cashrcpt_id", q.value("cashrcpt_id"));
      post.bindValue(":journalNumber", journalNumber);
      post.exec();
      if (post.first())
      {
        switch (post.value("result").toInt())
        {
          case -1:
            QMessageBox::critical( this, tr("Cannot Post Cash Receipt"),
                                   tr( "The selected Cash Receipt cannot be posted as the amount distributed is greater than\n"
                                       "the amount received. You must correct this before you may post this Cash Receipt." ) );
            break;

          case -5:
            QMessageBox::critical( this, tr("Cannot Post Cash Receipt"),
                                   tr( "A Cash Receipt for Customer #%1 cannot be posted as the A/R Account cannot be determined.\n"
                                       "You must make a A/R Account Assignment for the Customer Type to which this Customer\n"
                                       "is assigned for you may post this Cash Receipt." )
                                   .arg(q.value("cust_number").toString())  );
            break;

          case -6:
            QMessageBox::critical( this, tr("Cannot Post Cash Receipt"),
                                   tr( "A Cash Receipt for Customer #%1 cannot be posted as the Bank Account cannot be determined.\n"
                                       "You must make a Bank Account Assignment for this Cash Receipt before you may post it." )
                                   .arg(q.value("cust_number").toString())  );
            break;

          case -7:
            QMessageBox::critical( this, tr("Cannot Post Cash Receipt"),
                                   tr( "A Cash Receipt for Customer #%1 cannot be posted due to an unknown error.\n"
                                       "Contact you Systems Administrator." )
                                   .arg(q.value("cust_number").toString())  );

          default:
            counter++;
        }
      }
      else
      {
        systemError( this, tr("While posting the cash receipt for Customer %1:"
			      "\n\n%2")
                           .arg(q.value("cust_number").toString())
                           .arg(post.lastError().databaseText()),
		     __FILE__, __LINE__ );
        break;
      }
    }
    while (q.next());

    resetMessage();

    if ( (counter) && (_printJournal->isChecked()) )
    {
      ParameterList params;
      params.append("journalNumber", journalNumber);
     
      orReport report("CashReceiptsJournal", params);
      if (report.isValid())
        report.print();
      else
        report.reportError(this);
    }

    omfgThis->sCashReceiptsUpdated(-1, true);

    accept();
  }
  else
  {
    QMessageBox::information( this, tr("No Unposted Cash Receipts"),
                              tr("There are no unposted Cash Receipts to post.") );
  }
}
