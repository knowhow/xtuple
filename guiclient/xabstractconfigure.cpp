/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xabstractconfigure.h"

/** \brief Abstract interface definition for xTuple ERP Configure widgets.

  XAbstractConfigure is an abstract class defining a simple programming
  interface that all configuration windows must implement. This is
  used by the Setup window to ensure that all data get saved properly.

  Every configuration UI intended to be included in the Setup window
  should inherit from XAbstractConfigure. It must implement a
  languageChange() method and an sSave() method.  The languageChange()
  method is usually empty; it is required to ensure that the
  configuration window can be translated. The sSave method returns
  a boolean. It should return true if the save attempt was successful
  and false if the save attempt failed. If the save attempt failed,
  sSave() should set the focus on the failed widget and give the user a
  QMessageBox explaining the failure.  It is the Setup window's
  responsibility to handle the failure and focus widget gracefully.

  Child classes of XAbstractConfigure should define a saving() signal
  and emit it at the beginning of sSave().  This will allow scripts
  on these child classes to save their own data.

  \todo Find a way for scripts to return failures so they also can
        handle errors during saving.

  \see setup

*/

XAbstractConfigure::XAbstractConfigure(QWidget* parent, Qt::WindowFlags f)
  : XWidget(parent, f)
{
}

XAbstractConfigure::~XAbstractConfigure()
{
}
