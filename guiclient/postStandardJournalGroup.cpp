/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "postStandardJournalGroup.h"

#include <QVariant>
#include <QMessageBox>
#include "glSeries.h"

/*
 *  Constructs a postStandardJournalGroup as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
postStandardJournalGroup::postStandardJournalGroup(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_submit, SIGNAL(clicked()), this, SLOT(sSubmit()));

  if (!_metrics->boolean("EnableBatchManager"))
    _submit->hide();

  _captive = false;
  _doSubmit = false;

  _stdjrnlgrp->setAllowNull(TRUE);
  _stdjrnlgrp->populate( "SELECT stdjrnlgrp_id, stdjrnlgrp_name "
                         "FROM stdjrnlgrp "
                         "ORDER BY stdjrnlgrp_name;" );
}

/*
 *  Destroys the object and frees any allocated resources
 */
postStandardJournalGroup::~postStandardJournalGroup()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void postStandardJournalGroup::languageChange()
{
  retranslateUi(this);
}

enum SetResponse postStandardJournalGroup::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("stdjrnlgrp_id", &valid);
  if (valid)
  {
    _stdjrnlgrp->setId(param.toInt());
    _post->setFocus();
  }

  return NoError;
}


void postStandardJournalGroup::sPost()
{
  if (!_distDate->isValid())
  {
    QMessageBox::critical( this, tr("Cannot Post Standard Journal Group"),
                           tr("You must enter a Distribution Date before you may post this Standard Journal Group.") );
    _distDate->setFocus();
    return;
  }

  q.prepare("SELECT postStandardJournalGroup(:stdjrnlgrp_id, :distDate, :reverse) AS result;");
  q.bindValue(":stdjrnlgrp_id", _stdjrnlgrp->id());
  q.bindValue(":distDate", _distDate->date());
  q.bindValue(":reverse", QVariant(_reverse->isChecked()));
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("mode", "postStandardJournal");
    params.append("glSequence", q.value("result"));
    if(_doSubmit)
      params.append("submit");

    glSeries newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
  }
  else
    systemError(this, tr("A System Error occurred at %1::%2.")
                      .arg(__FILE__)
                      .arg(__LINE__) );

  if (_captive)
    accept();
  {
    _stdjrnlgrp->setNull();
    _close->setText(tr("&Close"));
    _stdjrnlgrp->setFocus();
  }
}

void postStandardJournalGroup::sSubmit()
{
  _doSubmit = true;
  sPost();
  _doSubmit = false;
}
