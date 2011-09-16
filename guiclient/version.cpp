/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "version.h"

QString _Name = "xTuple ERP: %1 Edition";

#ifndef __USEALTVERSION__
QString _Version   = "3.8.0Beta";
QString _dbVersion = "3.8.0Beta";

#else
#include "../altVersion.cpp"
#endif
QString _Copyright = "Copyright (c) 1999-2011, OpenMFG, LLC.";

/*: Please translate this Version string to the base version of the application
    you are translating. This is a hack to embed the application version number
    into the translation file so the Update Manager can find
    the best translation file for a given version of the application.
 */
static QString _translationFileVersionPlaceholder = QT_TRANSLATE_NOOP("xTuple", "Version");
