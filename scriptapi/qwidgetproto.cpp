/*
 *This file is part of the xTuple ERP: PostBooks Edition, a free and
 *open source Enterprise Resource Planning software suite,
 *Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 *It is licensed to you under the Common Public Attribution License
 *version 1.0, the full text of which(including xTuple-specific Exhibits)
 *is available at www.xtuple.com/CPAL.  By using this software, you agree
 *to be bound by its terms.
 */

#include "qwidgetproto.h"

#include <QApplication>

#define DEBUG true

QScriptValue scriptFind(QScriptContext *context, QScriptEngine  * engine)
{
#ifndef Q_WS_WIN
  if (context->argumentCount() >= 1 &&
      qscriptvalue_cast<WId>(context->argument(0)))
    return engine->toScriptValue(QWidget::find(qscriptvalue_cast<WId>(context->argument(0))));
#endif
  return 0;
}

QScriptValue scriptKeyboardGrabber(QScriptContext * /*context*/,
                                    QScriptEngine  *engine)
{
  return engine->toScriptValue(QWidget::keyboardGrabber());
}

QScriptValue scriptMouseGrabber(QScriptContext * /*context*/,
                                 QScriptEngine  *engine)
{
  return engine->toScriptValue(QWidget::mouseGrabber());
}

QScriptValue scriptSetTabOrder(QScriptContext *context,
                               QScriptEngine  * /*engine*/)
{
  if (context->argumentCount() >= 2 &&
      qscriptvalue_cast<QWidget*>(context->argument(0)) &&
      qscriptvalue_cast<QWidget*>(context->argument(1)))
  {
    if (DEBUG) qDebug("scriptSetTabOrder(%p, %p)",
                      qscriptvalue_cast<QWidget*>(context->argument(0)),
                      qscriptvalue_cast<QWidget*>(context->argument(1)));
    QWidget::setTabOrder(qscriptvalue_cast<QWidget*>(context->argument(0)),
                         qscriptvalue_cast<QWidget*>(context->argument(1)));
  }
  else
    context->throwError(QScriptContext::UnknownError,
                        "could not set tab order as requested");
  return QScriptValue();
}

void setupQWidgetProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QWidgetProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QWidget*>(), proto);

  QScriptValue constructor = engine->newFunction(constructQWidget,
                                                 proto);
  engine->globalObject().setProperty("QWidget", constructor);

  constructor.setProperty("find",           engine->newFunction(scriptFind));
  constructor.setProperty("keyboardGrabber",engine->newFunction(scriptKeyboardGrabber));
  constructor.setProperty("mouseGrabber",   engine->newFunction(scriptMouseGrabber));
  constructor.setProperty("setTabOrder",    engine->newFunction(scriptSetTabOrder));
}

QScriptValue constructQWidget(QScriptContext *context,
                              QScriptEngine  *engine)
{
  QWidget *obj = 0;
  if (context->argumentCount() >= 2)
    obj = new QWidget(qscriptvalue_cast<QWidget*>(context->argument(0)),
                      (Qt::WindowFlags)(context->argument(1).toInt32()));
  else if (context->argumentCount() == 1)
    obj = new QWidget(qscriptvalue_cast<QWidget*>(context->argument(0)));
  else
    obj = new QWidget();
  return engine->toScriptValue(obj);
}

QWidgetProto::QWidgetProto(QObject *parent)
    : QObject(parent)
{
}

QList<QAction*> QWidgetProto::actions() const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->actions();
  return QList<QAction*>();
}

void QWidgetProto::activateWindow()
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->activateWindow();
}

void QWidgetProto::addAction(QAction *action)
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->addAction(action);
}

void QWidgetProto::addActions(QList<QAction *> actions)
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->addActions(actions);
}

void QWidgetProto::adjustSize()
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->adjustSize();
}

int QWidgetProto::backgroundRole() const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->backgroundRole();
  return 0;
}

QWidget *QWidgetProto::childAt(int x, int y) const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->childAt(x, y);
  return 0;
}

QWidget *QWidgetProto::childAt(const QPoint &p) const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->childAt(p);
  return 0;
}

void QWidgetProto::clearFocus()
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->clearFocus();
}

void QWidgetProto::clearMask()
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->clearMask();
}

QRect QWidgetProto::contentsRect() const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->contentsRect();
  return QRect();
}

#ifndef Q_WS_WIN
WId QWidgetProto::effectiveWinId() const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->effectiveWinId();
  return WId();
}
#endif

void QWidgetProto::ensurePolished() const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->ensurePolished();
}

