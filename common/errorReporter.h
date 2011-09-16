/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QObject>
#include <QSqlError>
#include <QSqlQuery>

class ErrorReporterPrivate;
class QWidget;

class ErrorReporter : public QObject
{
  Q_OBJECT

  public:
    static bool error(const QString &err, const QString file = QString(), int line = -1);
    static bool error(QtMsgType type, QWidget *parent, const QString title, const QString &err, const QString file = QString(), int line = -1);
    static bool error(QtMsgType type, QWidget *parent, const QString title, const QSqlError &err, const QString file = QString(), int line = -1);
    static bool error(QtMsgType type, QWidget *parent, const QString title, const QSqlQuery &qry, const QString file = QString(), int line = -1);

  protected:
    ErrorReporter(QObject *parent = 0);
    ~ErrorReporter();
    static ErrorReporter *reporter();

    ErrorReporterPrivate *_private;
    static ErrorReporter *_singleton;

};
