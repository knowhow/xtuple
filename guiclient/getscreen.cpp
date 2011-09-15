/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "getscreen.h"

#define STARTCLASSLIST \
  if(classname.isEmpty()) \
    return 0;
#define ENDCLASSLIST
#define CLASSITEM(cn) \
  if(classname == #cn) { \
    w = new cn(parent, #cn, wflags); \
    w->setObjectName(#cn); \
  }
    
#include "getscreen_headerlist.h"

QWidget * xtGetScreen(const QString & classname, QWidget * parent, Qt::WindowFlags wflags, const QString & objectname)
{
  QWidget * w = 0;

STARTCLASSLIST
#include "getscreen_classlist.h"
ENDCLASSLIST

  if(w)
  {
    if(!objectname.isEmpty())
      w->setObjectName(objectname);
  }

  return w;
}

