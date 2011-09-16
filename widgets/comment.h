/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#ifndef comment_h
#define comment_h

#include <QDialog>

#include <xsqlquery.h>

#include "widgets.h"
#include "comments.h"

class XComboBox;
class XTextEdit;
class QPushButton;
class ParameterList;

class XTUPLEWIDGETS_EXPORT comment : public QDialog
{
    Q_OBJECT

    friend class Comments;

  public:
    comment(QWidget * = 0, const char * = 0, bool = FALSE, Qt::WindowFlags = 0);

    XComboBox* _cmnttype;
    XTextEdit* _comment;
    QPushButton* _close;
    QPushButton* _save;
    QPushButton* _next;
    QPushButton* _prev;
    QPushButton* _more;
    Comments* _comments;

  public slots:
    virtual void set( ParameterList & pParams );
    virtual void sSave();
    virtual void populate();
    virtual void sNextComment();
    virtual void sPrevComment();

  private:
    XSqlQuery _query;
    QList <QVariant> _commentIDList;
    int _commentLocation;
    int _commentid;
    int _targetId;
    int _mode;
    enum Comments::CommentSources _source;
};

#endif 
