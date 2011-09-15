/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef XTUPLEDESIGNERACTIONS_H
#define XTUPLEDESIGNERACTIONS_H

#include <QObject>

class QAction;
class QActionGroup;
class QDesignerFormWindowInterface;
class xTupleDesigner;

class xTupleDesignerActions : QObject
{
  Q_OBJECT

  public:
    xTupleDesignerActions(xTupleDesigner *parent);

    virtual QActionGroup *editActions()   { return _editActions;   }
    virtual QActionGroup *fileActions()   { return _fileActions;   }
    virtual QActionGroup *formActions()   { return _formActions;   }
    virtual QActionGroup *toolActions()   { return _toolActions;   }

  public slots:
    virtual void sActiveFormWindowChanged(QDesignerFormWindowInterface*);
    virtual bool sClose();
    virtual void sEditBuddies();
    virtual void sEditSignalSlot();
    virtual void sEditTabOrder();
    virtual void sEditWidgets();
    virtual void sOpen();
    virtual void sRevert();
    virtual bool sSave();
    virtual bool sSaveAs();
    virtual bool sSaveFile();
    virtual bool sSaveToDB();

  protected:
    xTupleDesigner *_designer;

    QActionGroup *_editActions;
    QActionGroup *_fileActions;
    QActionGroup *_formActions;
    QActionGroup *_toolActions;

};

#endif
