/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "updateABCClass.h"

#include <QVariant>
#include <QMessageBox>
#include "guiclient.h"

updateABCClass::updateABCClass(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_update, SIGNAL(clicked()), this, SLOT(sUpdate()));

  _classCode->setType(ParameterGroup::ClassCode);
  _dates->setStartCaption(tr("Start Evaluation Date"));
  _dates->setEndCaption(tr("End Evaluation Date"));
}

updateABCClass::~updateABCClass()
{
  // no need to delete child widgets, Qt does it all for us
}

void updateABCClass::languageChange()
{
  retranslateUi(this);
}

void updateABCClass::sUpdate()
{
  if ( (_classCode->isPattern()) && (_classCode->pattern().length() == 0) )
  {
    QMessageBox::critical( this, tr("Enter Class Code Pattern"),
                           tr( "You must enter the Class Code pattern to be used when updating Item Site\n"
                               "Class Codes or select a Class Code from the Class Code list." ) );
    _classCode->setFocus();
    return;
  }

  if (!_dates->startDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter Evaluation Start Date"),
                           tr("You must enter the Start Date of the evaluation period." ) );
    _dates->setFocus();
    return;
  }

  if (!_dates->endDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter Evaluation End Date"),
                           tr("You must enter the End Date of the evaluation period." ) );
    _dates->setFocus();
    return;
  }

  if (_classACutoff->toDouble() == 0.0)
  {
    QMessageBox::critical( this, tr("Enter Class A Cutoff %"),
                           tr("You must enter the cutoff point for Class A Items.") );
    _classACutoff->setFocus();
    return;
  }

  if (_classBCutoff->toDouble() == 0.0)
  {
    QMessageBox::critical( this, tr("Enter Class B Cutoff %"),
                           tr("You must enter the cutoff point for Class B Items.") );
    _classACutoff->setFocus();
    return;
  }

  if (_warehouse->isAll())
  {
    if (_classCode->isPattern())
      q.prepare("SELECT updateABCClass(:classcode_pattern, :aCutOff, :bCutOff, :startDate, :endDate) AS result;");
    else if (_classCode->isSelected())
      q.prepare("SELECT updateABCClass(:classcode_id, :aCutOff, :bCutOff, :startDate, :endDate) AS result;");
    else if (_classCode->isAll())
      q.prepare("SELECT updateABCClass(-1, :aCutOff, :bCutOff, :startDate, :endDate) AS result;");
  }
  else if (_warehouse->isSelected())
  {
    if (_classCode->isSelected())
      q.prepare("SELECT updateABCClass(:classcode_pattern, :warehous_id, :aCutOff, :bCutOff, :startDate, :endDate) AS result;");
    else if (_classCode->isSelected())
      q.prepare("SELECT updateABCClass(:classcode_id, :warehous_id, :aCutOff, :bCutOff, :startDate, :endDate) AS result;");
    else if (_classCode->isAll())
      q.prepare("SELECT updateABCClass(-1, :warehous_id, :aCutOff, :bCutOff, :startDate, :endDate) AS result;");
  }

  _warehouse->bindValue(q);
  _classCode->bindValue(q);
  _dates->bindValue(q);
  q.bindValue(":aCutOff", (_classACutoff->toDouble() / 100.0));
  q.bindValue(":bCutOff", (_classBCutoff->toDouble() / 100.0));
  q.exec();
  if (q.first())
    QMessageBox::information( this, tr("ABC Class Code Updated"),
                              tr("The ABC Class Code was updated for %1 Item Sites.")
                              .arg(q.value("result").toInt()) );
  else
    systemError(this, tr("A System Error occurred at %1::%2.")
                      .arg(__FILE__)
                      .arg(__LINE__) );

  _close->setText(tr("&Close"));
}

