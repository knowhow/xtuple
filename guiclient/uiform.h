/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef UIFORM_H
#define UIFORM_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_uiform.h"

class QDesignerFormWindowManagerInterface;
class QDesignerFormWindowInterface;
class QDesignerObjectInspectorInterface;
class QDesignerPropertyEditorInterface;
class QDesignerWidgetBoxInterface;
class XWidget;

class uiform : public XWidget, public Ui::uiform
{
    Q_OBJECT

public:
    uiform(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~uiform();
    virtual bool changed();
    static  bool saveFile(const QString &source, QString &filename);

public slots:
    virtual enum SetResponse set(const ParameterList & pParams );
    virtual void populate();
    virtual void close();
    virtual void sCmdDelete();
    virtual void sCmdEdit();
    virtual void sCmdNew();
    virtual void sEdit();
    virtual void sExport();
    virtual void sFillList();
    virtual void sImport();
    virtual void sSave();
    virtual void sScriptDelete();
    virtual void sScriptEdit();
    virtual void sScriptNew();
    virtual void setFormId(int);
    virtual void setSource(QString);

protected slots:
    virtual void languageChange();
    virtual void setMode(const int);

private:
    bool                                 _changed;
    QWidget                             *_designer;
    QDesignerFormWindowManagerInterface *_designerwm;
    XWidget                             *_editor;
    QDesignerFormWindowInterface        *_formwindow;
    int                                  _mode;
    QDesignerObjectInspectorInterface   *_objinsp;
    int                                  _pkgheadidOrig;
    QDesignerPropertyEditorInterface    *_propeditor;
    QWidget                             *_sloteditor;
    QString                              _source;
    int                                  _uiformid;
    QDesignerWidgetBoxInterface         *_widgetbox;


};

#endif // UIFORM_H
