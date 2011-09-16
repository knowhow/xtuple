/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printCheck.h"

#include <QCloseEvent>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QPrintDialog>
#include <QSqlError>

#include <metasql.h>
#include <orprerender.h>
#include <orprintrender.h>
#include <renderobjects.h>

#include "xtsettings.h"
#include "confirmAchOK.h"
#include "storedProcErrorLookup.h"

QString printCheck::eftFileDir = QString();

printCheck::printCheck(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_bankaccnt, SIGNAL(newID(int)), this, SLOT(sHandleBankAccount(int)));
  connect(_check,     SIGNAL(newID(int)), this, SLOT(sEnableCreateEFT()));
  connect(_createEFT, SIGNAL(clicked()),  this, SLOT(sCreateEFT()));
  connect(_print,     SIGNAL(clicked()),  this, SLOT(sPrint()));

  _captive = FALSE;
  _setCheckNumber = -1;

  _check->setAllowNull(TRUE);

  _bankaccnt->setType(XComboBox::APBankAccounts);

  _createEFT->setVisible(_metrics->boolean("ACHSupported") && _metrics->boolean("ACHEnabled"));
}

printCheck::~printCheck()
{
  // no need to delete child widgets, Qt does it all for us
}

void printCheck::languageChange()
{
  retranslateUi(this);
}

enum SetResponse printCheck::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("check_id", &valid);
  if (valid)
  {
    populate(param.toInt());
    _bankaccnt->setEnabled(FALSE);
    _check->setEnabled(FALSE);
  }

  return NoError;
}