QWidget *QWidgetProto::focusProxy() const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->focusProxy();
  return 0;
}

QWidget *QWidgetProto::focusWidget() const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->focusWidget();
  return 0;
}

QFontInfo QWidgetProto::fontInfo() const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->fontInfo();
  return QFontInfo(QApplication::font());
}

QFontMetrics QWidgetProto::fontMetrics() const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->fontMetrics();
  return QFontMetrics(QApplication::font());
}

int QWidgetProto::foregroundRole() const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->foregroundRole();
  return 0;
}

void QWidgetProto::getContentsMargins(int *left, int * top, int * right, int * bottom) const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->getContentsMargins(left, top, right, bottom);
}

void QWidgetProto::grabKeyboard()
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->grabKeyboard();
}

void QWidgetProto::grabMouse()
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->grabMouse();
}

void QWidgetProto::grabMouse(const QCursor &cursor)
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->grabMouse(cursor);
}

int QWidgetProto::grabShortcut(const QKeySequence &key, int context)
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->grabShortcut(key, (Qt::ShortcutContext)context);
  return 0;
}

QGraphicsProxyWidget *QWidgetProto::graphicsProxyWidget() const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->graphicsProxyWidget();
  return 0;
}

/*
bool QWidgetProto::hasEditFocus() const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->hasEditFocus();
  return false;
}
*/

bool QWidgetProto::hasFocus() const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->hasFocus();
  return false;
}

int QWidgetProto::heightForWidth(int w) const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->heightForWidth(w);
  return 0;
}

QInputContext *QWidgetProto::inputContext()
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->inputContext();
  return 0;
}

QVariant QWidgetProto::inputMethodQuery(int query) const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->inputMethodQuery((Qt::InputMethodQuery)query);
  return QVariant();
}

void QWidgetProto::insertAction(QAction *before, QAction *action)
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->insertAction(before, action);
}

void QWidgetProto::insertActions(QAction *before, QList<QAction *> actions)
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->insertActions(before, actions);
}

bool QWidgetProto::isAncestorOf(const QWidget *child) const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->isAncestorOf(child);
  return false;
}

bool QWidgetProto::isEnabledTo(QWidget *ancestor) const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->isEnabledTo(ancestor);
  return false;
}

bool QWidgetProto::isHidden() const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->isHidden();
  return false;
}

bool QWidgetProto::isVisibleTo(QWidget *ancestor) const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->isVisibleTo(ancestor);
  return false;
}

bool QWidgetProto::isWindow() const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->isWindow();
  return false;
}

QLayout *QWidgetProto::layout() const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->layout();
  return 0;
}

QPoint QWidgetProto::mapFrom(QWidget *parent, const QPoint &pos) const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->mapFrom(parent, pos);
  return QPoint();
}

QPoint QWidgetProto::mapFromGlobal(const QPoint &pos) const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->mapFromGlobal(pos);
  return QPoint();
}

QPoint QWidgetProto::mapFromParent(const QPoint &pos) const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->mapFromParent(pos);
  return QPoint();
}

QPoint QWidgetProto::mapTo(QWidget *parent, const QPoint &pos)   const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->mapTo(parent, pos);
  return QPoint();
}

QPoint QWidgetProto::mapToGlobal(const QPoint &pos) const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->mapToGlobal(pos);
  return QPoint();
}

QPoint QWidgetProto::mapToParent(const QPoint &pos) const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->mapToParent(pos);
  return QPoint();
}

QRegion QWidgetProto::mask() const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->mask();
  return QRegion();
}

QSize QWidgetProto::minimumSizeHint() const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->minimumSizeHint();
  return QSize();
}

QWidget *QWidgetProto::nativeParentWidget() const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->nativeParentWidget();
  return 0;
}

QWidget *QWidgetProto::nextInFocusChain() const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->nextInFocusChain();
  return 0;
}

void QWidgetProto::overrideWindowFlags(int flags)
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->overrideWindowFlags((Qt::WindowFlags)flags);
}

QPaintEngine *QWidgetProto::paintEngine() const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->paintEngine();
  return 0;
}

QWidget *QWidgetProto::parentWidget() const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->parentWidget();
  return 0;
}

void QWidgetProto::releaseKeyboard()
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->releaseKeyboard();
}

void QWidgetProto::releaseMouse()
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->releaseMouse();
}

void QWidgetProto::releaseShortcut(int id)
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->releaseShortcut(id);
}

void QWidgetProto::removeAction(QAction *action)
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->removeAction(action);
}

