/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "glSeries.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "glSeriesItem.h"
#include "storedProcErrorLookup.h"
#include "submitAction.h"

#define cPostStandardJournal 0x10

#define DEBUG false

glSeries::glSeries(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _post = _buttonBox->addButton(tr("&Post"), QDialogButtonBox::ActionRole);

  connect(_delete,	SIGNAL(clicked()),	this, SLOT(sDelete()));
  connect(_edit,	SIGNAL(clicked()),	this, SLOT(sEdit()));
  connect(_new,		SIGNAL(clicked()),	this, SLOT(sNew()));
  connect(_post,	SIGNAL(clicked()),	this, SLOT(sPost()));
  connect(_buttonBox,	SIGNAL(accepted()),	this, SLOT(sSave()));

  _glseries->addColumn(tr("Account"), -1,           Qt::AlignLeft,  true,  "account"  );
  _glseries->addColumn(tr("Debit"),   _moneyColumn, Qt::AlignRight, true,  "debit" );
  _glseries->addColumn(tr("Credit"),  _moneyColumn, Qt::AlignRight, true,  "credit" );

  _credits->setPrecision(omfgThis->moneyVal());
  _debits->setPrecision(omfgThis->moneyVal());
  _diff->setPrecision(omfgThis->moneyVal());

  _source->setText("G/L");
  _source->setEnabled(false);

  _doctype->addItem("AD");
  _doctype->addItem("CD");
  _doctype->addItem("CK");
  _doctype->addItem("CM");
  _doctype->addItem("CR");
  _doctype->addItem("CT");
  _doctype->addItem("DM");
  _doctype->addItem("DS");
  _doctype->addItem("IN");
  _doctype->addItem("JE");
  _doctype->addItem("MM");
  _doctype->addItem("PO");
  _doctype->addItem("SO");
  _doctype->addItem("ST");
  _doctype->addItem("VO");
  _doctype->addItem("WO");
  _doctype->setCurrentIndex(_doctype->findText("JE"));

  _submit = false;
  _journal = 0;
}

glSeries::~glSeries()
{
  // no need to delete child widgets, Qt does it all for us
}

void glSeries::languageChange()
{
  retranslateUi(this);
}

enum SetResponse glSeries::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  _submit = pParams.inList("submit");
  if((_submit)  && (_metrics->boolean("EnableBatchManager")))
    _post->setText(tr("Schedule"));
  else
    _submit = false;

  param = pParams.value("glSequence", &valid);
  if (valid)
  {
    _glsequence = param.toInt();
    q.prepare("SELECT DISTINCT glseries_distdate, glseries_source,"
	      "                glseries_doctype,  glseries_docnumber,"
	      "                glseries_notes"
              "  FROM glseries"
              " WHERE (glseries_sequence=:glseries_sequence);" );
    q.bindValue(":glseries_sequence", _glsequence);
    q.exec();
    if(q.first())
    {
      _date->setDate(q.value("glseries_distdate").toDate());
      _source->setText(q.value("glseries_source").toString());
      int idx = _doctype->findText(q.value("glseries_doctype").toString());
      if(idx < 0)
        _doctype->addItem(q.value("glseries_doctype").toString());
      _doctype->setCurrentIndex(_doctype->findText(q.value("glseries_doctype").toString()));
      _docnumber->setText(q.value("glseries_docnumber").toString());
      _notes->setText(q.value("glseries_notes").toString());
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
    sFillList();
  }

  param = pParams.value("journalnumber", &valid);
  if (valid)
  {
    _journal = param.toInt();
    _buttonBox->removeButton(_buttonBox->button(QDialogButtonBox::Save));
    _mode = cEdit;
    _doctype->setCurrentIndex(_doctype->findText("JE"));
    _doctype->setEnabled(false);
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      q.exec("SELECT fetchGLSequence() AS glsequence;");
      if (q.first())
        _glsequence = q.value("glsequence").toInt();
      else if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _post->setFocus();
    }
    else if (param.toString() == "postStandardJournal")
    {
      _mode = cPostStandardJournal;
      
      _doctype->setCurrentIndex(_doctype->findText("ST"));
      _doctype->setEnabled(false);
      _date->setEnabled(FALSE);
      _notes->setEnabled(FALSE);

      q.prepare( "SELECT DISTINCT glseries_docnumber, stdjrnl_notes "
                 "  FROM glseries, stdjrnl "
                 " WHERE ( (stdjrnl_name=glseries_docnumber) "
                 "   AND   (stdjrnl_notes IS NOT NULL) "
                 "   AND   (stdjrnl_notes != '') "
                 "   AND   (glseries_sequence=:glsequence) ); ");
      q.bindValue(":glsequence", _glsequence);
      q.exec();
      if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }
      while(q.next())
        _notes->append(q.value("glseries_docnumber").toString() + ": " + q.value("stdjrnl_notes").toString() + "\n\n");

      _post->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _new->setEnabled(false);
      disconnect(_glseries, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      disconnect(_glseries, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      _source->setEnabled(false);
      _doctype->setEnabled(false);
      _docnumber->setEnabled(false);
      _date->setEnabled(false);
      _notes->setEnabled(false);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
      _buttonBox->setFocus();
    }
  }

  return NoError;
}

