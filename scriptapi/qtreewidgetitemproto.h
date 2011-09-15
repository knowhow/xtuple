/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QTREEWIDGETITEMPROTO_H__
#define __QTREEWIDGETITEMPROTO_H__

#include <QTreeWidgetItem>
#include <QBrush>
#include <QFont>
#include <QIcon>
#include <QList>
#include <QVariant>
#include <QSize>
#include <QDataStream>
#include <QString>
#include <QObject>
#include <QtScript>

Q_DECLARE_METATYPE(QTreeWidgetItem*)
Q_DECLARE_METATYPE(QTreeWidgetItem)

void setupQTreeWidgetItemProto(QScriptEngine *engine);
QScriptValue constructQTreeWidgetItem(QScriptContext *context, QScriptEngine *engine);

class QTreeWidgetItemProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QTreeWidgetItemProto(QObject *parent);

    Q_INVOKABLE void             addChild(QTreeWidgetItem * child);
    Q_INVOKABLE void             addChildren(const QList<QTreeWidgetItem *> &children);
    Q_INVOKABLE QBrush           background(int column)	        const;
    Q_INVOKABLE int              checkState(int column)	        const;
    Q_INVOKABLE QTreeWidgetItem *child(int index)	        const;
    Q_INVOKABLE int              childCount()	                const;
    Q_INVOKABLE int              childIndicatorPolicy()	        const;
    Q_INVOKABLE QTreeWidgetItem *clone()	                const;
    Q_INVOKABLE int              columnCount()	                const;
    Q_INVOKABLE QVariant         data(int column, int role)	const;
    Q_INVOKABLE int              flags()	                const;
    Q_INVOKABLE QFont            font(int column)	        const;
    Q_INVOKABLE QBrush           foreground(int column)	        const;
    Q_INVOKABLE QIcon            icon(int column)	        const;
    Q_INVOKABLE int              indexOfChild(QTreeWidgetItem *child)	const;
    Q_INVOKABLE void             insertChild(int index, QTreeWidgetItem *child);
    Q_INVOKABLE void             insertChildren(int index, const QList<QTreeWidgetItem *> &children);
    Q_INVOKABLE bool             isDisabled()	        const;
    Q_INVOKABLE bool             isExpanded()	        const;
    Q_INVOKABLE bool             isFirstColumnSpanned()	const;
    Q_INVOKABLE bool             isHidden()	        const;
    Q_INVOKABLE bool             isSelected()	        const;
    Q_INVOKABLE QTreeWidgetItem *parent()	        const;
    Q_INVOKABLE void             read(QDataStream &in);
    Q_INVOKABLE void             removeChild(QTreeWidgetItem *child);
    Q_INVOKABLE void             setBackground(int column, const QBrush &brush);
    Q_INVOKABLE void             setCheckState(int column, int state);
    Q_INVOKABLE void             setChildIndicatorPolicy(int policy);
    Q_INVOKABLE void             setData(int column, int role, const QVariant &value);
    Q_INVOKABLE void             setDisabled(bool disabled);
    Q_INVOKABLE void             setExpanded(bool expand);
    Q_INVOKABLE void             setFirstColumnSpanned(bool span);
    Q_INVOKABLE void             setFlags(int flags);
    Q_INVOKABLE void             setFont(int column, const QFont &font);
    Q_INVOKABLE void             setForeground(int column, const QBrush &brush);
    Q_INVOKABLE void             setHidden(bool hide);
    Q_INVOKABLE void             setIcon(int column, const QIcon &icon);
    Q_INVOKABLE void             setSelected(bool select);
    Q_INVOKABLE void             setSizeHint(int column, const QSize &size);
    Q_INVOKABLE void             setStatusTip(int column, const QString &statusTip);
    Q_INVOKABLE void             setText(int column, const QString &text);
    Q_INVOKABLE void             setTextAlignment(int column, int alignment);
    Q_INVOKABLE void             setToolTip(int column, const QString &toolTip);
    Q_INVOKABLE void             setWhatsThis(int column, const QString &whatsThis);
    Q_INVOKABLE QSize            sizeHint(int column)	        const;
    Q_INVOKABLE void             sortChildren(int column, int order);
    Q_INVOKABLE QString          statusTip(int column)	        const;
    Q_INVOKABLE QTreeWidgetItem *takeChild(int index);
    Q_INVOKABLE QList<QTreeWidgetItem *> takeChildren();
    Q_INVOKABLE QString          text(int column)	        const;
    Q_INVOKABLE int              textAlignment(int column)	const;
    Q_INVOKABLE QString          toolTip(int column)	        const;
    Q_INVOKABLE QTreeWidget     *treeWidget()	                const;
    Q_INVOKABLE int              type()	                        const;
    Q_INVOKABLE QString          whatsThis(int column)	        const;
    Q_INVOKABLE void             write(QDataStream &out)	const;

    Q_INVOKABLE QString          toString()    	                const;
};

#endif
