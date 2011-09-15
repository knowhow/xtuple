/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "welcomeStub.h"

#include <QStyle>
#include <QPixmap>
#include <QIcon>

welcomeStub::welcomeStub(QWidget* parent, Qt::WindowFlags f)
  : QDialog(parent, f)
{
  setupUi(this);

  QString url = "<a href=\"http://www.xtuple.org/translate\">http://www.xtuple.org/translate</a>";

  int iconSize = style()->pixelMetric(QStyle::PM_MessageBoxIconSize);
  iconLabel->setPixmap(style()->standardIcon(QStyle::SP_MessageBoxInformation).pixmap(iconSize, iconSize));
  label->setText(tr("<p>It appears you do not have an xTuple translation file installed on your system. Please use the following link to find a translation file for your language and learn more about how to install translation files in xTuple ERP:<p>%1<p>").arg(url));

  adjustSize();
}

welcomeStub::~welcomeStub()
{
  // no need to delete child widgets, Qt does it all for us
}

void welcomeStub::languageChange()
{
  retranslateUi(this);
}