void glSeries::sNew()
{
  if(!_date->isValid())
  {
    QMessageBox::information( this, tr("Cannot Maintain G/L Series"),
                              tr("<p>You must enter a Distribution Date for this Series.") );
    _date->setFocus();
    return;
  }

  ParameterList params;
  params.append("mode", "new");
  params.append("doctype", _doctype->currentText());
  params.append("docnumber", _docnumber->text());
  params.append("glSequence", _glsequence);
  params.append("distDate", _date->date());

  if (_mode == cPostStandardJournal)
    params.append("postStandardJournal");

  glSeriesItem newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
  {
    sFillList();
    sNew();
  }
}

void glSeries::sEdit()
{
  if(!_date->isValid())
  {
    QMessageBox::information( this, tr("Cannot Maintain G/L Series"),
                              tr("<p>You must enter a Distribution Date for this Series.") );
    _date->setFocus();
    return;
  }

  ParameterList params;
  params.append("mode", "edit");
  params.append("glseries_id", _glseries->id());
  params.append("glSequence", _glsequence);
  params.append("distDate", _date->date());

  if (_mode == cPostStandardJournal)
    params.append("postStandardJournal");

  glSeriesItem newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void glSeries::sDelete()
{
  q.prepare( "DELETE FROM glseries "
             "WHERE (glseries_id=:glseries_id);" );
  q.bindValue(":glseries_id", _glseries->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

bool glSeries::update()
{
  if (_mode != cPostStandardJournal)
  {
    if(!_date->isValid())
    {
      QMessageBox::information( this, tr("Cannot Post G/L Series"),
				tr("<p>You must enter a Distribution Date for this Series.") );
      _date->setFocus();
      return false;
    }

    if(_metrics->boolean("MandatoryGLEntryNotes") && _notes->toPlainText().trimmed().isEmpty())
    {
      QMessageBox::information( this, tr("Cannot Post G/L Series"),
				tr("<p>You must enter some Notes to describe this transaction.") );
      _notes->setFocus();
      return false;
    }

    q.prepare( "UPDATE glseries "
               "SET glseries_notes=:glseries_notes, "
               "    glseries_source=:source,"
	       "    glseries_doctype=:doctype,"
	       "    glseries_docnumber=:docnumber,"
               "    glseries_distdate=:glseries_distdate "
               "WHERE (glseries_sequence=:glseries_sequence);" );
    q.bindValue(":glseries_notes", _notes->toPlainText());
    q.bindValue(":source",	_source->text());
    q.bindValue(":doctype",	_doctype->currentText());
    q.bindValue(":docnumber",	_docnumber->text());
    q.bindValue(":glseries_sequence", _glsequence);
    q.bindValue(":glseries_distdate", _date->date());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return false;
    }
  }

  q.prepare("SELECT SUM(glseries_amount) AS result "
            "  FROM glseries "
            " WHERE (glseries_sequence=:glseries_sequence); ");
  q.bindValue(":glseries_sequence", _glsequence);
  q.exec();
  if(q.first())
  {
    double result = q.value("result").toDouble();
    if(result != 0)
    {
      QMessageBox::critical( this, tr("Cannot Post G/L Series"),
			     tr("<p>The G/L Series information is unbalanced and cannot be posted. Please correct this before continuing.") );
      return false;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  omfgThis->sGlSeriesUpdated();
  return true;
}

void glSeries::sSave()
{
  if (! update())
    return;
  accept();
}

void glSeries::sPost()
{
  if (! update())
    return;

  if(_submit)
  {
    ParameterList params;

    params.append("action_name", "PostGLSeries");
    params.append("glseries_sequence", _glsequence); 

    submitAction newdlg(this, "", true);
    newdlg.set(params);

    if(newdlg.exec() == XDialog::Accepted)
    {
      // TODO: do something?
    }
  }
  else
  {
    q.prepare("SELECT postGLSeriesNoSumm(:glseries_sequence,COALESCE(:journal,fetchJournalNumber('G/L'))) AS return;");
    q.bindValue(":glseries_sequence", _glsequence);
    if (_journal)
      q.bindValue(":journal", _journal);
    q.exec();
    if (q.first())
    {
      int returnVal = q.value("return").toInt();
      if (returnVal < 0)
      {
        systemError(this, storedProcErrorLookup("postGLSeriesNoSumm", returnVal),
		    __FILE__, __LINE__);
        return;
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    omfgThis->sGlSeriesUpdated();
  }

  if(cPostStandardJournal == _mode || _journal)
  {
    accept();
    return;
  }

  _date->clear();
  _notes->clear();
  _debits->clear();
  _credits->clear();
  _diff->clear();
  _glseries->clear();
  _docnumber->clear();
  
  ParameterList params;
  params.append("mode", "new");
  set(params);
}

void glSeries::reject()
{
  if (DEBUG)
    qDebug("glSeries::reject() entered with _mode %d, topLevelItemCount %d",
           _mode, _glseries->topLevelItemCount());
  if (cNew == _mode &&
      _glseries->topLevelItemCount() > 0)
  {
    if (QMessageBox::question(this, tr("Delete G/L Series?"),
                              tr("<p>Are you sure you want to delete this G/L "
                                 "Series Entry?"),
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No) == QMessageBox::Yes)
    {
      q.prepare("SELECT deleteGLSeries(:glsequence);");
      q.bindValue(":glsequence", _glsequence);
      q.exec();
      if (q.lastError().type() != QSqlError::NoError)
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    }
    else
      return;
  }

  omfgThis->sGlSeriesUpdated();

  XDialog::reject();
}

void glSeries::sFillList()
{
  q.prepare( "SELECT glseries_id, (formatGLAccount(accnt_id) || '-' || accnt_descrip) AS account,"
             "       CASE WHEN (glseries_amount < 0) THEN (glseries_amount * -1)"
             "            ELSE 0"
             "       END AS debit,"
             "       CASE WHEN (glseries_amount > 0) THEN glseries_amount"
             "            ELSE 0"
             "       END AS credit,"
             "       'curr' AS debit_xtnumericrole,"
             "       'curr' AS credit_xtnumericrole "
             "FROM glseries, accnt "
             "WHERE ( (glseries_accnt_id=accnt_id)"
             " AND (glseries_sequence=:glseries_sequence) );" );
  q.bindValue(":glseries_sequence", _glsequence);
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  _glseries->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.prepare("SELECT SUM(CASE WHEN (glseries_amount < 0) THEN (glseries_amount * -1)"
            "                              ELSE 0"
            "                         END) AS debit,"
            "       SUM(CASE WHEN (glseries_amount > 0) THEN glseries_amount"
            "                              ELSE 0"
            "                         END ) AS credit,"
            "       SUM(glseries_amount) AS diff,"
            "       (SUM(glseries_amount) <> 0) AS oob "
            "FROM glseries "
            "WHERE (glseries_sequence=:glseries_sequence);" );
  q.bindValue(":glseries_sequence", _glsequence);
  q.exec();
  if (q.first())
  {
    _debits->setDouble(q.value("debit").toDouble());
    _credits->setDouble(q.value("credit").toDouble());
    _diff->setDouble(q.value("diff").toDouble());

    QString stylesheet;
    if (q.value("oob").toBool())
        stylesheet = QString("* { color: %1; }").arg(namedColor("error").name());

    _debits->setStyleSheet(stylesheet);
    _credits->setStyleSheet(stylesheet);
    _diff->setStyleSheet(stylesheet);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
