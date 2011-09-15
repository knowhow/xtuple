/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef XTUPLEDESIGNER_H
#define XTUPLEDESIGNER_H

#include "guiclient.h"
#include "xmainwindow.h"

class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;
class QDesignerFormWindowManagerInterface;
class QDesignerIntegrationInterface;
class QDesignerObjectInspectorInterface;
class QDesignerPropertyEditorInterface;
class QDesignerWidgetBoxInterface;
class QIODevice;
class QMenu;
class WidgetBoxWindow;
class xTupleDesignerActions;

class xTupleDesigner : public XMainWindow
{
    Q_OBJECT

  public:
    xTupleDesigner(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = 0);
    ~xTupleDesigner();
    QDesignerFormEditorInterface *formeditor()     { return _formeditor;  }
    virtual bool                  formEnabled()    { return _formEnabled; }
    virtual int                   formId()         { return _formId;      }
    QDesignerFormWindowInterface *formwindow()     { return _formwindow;  }
    virtual QString               name();
    virtual QString               notes() const    { return _notes;       }
    virtual int                   order()          { return _order;       }
    virtual XMainWindow          *objinspwindow()  { return _objinspwindow;  }
    virtual XMainWindow          *propinspwindow() { return _propinspwindow; }
    virtual XMainWindow          *slotedwindow()   { return _slotedwindow;   }
    virtual QString               source();
    virtual XMainWindow          *widgetwindow()   { return _widgetwindow;   }

  public slots:
    virtual void setFormEnabled(bool p);
    virtual void setFormId(int p);
    virtual void setNotes(QString p);
    virtual void setOrder(int p);
    virtual void setSource(QIODevice *, QString = QString());
    virtual void setSource(QString);
    virtual void sRevert()              { setSource(_source); }

  signals:
    void formEnabledChanged(bool);
    void formIdChanged(int);
    void nameChanged(QString);
    void notesChanged(QString);
    void orderChanged(int);
    void sourceChanged(QString);

  protected:
    virtual void closeEvent(QCloseEvent *);

  private:
    friend class WidgetBoxWindow;
    QWidget                             *_designer;
    static QDesignerFormEditorInterface *_formeditor;
    bool                                 _formEnabled;
    int                                  _formId;
    QDesignerFormWindowInterface        *_formwindow;
    QDesignerIntegrationInterface       *_integration;
    QString                              _notes;
    XMainWindow                         *_objinspwindow;
    int                                  _order;
    XMainWindow                         *_propinspwindow;
    XMainWindow                         *_slotedwindow;
    QIODevice                           *_source;
    XMainWindow                         *_widgetwindow;

    QMenuBar                            *_menubar;
    QMenu                               *_editmenu;
    QMenu                               *_filemenu;
    QMenu                               *_formmenu;
    QMenu                               *_toolmenu;

    xTupleDesignerActions               *_actions;
};

#endif // XTUPLEDESIGNER_H
