/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "updateOUTLevelsByClassCode.h"

#include <QMessageBox>
#include <qvariant.h>
#include <parameter.h>
#include "submitAction.h"

/*
 *  Constructs a updateOUTLevelsByClassCode as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
updateOUTLevelsByClassCode::updateOUTLevelsByClassCode(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    _daysGroupInt = new QButtonGroup(this);
    _daysGroupInt->addButton(_leadTime);
    _daysGroupInt->addButton(_fixedDays);

    // signals and slots connections
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_update, SIGNAL(clicked()), this, SLOT(sUpdate()));
    connect(_calendar, SIGNAL(newCalendarId(int)), _periods, SLOT(populate(int)));
    connect(_fixedDays, SIGNAL(toggled(bool)), _days, SLOT(setEnabled(bool)));
    connect(_leadTime, SIGNAL(toggled(bool)), _leadTimePad, SLOT(setEnabled(bool)));
    connect(_submit, SIGNAL(clicked()), this, SLOT(sSubmit()));
    
    if (!_metrics->boolean("EnableBatchManager"))
      _submit->hide();
    
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
updateOUTLevelsByClassCode::~updateOUTLevelsByClassCode()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void updateOUTLevelsByClassCode::languageChange()
{
    retranslateUi(this);
}


void updateOUTLevelsByClassCode::init()
{
  _classCode->setType(ParameterGroup::ClassCode);
}

void updateOUTLevelsByClassCode::sUpdate()
{
  if (_periods->topLevelItemCount() > 0)
  {
    QString sql;

    if (_leadTime->isChecked())
      sql = QString( "SELECT updateOUTLevel(itemsite_id, (itemsite_leadtime + :leadTimePad), '{%1}') AS result "
                     "FROM itemsite, item, classcode "
                     "WHERE ( (itemsite_item_id=item_id)"
                     " AND (item_classcode_id=classcode_id)" )
            .arg(_periods->periodString());

    else if (_fixedDays->isChecked())
      sql = QString( "SELECT updateOUTLevel(itemsite_id, :days, '{%1}') AS result "
                     "FROM itemsite, item, classcode "
                     "WHERE ( (itemsite_item_id=item_id)"
                     " AND (item_classcode_id=classcode_id)" )
            .arg(_periods->periodString());

    if (_warehouse->isSelected())
      sql += " AND (itemsite_warehous_id=:warehous_id)";

    if (_classCode->isSelected())
      sql += " AND (classcode_id=:classcode_id)";
    else if (_classCode->isPattern())
      sql += " AND (classcode_code ~ :classcode_pattern)";

    sql += ");";

    q.prepare(sql);
    q.bindValue(":leadTimePad", _leadTimePad->value());
    q.bindValue(":days", _days->value());
    _warehouse->bindValue(q);
    _classCode->bindValue(q);
    q.exec();

    accept();
  }
  else
  {
    QMessageBox::critical( this, tr("Incomplete Data"),
                           tr("You must select at least one Period to continue.") );
    _periods->setFocus();
    return;
  }
}

void updateOUTLevelsByClassCode::sSubmit()
{
  if (_periods->topLevelItemCount() > 0)
  {
    ParameterList params;
    params.append("action_name", "UpdateOUTLevel");
    params.append("period_id_list", _periods->periodString());
    _warehouse->appendValue(params);
    _classCode->appendValue(params);

    if (_leadTime->isChecked())
      params.append("leadtimepad", _leadTimePad->value());
    else if (_fixedDays->isChecked())
      params.append("fixedlookahead", _days->value());

    submitAction newdlg(this, "", TRUE);
    newdlg.set(params);

    if (newdlg.exec() == XDialog::Accepted)
      accept();
  }
  else
  {
    QMessageBox::critical( this, tr("Incomplete Data"),
                           tr("You must select at least one Period to continue.") );
    _periods->setFocus();
    return;
  }
}
