/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QAbstractMessageHandler>

class XMessageBoxMessageHandler : public QAbstractMessageHandler
{
  Q_OBJECT

  public:
    XMessageBoxMessageHandler(QObject *p = 0);
    QString lastMessage() const;
    virtual void setPrefix(const QString &prefix);
    virtual void setTitle(const QString &title);
    virtual QString title() const;

  protected:
    virtual void handleMessage(QtMsgType type, const QString &description, const QUrl &identifier, const QSourceLocation &sourceLocation);

  private:
    QString _message;
    QString _prefix;
    QString _title;
};