void printCheck::sPrint()
{
  if(_setCheckNumber != -1 && _setCheckNumber != _nextCheckNum->text().toInt())
  {
    q.prepare("SELECT checkhead_id "
              "FROM checkhead "
              "WHERE ( (checkhead_bankaccnt_id=:bankaccnt_id) "
              "  AND   (checkhead_id <> :checkhead_id) "
              "  AND   (checkhead_number=:nextCheckNumber));");
    q.bindValue(":bankaccnt_id", _bankaccnt->id());
    q.bindValue(":checkhead_id", _check->id());
    q.bindValue(":nextCheckNumber", _nextCheckNum->text().toInt());
    q.exec();
    if (q.first())
    {
      QMessageBox::information( this, tr("Check Number Already Used"),
                    tr("<p>The selected Check Number has already been used.") );
      return;
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  if (_createEFT->isEnabled() &&
      QMessageBox::question(this, tr("Print Anyway?"),
                            tr("<p>The recipient of this check has been "
                               "configured for EFT transactions. Do you want "
                               "to print this check for them anyway?<p>If you "
                               "answer 'Yes' then a check will be printed. "
                               "If you say 'No' then you should click %1. ")
                            .arg(_createEFT->text()),
                            QMessageBox::Yes | QMessageBox::Default,
                            QMessageBox::No) == QMessageBox::No)
    return;
  q.prepare( "SELECT checkhead_printed, form_report_name AS report_name, bankaccnt_id "
             "FROM checkhead, bankaccnt, form "
             "WHERE ((checkhead_bankaccnt_id=bankaccnt_id)"
             "  AND  (bankaccnt_check_form_id=form_id)"
             "  AND  (checkhead_id=:checkhead_id) );" );
  q.bindValue(":checkhead_id", _check->id());
  q.exec();
  if (q.first())
  {
    if(q.value("checkhead_printed").toBool())
    {
      QMessageBox::information( this, tr("Check Already Printed"),
		    tr("<p>The selected Check has already been printed.") );
      return;
    }
    QString reportname = q.value("report_name").toString();

// get the report definition out of the database
// this should somehow be condensed into a common code call or something
// in the near future to facilitate easier conversion in other places
// of the application to use the new rendering engine directly
    XSqlQuery report;
    report.prepare( "SELECT report_source "
                    "  FROM report "
                    " WHERE (report_name=:report_name) "
                    "ORDER BY report_grade DESC LIMIT 1;" );
    report.bindValue(":report_name", reportname);
    report.exec();
    QDomDocument docReport;
    if (report.first())
    {
      QString errorMessage;
      int     errorLine;
  
      if (!docReport.setContent(report.value("report_source").toString(), &errorMessage, &errorLine))
      {
        systemError(this, errorMessage, __FILE__, __LINE__);
        return;
      }
    }
    else
    {
      systemError(this, report.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
// end getting the report definition out the database

    if(_setCheckNumber != -1 && _setCheckNumber != _nextCheckNum->text().toInt())
    {
      q.prepare("SELECT setNextCheckNumber(:bankaccnt_id, :nextCheckNumber) AS result;");
      q.bindValue(":bankaccnt_id", _bankaccnt->id());
      q.bindValue(":nextCheckNumber", _nextCheckNum->text().toInt());
      q.exec();
      if (q.first())
      {
        int result = q.value("result").toInt();
        if (result < 0)
        {
          systemError(this, storedProcErrorLookup("setNextCheckNumber", result),
                      __FILE__, __LINE__);
          return;
        }
      }
      else if (q.lastError().type() != QSqlError::NoError)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }

    q.prepare("UPDATE checkhead SET checkhead_number=fetchNextCheckNumber(checkhead_bankaccnt_id)"
              " WHERE(checkhead_id=:checkhead_id);");
    q.bindValue(":checkhead_id", _check->id());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    ParameterList params;

    params.append("checkhead_id", _check->id());
    params.append("apchk_id", _check->id());

// replace with new renderer code so we can get a page count
    ORPreRender pre;
    pre.setDom(docReport);
    pre.setParamList(params);
    ORODocument * doc = pre.generate();

    QPrinter printer(QPrinter::HighResolution);
    ORPrintRender render;
    render.setupPrinter(doc, &printer);

    QPrintDialog pd(&printer);
    pd.setMinMax(1, doc->pages());
    if(pd.exec() == XDialog::Accepted)
    {
      render.setPrinter(&printer);
      render.render(doc);
    }
    else
      return;

    int page_num = 1;
    while(page_num < doc->pages())
    {
      page_num++;

      XSqlQuery qq;
      qq.prepare("INSERT INTO checkhead"
                 "      (checkhead_recip_id, checkhead_recip_type,"
                 "       checkhead_bankaccnt_id, checkhead_printed,"
                 "       checkhead_checkdate, checkhead_number,"
                 "       checkhead_amount, checkhead_void,"
                 "       checkhead_misc,"
                 "       checkhead_for, checkhead_notes,"
                 "       checkhead_curr_id, checkhead_deleted) "
                 "SELECT checkhead_recip_id, checkhead_recip_type,"
                 "       checkhead_bankaccnt_id, TRUE,"
                 "       checkhead_checkdate, fetchNextCheckNumber(checkhead_bankaccnt_id),"
                 "       checkhead_amount, TRUE, TRUE,"
                 "       'Continuation of Check #'||checkhead_number,"
                 "       'Continuation of Check #'||checkhead_number,"
                 "       checkhead_curr_id, TRUE"
                 "  FROM checkhead"
                 " WHERE(checkhead_id=:checkhead_id);");
      qq.bindValue(":checkhead_id", _check->id());
      if(!qq.exec())
      {
        systemError(this, "Received error but will continue anyway:\n"+qq.lastError().databaseText(), __FILE__, __LINE__);
      }
    }

    omfgThis->sChecksUpdated(_bankaccnt->id(), _check->id(), TRUE);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else // join failed
  {
    QMessageBox::critical(this, tr("Cannot Print Check"),
                          tr("<p>The selected Check cannot be printed as the "
			     "Bank Account that it is to draw upon does not "
			     "have a valid Check Format assigned to it. "
			     "Please assign a valid Check Format to this Bank "
			     "Account before attempting to print this Check."));
    return;
  }

  if ( QMessageBox::question( this, tr("Check Printed"),
                             tr("Was the selected Check printed successfully?"),
			     QMessageBox::Yes,
			     QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
    markCheckAsPrinted(_check->id());
  else if ( QMessageBox::question(this, tr("Mark Check as Voided"),
                                  tr("<p>Would you like to mark the selected "
				     "Check as Void and create a replacement "
				     "check?"),
				   QMessageBox::Yes,
				   QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    q.prepare("SELECT voidCheck(:checkhead_id) AS result;");
    q.bindValue(":checkhead_id", _check->id());
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
	systemError(this, storedProcErrorLookup("voidCheck", result),
		    __FILE__, __LINE__);
	return;
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "SELECT checkhead_bankaccnt_id,"
	       "       replaceVoidedCheck(checkhead_id) AS result "
               "FROM checkhead "
               "WHERE (checkhead_id=:checkhead_id);" );
    q.bindValue(":checkhead_id", _check->id());
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
	systemError(this, storedProcErrorLookup("replaceVoidedCheck", result),
		    __FILE__, __LINE__);
	return;
      }
      omfgThis->sChecksUpdated(q.value("checkhead_bankaccnt_id").toInt(),
			       _check->id(), TRUE);

      sHandleBankAccount(_bankaccnt->id());
      _print->setFocus();
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void printCheck::sHandleBankAccount(int pBankaccntid)
{
  if (_bankaccnt->id() != -1)
  {
    XSqlQuery checkNumber;
    checkNumber.prepare( "SELECT bankaccnt_nextchknum, bankaccnt_ach_enabled "
                         "FROM bankaccnt "
                         "WHERE (bankaccnt_id=:bankaccnt_id);" );
    checkNumber.bindValue(":bankaccnt_id", _bankaccnt->id());
    checkNumber.exec();
    if (checkNumber.first())
    {
      _setCheckNumber = checkNumber.value("bankaccnt_nextchknum").toInt();
      _createEFT->setEnabled(checkNumber.value("bankaccnt_ach_enabled").toBool());
      _nextCheckNum->setText(checkNumber.value("bankaccnt_nextchknum").toString());
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
  {
    _setCheckNumber = -1;
    _nextCheckNum->clear();
  }

  q.prepare( "SELECT checkhead_id,"
	     "       (CASE WHEN(checkhead_number=-1) THEN TEXT('Unspecified')"
             "             ELSE TEXT(checkhead_number)"
             "        END || '-' || checkrecip_name) "
             "FROM checkhead LEFT OUTER JOIN "
	     "      checkrecip ON ((checkhead_recip_id=checkrecip_id)"
	     "                AND  (checkhead_recip_type=checkrecip_type))"
             "WHERE ((NOT checkhead_void)"
             "  AND  (NOT checkhead_printed)"
             "  AND  (NOT checkhead_posted)"
             "  AND  (checkhead_bankaccnt_id=:bankaccnt_id) ) "
             "ORDER BY checkhead_number;" );
  q.bindValue(":bankaccnt_id", pBankaccntid);
  q.exec();
  _check->populate(q);
  _check->setNull();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void printCheck::populate(int pcheckid)
{
  q.prepare( "SELECT checkhead_bankaccnt_id "
             "FROM checkhead "
             "WHERE (checkhead_id=:checkhead_id);" );
  q.bindValue(":checkhead_id", pcheckid);
  q.exec();
  if (q.first())
  {
    _bankaccnt->setId(q.value("checkhead_bankaccnt_id").toInt());
    _check->setId(pcheckid);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void printCheck::sCreateEFT()
{
  XSqlQuery releasenum;
  releasenum.prepare("SELECT releaseNumber('ACHBatch', :batch);");

  QString batch;

  MetaSQLQuery mql("SELECT *"
                   "  FROM <? literal(\"func\") ?>(<? value(\"bank\") ?>,"
                   "           <? value(\"check\") ?>, <? value(\"key\") ?>);");
  ParameterList params;
  params.append("func", _metrics->value("EFTFunction"));
  params.append("bank", _bankaccnt->id());
  params.append("check",_check->id());
  params.append("key",  omfgThis->_key);
  q = mql.toQuery(params);
  if (q.first())
  {
    batch = q.value("achline_batch").toString();
    releasenum.bindValue(":batch", batch);
    if (eftFileDir.isEmpty())
    {
      eftFileDir = xtsettingsValue("ACHOutputDirectory").toString();
    }
    QString suffixes = "*.ach *.aba *.dat *.txt";
    if (! suffixes.contains(_metrics->value("ACHDefaultSuffix")))
      suffixes = "*" + _metrics->value("ACHDefaultSuffix") + " " + suffixes;
    QString filename = QFileDialog::getSaveFileName(this, tr("EFT Output File"),
                            printCheck::eftFileDir + QDir::separator() +
                            "eft" + batch + _metrics->value("ACHDefaultSuffix"),
                            "(" + suffixes + ")");
    if (filename.isEmpty())
    {
      releasenum.exec();
      return;
    }
    QFileInfo fileinfo(filename);
    eftFileDir = fileinfo.absolutePath();
    QFile eftfile(filename);
    if (! eftfile.open(QIODevice::WriteOnly))
    {
      releasenum.exec();
      QMessageBox::critical(this, tr("Could Not Open File"),
                            tr("Could not open %1 for writing EFT data.")
                            .arg(filename));
      return;
    }
    do
    {
      eftfile.write(q.value("achline_value").toString().toAscii());
      eftfile.write("\n");
    } while (q.next());
    eftfile.close();
    if (q.lastError().type() != QSqlError::NoError)
    {
      releasenum.exec();
      eftfile.remove();
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    if (confirmAchOK::askOK(this, eftfile))
      markCheckAsPrinted(_check->id());
    else
    {
      releasenum.exec();
      XSqlQuery clearq;
      clearq.prepare("UPDATE checkhead "
                     "SET checkhead_printed=false,"
                     "    checkhead_ach_batch=NULL "
                     "WHERE (checkhead_id=:checkhead_id);");
      clearq.bindValue(":checkhead_id", _check->id());
      clearq.exec();
      if (clearq.lastError().type() != QSqlError::NoError)
      {
        systemError(this, clearq.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

}

void printCheck::sEnableCreateEFT()
{
  if (_bankaccnt->isValid() && _check->isValid() &&
      _metrics->boolean("ACHSupported") && _metrics->boolean("ACHEnabled"))
  {
    q.prepare("SELECT bankaccnt_ach_enabled AND vend_ach_enabled AS eftenabled "
              "FROM bankaccnt"
              "      JOIN checkhead ON (checkhead_bankaccnt_id=bankaccnt_id)"
              "      JOIN vendinfo ON (checkhead_recip_id=vend_id"
              "                    AND checkhead_recip_type='V') "
              "WHERE ((bankaccnt_id=:bankaccnt_id)"
              "  AND  (checkhead_id=:checkhead_id));");
    q.bindValue(":bankaccnt_id", _bankaccnt->id());
    q.bindValue(":checkhead_id", _check->id());
    q.exec();
    if (q.first())
      _createEFT->setEnabled(q.value("eftenabled").toBool());
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    else // not found
      _createEFT->setEnabled(false);
  }
  else
    _createEFT->setEnabled(false);
}


void printCheck::storeEftFileDir()
{
  if (_metrics->boolean("ACHSupported") && _metrics->boolean("ACHEnabled") && !eftFileDir.isEmpty())
  {
    xtsettingsSetValue("ACHOutputDirectory", eftFileDir);
  }
}

void printCheck::done(int /*p*/)
{
  storeEftFileDir();
  close();
}

void printCheck::markCheckAsPrinted(const int pcheckid)
{
  XSqlQuery markq;
  markq.prepare("SELECT checkhead_bankaccnt_id,"
                "       markCheckAsPrinted(checkhead_id) AS result "
                "FROM checkhead "
                "WHERE (checkhead_id=:checkhead_id);" );
  markq.bindValue(":checkhead_id", pcheckid);
  markq.exec();
  if (markq.first())
  {
    int result = markq.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("markCheckAsPrinted", result),
                  __FILE__, __LINE__);
      return;
    }
    omfgThis->sChecksUpdated(markq.value("checkhead_bankaccnt_id").toInt(),
                             pcheckid, TRUE);

    if (_captive)
      close();
    else
    {
      sHandleBankAccount(_bankaccnt->id());
      _close->setText(tr("&Close"));
    }
  }
}
