/*
 *This file is part of the xTuple ERP: PostBooks Edition, a free and
 *open source Enterprise Resource Planning software suite,
 *Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 *It is licensed to you under the Common Public Attribution License
 *version 1.0, the full text of which(including xTuple-specific Exhibits)
 *is available at www.xtuple.com/CPAL.  By using this software, you agree
 *to be bound by its terms.
 */

#ifndef __QWIDGETPROTO_H__
#define __QWIDGETPROTO_H__

#include <QAction>
#include <QBitmap>
#include <QByteArray>
#include <QCursor>
#include <QFontInfo>
#include <QFontMetrics>
#include <QGraphicsProxyWidget>
#include <QInputContext>
#include <QKeySequence>
#include <QLayout>
#include <QList>
#include <QObject>
#include <QPaintDevice>
#include <QPaintEngine>
#include <QPainter>
#include <QPalette>
#include <QPoint>
#include <QRect>
#include <QRegion>
#include <QScriptContext>
#include <QScriptEngine>
#include <QScriptValue>
#include <QScriptable>
#include <QSize>
#include <QString>
#include <QStyle>
#include <QVariant>
#include <QWidget>
#include <QtScript>

Q_DECLARE_METATYPE(QWidget*)
//Q_DECLARE_METATYPE(QWidget)

void setupQWidgetProto(QScriptEngine *engine);
QScriptValue constructQWidget(QScriptContext *context, QScriptEngine *engine);

class QWidgetProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QWidgetProto(QObject *parent);

    Q_INVOKABLE QList<QAction *> actions()      const;
    Q_INVOKABLE void activateWindow();
    Q_INVOKABLE void addAction(QAction *action);
    Q_INVOKABLE void addActions(QList<QAction *> actions);
    Q_INVOKABLE void adjustSize();
    Q_INVOKABLE int  backgroundRole()                   const;
    Q_INVOKABLE QWidget *childAt(int x, int y)          const;
    Q_INVOKABLE QWidget *childAt(const QPoint &p)       const;
    Q_INVOKABLE void clearFocus();
    Q_INVOKABLE void clearMask();
    Q_INVOKABLE QRect contentsRect()                    const;
#ifndef Q_WS_WIN
    Q_INVOKABLE WId effectiveWinId()                    const;