void QWidgetProto::repaint(int x, int y, int w, int h)
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->repaint(x, y, w, h);
}

void QWidgetProto::repaint(const QRect &rect)
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->repaint(rect);
}

void QWidgetProto::repaint(const QRegion &rgn)
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->repaint(rgn);
}

bool QWidgetProto::restoreGeometry(const QByteArray &geometry)
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->restoreGeometry(geometry);
  return false;
}

QByteArray QWidgetProto::saveGeometry() const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->saveGeometry();
  return QByteArray();
}

void QWidgetProto::scroll(int dx, int dy)
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->scroll(dx, dy);
}

void QWidgetProto::scroll(int dx, int dy, const QRect &r)
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->scroll(dx, dy, r);
}

void QWidgetProto::setAttribute(int attribute, bool on)
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->setAttribute((Qt::WidgetAttribute)attribute, on);
}

void QWidgetProto::setBackgroundRole(int role)
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->setBackgroundRole((QPalette::ColorRole)role);
}

void QWidgetProto::setContentsMargins(int left, int top, int right, int bottom)
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->setContentsMargins(left, top, right, bottom);
}

/*
void QWidgetProto::setEditFocus(bool enable)
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->setEditFocus(enable);
}
*/

void QWidgetProto::setFixedHeight(int h)
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->setFixedHeight(h);
}

void QWidgetProto::setFixedSize(const QSize &s)
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->setFixedSize(s);
}

void QWidgetProto::setFixedSize(int w, int h)
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->setFixedSize(w, h);
}

void QWidgetProto::setFixedWidth(int w)
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->setFixedWidth(w);
}

void QWidgetProto::setFocus(int reason)
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->setFocus((Qt::FocusReason)reason);
}

void QWidgetProto::setFocusProxy(QWidget *w)
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->setFocusProxy(w);
}

void QWidgetProto::setForegroundRole(int role)
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->setForegroundRole((QPalette::ColorRole)role);
}

void QWidgetProto::setInputContext(QInputContext *context)
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->setInputContext(context);
}

void QWidgetProto::setLayout(QLayout *layout)
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->setLayout(layout);
}

void QWidgetProto::setMask(const QBitmap &bitmap)
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->setMask(bitmap);
}

void QWidgetProto::setMask(const QRegion &region)
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->setMask(region);
}

void QWidgetProto::setParent(QWidget *parent)
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->setParent(parent);
}

void QWidgetProto::setParent(QWidget *parent, int f)
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->setParent(parent, (Qt::WindowFlags)f);
}

void QWidgetProto::setShortcutAutoRepeat(int id, bool enable)
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->setShortcutAutoRepeat(id, enable);
}

void QWidgetProto::setShortcutEnabled(int id, bool enable)
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->setShortcutEnabled(id, enable);
}

void QWidgetProto::setStyle(QStyle *style)
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->setStyle(style);
}

void QWidgetProto::setWindowRole(const QString &role)
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->setWindowRole(role);
}

void QWidgetProto::setWindowState(int windowState)
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->setWindowState((Qt::WindowStates)windowState);
}

void QWidgetProto::stackUnder(QWidget *w)
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->stackUnder(w);
}

QStyle *QWidgetProto::style() const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->style();
  return 0;
}

bool QWidgetProto::testAttribute(int attribute) const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->testAttribute((Qt::WidgetAttribute)attribute);
  return false;
}

bool QWidgetProto::underMouse() const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->underMouse();
  return false;
}

void QWidgetProto::unsetCursor()
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->unsetCursor();
}

void QWidgetProto::update(int x, int y, int w, int h)
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->update(x, y, w, h);
}

void QWidgetProto::update(const QRect &rect)
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->update(rect);
}

void QWidgetProto::update(const QRegion &rgn)
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->update(rgn);
}

void QWidgetProto::updateGeometry()
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    item->updateGeometry();
}

QRegion QWidgetProto::visibleRegion() const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->visibleRegion();
  return QRegion();
}

#ifndef Q_WS_WIN
WId QWidgetProto::winId() const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->winId();
  return WId();
}
#endif

QWidget *QWidgetProto::window() const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->window();
  return 0;
}

QString QWidgetProto::windowRole() const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->windowRole();
  return QString();
}

int QWidgetProto::windowState() const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->windowState();
  return 0;
}

int QWidgetProto::windowType() const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return item->windowType();
  return 0;
}

QString QWidgetProto::toString() const
{
  QWidget *item = qscriptvalue_cast<QWidget*>(thisObject());
  if (item)
    return QString(item->objectName());
  return QString();
}

