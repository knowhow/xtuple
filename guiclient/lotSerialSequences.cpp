/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "lotSerialSequences.h"

#include <QVariant>
#include <QMessageBox>

#include <parameter.h>
#include <openreports.h>
#include "lotSerialSequence.h"
#include "guiclient.h"

/*
 *  Constructs a lotSerialSequences as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
lotSerialSequences::lotSerialSequences(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete())); 
  
  _lsseq->addColumn(tr("Number"),        _itemColumn, Qt::AlignLeft, true, "lsseq_number" );
  _lsseq->addColumn(tr("Description"),   -1,          Qt::AlignLeft, true, "lsseq_descrip" );

  if (_privileges->check("MaintainLotSerialSequences"))
  {
    connect(_lsseq, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_lsseq, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_lsseq, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    connect(_lsseq, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
    _new->setEnabled(FALSE);
  }

   sFillList();
}

/*
 *  Destroys the object and frees any allocated resources
 */
lotSerialSequences::~lotSerialSequences()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void lotSerialSequences::languageChange()
{
  retranslateUi(this);
}

void lotSerialSequences::sDelete()
{
  q.prepare( "SELECT itemsite_id "
             "FROM itemsite "
             "WHERE (itemsite_lsseq_id=:lsseq_id);" );
  q.bindValue(":lsseq_id", _lsseq->id());
  q.exec();
  if (q.first())
  {
    QMessageBox::critical( this, tr("Cannot Delete Sequence"),
                           tr( "The selected Planner Code cannot be deleted as there are one or more Item Sites currently assigned to it.\n"
                               "You must reassign these Item Sites before you may delete the selected Planner Code." ) );
    return;
  }

  q.prepare( "DELETE FROM lsseq "
             "WHERE (lsseq_id=:lsseq_id);" );
  q.bindValue(":lsseq_id", _lsseq->id());
  q.exec();

  sFillList();
}

void lotSerialSequences::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("lsseq_id", _lsseq->id());

  lotSerialSequence newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void lotSerialSequences::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("lsseq_id", _lsseq->id());

  lotSerialSequence newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void lotSerialSequences::sFillList()
{
  _lsseq->populate( "SELECT lsseq_id, lsseq_number, lsseq_descrip "
                        "FROM lsseq "
                        "ORDER BY lsseq_number;" );
}

void lotSerialSequences::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  lotSerialSequence newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}
