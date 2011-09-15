/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "purgePostedCounts.h"

#include <qvariant.h>

/*
 *  Constructs a purgePostedCounts as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
purgePostedCounts::purgePostedCounts(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_purge, SIGNAL(clicked()), this, SLOT(sPurge()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
purgePostedCounts::~purgePostedCounts()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void purgePostedCounts::languageChange()
{
    retranslateUi(this);
}


void purgePostedCounts::init()
{
}

void purgePostedCounts::sPurge()
{
  if (_cutOffDate->isValid())
  {
    q.prepare("SELECT purgePostedCountTags(:cutOffDate, :warehous_id) AS _result;");
    q.bindValue(":cutOffDate", _cutOffDate->date());
    q.bindValue(":warehous_id", ((_warehouse->isSelected()) ? _warehouse->id() : -1));
    q.exec();

    _cutOffDate->clear();
    _cutOffDate->setFocus();

    _close->setText(tr("&Cancel"));
  }
}