#endif
    Q_INVOKABLE void ensurePolished()                   const;
    Q_INVOKABLE QWidget *focusProxy()                   const;
    Q_INVOKABLE QWidget *focusWidget()                  const;
    Q_INVOKABLE QFontInfo fontInfo()                    const;
    Q_INVOKABLE QFontMetrics fontMetrics()              const;
    Q_INVOKABLE int foregroundRole()                    const;
    Q_INVOKABLE void getContentsMargins(int *left, int * top, int * right, int * bottom)        const;
    Q_INVOKABLE void grabKeyboard();
    Q_INVOKABLE void grabMouse();
    Q_INVOKABLE void grabMouse(const QCursor &cursor);
    Q_INVOKABLE int grabShortcut(const QKeySequence &key, int context = Qt::WindowShortcut);
    Q_INVOKABLE QGraphicsProxyWidget *graphicsProxyWidget()     const;
    //Q_INVOKABLE bool hasEditFocus()                           const;
    Q_INVOKABLE bool hasFocus()                                 const;
    Q_INVOKABLE int heightForWidth(int w)                       const;
    Q_INVOKABLE QInputContext *inputContext();
    Q_INVOKABLE QVariant inputMethodQuery(int query)            const;
    Q_INVOKABLE void insertAction(QAction *before, QAction *action);
    Q_INVOKABLE void insertActions(QAction *before, QList<QAction *> actions);
    Q_INVOKABLE bool isAncestorOf(const QWidget *child) const;
    Q_INVOKABLE bool isEnabledTo(QWidget *ancestor)     const;
    Q_INVOKABLE bool isHidden()                         const;
    Q_INVOKABLE bool isVisibleTo(QWidget *ancestor)     const;
    Q_INVOKABLE bool isWindow()                         const;
    Q_INVOKABLE QLayout *layout()                       const;
    Q_INVOKABLE QPoint mapFrom(QWidget *parent, const QPoint &pos) const;
    Q_INVOKABLE QPoint mapFromGlobal(const QPoint &pos) const;
    Q_INVOKABLE QPoint mapFromParent(const QPoint &pos) const;
    Q_INVOKABLE QPoint mapTo(QWidget *parent, const QPoint &pos)   const;
    Q_INVOKABLE QPoint mapToGlobal(const QPoint &pos)   const;
    Q_INVOKABLE QPoint mapToParent(const QPoint &pos)   const;
    Q_INVOKABLE QRegion mask()                          const;
    Q_INVOKABLE QSize minimumSizeHint()                 const;
    Q_INVOKABLE QWidget *nativeParentWidget()           const;
    Q_INVOKABLE QWidget *nextInFocusChain()             const;
    Q_INVOKABLE void overrideWindowFlags(int flags);
    Q_INVOKABLE QPaintEngine *paintEngine()             const;
    Q_INVOKABLE QWidget *parentWidget()                 const;
    Q_INVOKABLE void releaseKeyboard();
    Q_INVOKABLE void releaseMouse();
    Q_INVOKABLE void releaseShortcut(int id);
    Q_INVOKABLE void removeAction(QAction *action);
    Q_INVOKABLE void repaint(int x, int y, int w, int h);
    Q_INVOKABLE void repaint(const QRect &rect);
    Q_INVOKABLE void repaint(const QRegion &rgn);
    Q_INVOKABLE bool restoreGeometry(const QByteArray &geometry);
    Q_INVOKABLE QByteArray saveGeometry()       const;
    Q_INVOKABLE void scroll(int dx, int dy);
    Q_INVOKABLE void scroll(int dx, int dy, const QRect &r);
    Q_INVOKABLE void setAttribute(int attribute, bool on = true);
    Q_INVOKABLE void setBackgroundRole(int role);
    Q_INVOKABLE void setContentsMargins(int left, int top, int right, int bottom);
    //Q_INVOKABLE void setEditFocus(bool enable);
    Q_INVOKABLE void setFixedHeight(int h);
    Q_INVOKABLE void setFixedSize(const QSize &s);
    Q_INVOKABLE void setFixedSize(int w, int h);
    Q_INVOKABLE void setFixedWidth(int w);
    Q_INVOKABLE void setFocus(int reason);
    Q_INVOKABLE void setFocusProxy(QWidget *w);
    Q_INVOKABLE void setForegroundRole(int role);
    Q_INVOKABLE void setInputContext(QInputContext *context);
    Q_INVOKABLE void setLayout(QLayout *layout);
    Q_INVOKABLE void setMask(const QBitmap &bitmap);
    Q_INVOKABLE void setMask(const QRegion &region);
    Q_INVOKABLE void setParent(QWidget *parent);
    Q_INVOKABLE void setParent(QWidget *parent, int f);
    Q_INVOKABLE void setShortcutAutoRepeat(int id, bool enable = true);
    Q_INVOKABLE void setShortcutEnabled(int id, bool enable = true);
    Q_INVOKABLE void setStyle(QStyle *style);
    Q_INVOKABLE void setWindowRole(const QString &role);
    Q_INVOKABLE void setWindowState(int windowState);
    Q_INVOKABLE void stackUnder(QWidget *w);
    Q_INVOKABLE QStyle *style()                         const;
    Q_INVOKABLE bool testAttribute(int attribute)       const;
    Q_INVOKABLE bool underMouse()                       const;
    Q_INVOKABLE void unsetCursor();
    Q_INVOKABLE void update(int x, int y, int w, int h);
    Q_INVOKABLE void update(const QRect &rect);
    Q_INVOKABLE void update(const QRegion &rgn);
    Q_INVOKABLE void updateGeometry();
    Q_INVOKABLE QRegion visibleRegion()         const;
#ifndef Q_WS_WIN
    Q_INVOKABLE WId winId()                     const;
#endif
    Q_INVOKABLE QWidget *window()               const;
    Q_INVOKABLE QString windowRole()            const;
    Q_INVOKABLE int windowState()               const;
    Q_INVOKABLE int windowType()                const;

    Q_INVOKABLE QString     toString()          const;

};

#endif
