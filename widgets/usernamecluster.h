/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __USERNAMECLUSTER_H__
#define __USERNAMECLUSTER_H__

#include "virtualCluster.h"

#include "widgets.h"

class QLabel;
class QPushButton;
class QScriptEngine;

class UsernameList;
class UsernameSearch;
class UsernameCluster;

void setupUsernameLineEdit(QScriptEngine *engine);
void setupUsernameCluster(QScriptEngine *engine);

class XTUPLEWIDGETS_EXPORT UsernameList : public VirtualList
{
  Q_OBJECT

public:
  UsernameList(QWidget*, Qt::WindowFlags = 0);
};

class XTUPLEWIDGETS_EXPORT UsernameSearch : public VirtualSearch
{
  Q_OBJECT

public:
  UsernameSearch(QWidget*, Qt::WindowFlags = 0);
};

class XTUPLEWIDGETS_EXPORT UsernameLineEdit : public VirtualClusterLineEdit
{
  Q_OBJECT

  friend class UsernameCluster;

  public:
    UsernameLineEdit(QWidget*, const char* = 0);

    enum Type {
      UsersAll = 0,
      UsersActive,
      UsersInactive
    };

    Q_INVOKABLE inline enum Type type() const { return _type; }
    Q_INVOKABLE void setType(enum Type pType);

    Q_INVOKABLE int id();
    Q_INVOKABLE const QString & username();

  public slots:
    void setId(const int);
    void setUsername(const QString &);
    void clear();
    void sParse();

  protected:
    UsernameList* listFactory();
    UsernameSearch* searchFactory();

  private:
    enum Type _type;
    QString _username;
};

class XTUPLEWIDGETS_EXPORT UsernameCluster : public VirtualCluster
{
  Q_OBJECT

  Q_PROPERTY( QString label READ label WRITE setLabel )

  public:
    UsernameCluster(QWidget*, const char* = 0);

    Q_INVOKABLE inline const QString username() const { return static_cast<UsernameLineEdit *>(_number)->username(); }

    Q_INVOKABLE inline UsernameLineEdit::Type type() const { return static_cast<UsernameLineEdit *>(_number)->type(); }
    Q_INVOKABLE inline void setType(UsernameLineEdit::Type pType) { static_cast<UsernameLineEdit *>(_number)->setType(pType); }

  public slots:
    void setUsername(const QString & pUsername);
    void setReadOnly(const bool);
    inline void setId(const int pId)  { static_cast<UsernameLineEdit *>(_number)->setId(pId);   }

  protected:
    void addNumberWidget(UsernameLineEdit* pNumberWidget);
};

Q_DECLARE_METATYPE(UsernameLineEdit*)
Q_DECLARE_METATYPE(UsernameCluster*)

#endif
