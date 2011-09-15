/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "standardJournals.h"

#include <QVariant>
#include <QMessageBox>
//#include <QStatusBar>
#include <openreports.h>
#include <parameter.h>
#include "postStandardJournal.h"
#include "standardJournal.h"

/*
 *  Constructs a standardJournals as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
standardJournals::standardJournals(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
    setupUi(this);

//    (void)statusBar();

    // signals and slots connections
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
    connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
    connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
    connect(_stdjrnl, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
    connect(_stdjrnl, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
    connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
standardJournals::~standardJournals()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void standardJournals::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <QMenu>

void standardJournals::init()
{
//  statusBar()->hide();
  
  if (_privileges->check("MaintainStandardJournals"))
  {
    connect(_stdjrnl, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_stdjrnl, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_stdjrnl, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
    _new->setEnabled(FALSE);

  if (_privileges->check("PostStandardJournals"))
    connect(_stdjrnl, SIGNAL(valid(bool)), _post, SLOT(setEnabled(bool)));

  _stdjrnl->addColumn(tr("Name"),        _itemColumn, Qt::AlignLeft,  true,  "stdjrnl_name" );
  _stdjrnl->addColumn(tr("Description"), -1,          Qt::AlignLeft,  true,  "stdjrnl_descrip" );

  sFillList();
}

void standardJournals::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  standardJournal newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void standardJournals::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("stdjrnl_id", _stdjrnl->id());

  standardJournal newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void standardJournals::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("stdjrnl_id", _stdjrnl->id());

  standardJournal newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void standardJournals::sPost()
{
  ParameterList params;
  params.append("stdjrnl_id", _stdjrnl->id());

  postStandardJournal newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void standardJournals::sDelete()
{
  q.prepare("SELECT deleteStandardJournal(:stdjrnl_id);");
  q.bindValue(":stdjrnl_id", _stdjrnl->id());
  q.exec();

  sFillList();
}

void standardJournals::sFillList()
{
  _stdjrnl->populate( "SELECT stdjrnl_id, stdjrnl_name, stdjrnl_descrip "
                      "FROM stdjrnl "
                      "ORDER BY stdjrnl_name;" );
}

void standardJournals::sPopulateMenu(QMenu *)
{

}

void standardJournals::sPrint()
{
  orReport report("StandardJournalMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

