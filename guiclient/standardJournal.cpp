/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "standardJournal.h"

#include <QVariant>
#include <QMessageBox>
#include "standardJournalItem.h"

standardJournal::standardJournal(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _stdjrnlid = -1;

  // signals and slots connections
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_stdjrnlitem, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
  connect(_name, SIGNAL(lostFocus()), this, SLOT(sCheck()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(this, SIGNAL(rejected()), this, SLOT(sReject()));

  _stdjrnlitem->addColumn(tr("Account"), 200,          Qt::AlignLeft,   true,  "account"  );
  _stdjrnlitem->addColumn(tr("Notes"),   -1,           Qt::AlignLeft,   true,  "note"  );
  _stdjrnlitem->addColumn(tr("Debit"),   _priceColumn, Qt::AlignRight,  true,  "debit" );
  _stdjrnlitem->addColumn(tr("Credit"),  _priceColumn, Qt::AlignRight,  true,  "credit" );

  _debits->setValidator(omfgThis->moneyVal());
  _credits->setValidator(omfgThis->moneyVal());
}

standardJournal::~standardJournal()
{
  destroy();
  // no need to delete child widgets, Qt does it all for us
}

void standardJournal::languageChange()
{
  retranslateUi(this);
}

void standardJournal::sReject()
{
  if (_mode == cNew)
  {
    q.prepare( "DELETE FROM stdjrnlitem "
               "WHERE (stdjrnlitem_stdjrnl_id=:stdjrnl_id);" );
    q.bindValue(":stdjrnl_id", _stdjrnlid);
    q.exec();
  }
}

enum SetResponse standardJournal::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("stdjrnl_id", &valid);
  if (valid)
  {
    _stdjrnlid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      q.exec("SELECT NEXTVAL('stdjrnl_stdjrnl_id_seq') AS _stdjrnl_id");
      if (q.first())
        _stdjrnlid = q.value("_stdjrnl_id").toInt();

      connect(_stdjrnlitem, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_stdjrnlitem, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      connect(_stdjrnlitem, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));

      _stdjrnlitem->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      connect(_stdjrnlitem, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_stdjrnlitem, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      connect(_stdjrnlitem, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));

      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      connect(_stdjrnlitem, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));

      _new->setEnabled(FALSE);
      _name->setEnabled(FALSE);
      _descrip->setEnabled(FALSE);
      _notes->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void standardJournal::sSave()
{
  if (_name->text().length() == 0)
  {
    QMessageBox::warning( this, tr("Cannot Save Standard Journal"),
                          tr("You must enter a valid Name.") );
    return;
  }
  
  q.prepare( "SELECT stdjrnl_id"
             "  FROM stdjrnl "
             " WHERE((UPPER(stdjrnl_name)=UPPER(:stdjrnl_name))"
             "   AND (stdjrnl_id != :stdjrnl_id));" );
  q.bindValue(":stdjrnl_name", _name->text());
  q.bindValue(":stdjrnl_id", _stdjrnlid);
  q.exec();
  if (q.first())
  {
    QMessageBox::warning( this, tr("Cannot Save Standard Journal"),
                          tr("The Name you have entered for this Standard Journal already exists. "
                             "Please enter in a different Name for this Standard Journal."));
    _name->setFocus();
    return;
  }

  if (_mode == cNew)
    q.prepare( "INSERT INTO stdjrnl "
               "(stdjrnl_id, stdjrnl_name, stdjrnl_descrip, stdjrnl_notes) "
               "VALUES "
               "(:stdjrnl_id, :stdjrnl_name, :stdjrnl_descrip, :stdjrnl_notes);" );
  else if (_mode == cEdit)
    q.prepare( "UPDATE stdjrnl "
               " SET stdjrnl_name=:stdjrnl_name, stdjrnl_descrip=:stdjrnl_descrip, stdjrnl_notes=:stdjrnl_notes "
               " WHERE (stdjrnl_id=:stdjrnl_id);" );

  q.bindValue(":stdjrnl_id", _stdjrnlid);
  q.bindValue(":stdjrnl_name", _name->text());
  q.bindValue(":stdjrnl_descrip", _descrip->text());
  q.bindValue(":stdjrnl_notes", _notes->toPlainText());
  q.exec();

  done(_stdjrnlid);
}

void standardJournal::sCheck()
{
  _name->setText(_name->text().trimmed());
  if ((_mode == cNew) && (_name->text().length()))
  {
    q.prepare( "SELECT stdjrnl_id"
               "  FROM stdjrnl"
               " WHERE((UPPER(stdjrnl_name)=UPPER(:stdjrnl_name))"
               "   AND (stdjrnl_id != :stdjrnl_id));" );
    q.bindValue(":stdjrnl_name", _name->text());
    q.bindValue(":stdjrnl_id", _stdjrnlid);
    q.exec();
    if (q.first())
    {
      _stdjrnlid = q.value("stdjrnl_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(FALSE);
    }
  }
}

void standardJournal::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("stdjrnl_id", _stdjrnlid);

  standardJournalItem newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void standardJournal::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("stdjrnlitem_id", _stdjrnlitem->id());

  standardJournalItem newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void standardJournal::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("stdjrnlitem_id", _stdjrnlitem->id());

  standardJournalItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void standardJournal::sDelete()
{
  q.prepare( "DELETE FROM stdjrnlitem "
             "WHERE (stdjrnlitem_id=:stdjrnlitem_id);" );
  q.bindValue(":stdjrnlitem_id", _stdjrnlitem->id());
  q.exec();
  sFillList();
}

void standardJournal::sFillList()
{
  q.prepare( "SELECT stdjrnlitem_id,"
             "       CASE WHEN(accnt_id IS NOT NULL) THEN (formatGLAccount(accnt_id) || '-' || accnt_descrip)"
             "            ELSE 'ERROR - NO ACCOUNT SPECIFIED'"
             "       END AS account,"
             "       firstLine(stdjrnlitem_notes) AS note,"
             "       CASE WHEN (stdjrnlitem_amount < 0) THEN (stdjrnlitem_amount * -1)"
             "            ELSE NULL"
             "       END AS debit,"
             "       CASE WHEN (stdjrnlitem_amount > 0) THEN (stdjrnlitem_amount)"
             "            ELSE NULL"
             "       END AS credit,"
             "       'curr' AS debit_xtnumericrole,"
             "       'curr' AS credit_xtnumericrole,"
             "       '' AS debit_xtnullrole,"
             "       '' AS credit_xtnullrole "
             "  FROM stdjrnlitem LEFT OUTER JOIN accnt ON (stdjrnlitem_accnt_id=accnt_id)"
             " WHERE (stdjrnlitem_stdjrnl_id=:stdjrnl_id) "
             " ORDER BY accnt_number, accnt_profit, accnt_sub;" );
  q.bindValue(":stdjrnl_id", _stdjrnlid);
  q.exec();
  _stdjrnlitem->populate(q);

  q.prepare( "SELECT SUM( CASE WHEN (stdjrnlitem_amount < 0) THEN (stdjrnlitem_amount * -1)"
             "                 ELSE 0"
             "            END ) AS debit,"
             "       SUM( CASE WHEN (stdjrnlitem_amount > 0) THEN stdjrnlitem_amount"
             "                 ELSE 0"
             "            END ) AS credit,"
             "       (SUM(stdjrnlitem_amount) <> 0) AS oob "
             "FROM stdjrnlitem "
             "WHERE (stdjrnlitem_stdjrnl_id=:stdjrnl_id);" );
  q.bindValue(":stdjrnl_id", _stdjrnlid);
  q.exec();
  if (q.first())
  {
    _debits->setDouble(q.value("debit").toDouble());
    _credits->setDouble(q.value("credit").toDouble());

    QString stylesheet;
    if (q.value("oob").toBool())
      stylesheet = QString("* { color: %1; }").arg(namedColor("error").name());
    _debits->setStyleSheet(stylesheet);
    _credits->setStyleSheet(stylesheet);
  }
  else
    systemError(this, tr("A System Error occurred at %1::%2.")
                      .arg(__FILE__)
                      .arg(__LINE__) );
}

void standardJournal::populate()
{
  q.prepare( "SELECT stdjrnl_name, stdjrnl_descrip, stdjrnl_notes "
             "FROM stdjrnl "
             "WHERE (stdjrnl_id=:stdjrnl_id);" );
  q.bindValue(":stdjrnl_id", _stdjrnlid);
  q.exec();
  if (q.first())
  {
    _name->setText(q.value("stdjrnl_name").toString());
    _descrip->setText(q.value("stdjrnl_descrip").toString());
    _notes->setText(q.value("stdjrnl_notes").toString());

    sFillList();
  }
  else
    systemError(this, tr("A System Error occurred at %1::%2.")
                      .arg(__FILE__)
                      .arg(__LINE__) );
}

