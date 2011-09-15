/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QtGui>
#include <QComboBox>

#include "characteristic.h"
#include "custCharacteristicDelegate.h"
#include "datecluster.h"
#include "guiclient.h"
#include "xt.h"

CustCharacteristicDelegate::CustCharacteristicDelegate(QObject *parent)
  : QItemDelegate(parent)
{
}

QWidget *CustCharacteristicDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem & /*style*/,
    const QModelIndex & index) const
{
  if(index.column()!=1)
    return 0;

  QModelIndex idx = index.sibling(index.row(), 0);
  characteristic::Type chartype = characteristic::Text;

  // Determine what type we have
  XSqlQuery qry;
  qry.prepare("SELECT char_id, char_type "
              "FROM char "
              "WHERE (char_id=:char_id); ");
  qry.bindValue(":char_id", idx.model()->data(idx, Qt::UserRole));
  qry.exec();
  if (qry.first())
    chartype = (characteristic::Type)qry.value("char_type").toInt();

  if (chartype == characteristic::Text ||
      chartype == characteristic::List)
  {
    q.prepare("SELECT charass_value "
              "FROM char, charass "
              "  LEFT OUTER JOIN charopt ON ((charopt_char_id=charass_char_id) "
              "                          AND (charopt_value=charass_value)) "
              "WHERE ((charass_char_id=char_id)"
              "  AND  (charass_target_type='CT')"
              "  AND  (charass_target_id=:custtype_id)"
              "  AND  (char_id=:char_id) ) "
              "ORDER BY COALESCE(charopt_order,0), charass_value;");
    q.bindValue(":char_id", idx.model()->data(idx, Qt::UserRole));
    q.bindValue(":custtype_id", index.model()->data(index, Xt::IdRole));
    q.exec();

    QComboBox *editor = new QComboBox(parent);
    editor->setEditable(chartype == characteristic::Text);


#ifdef Q_WS_MAC
    QFont boxfont = editor->font();
    boxfont.setPointSize((boxfont.pointSize() == -1) ? boxfont.pixelSize() - 3 : boxfont.pointSize() - 3);
    editor->setFont(boxfont);
#endif

    while(q.next())
      editor->addItem(q.value("charass_value").toString());
    editor->installEventFilter(const_cast<CustCharacteristicDelegate*>(this));

    return editor;
  }
  else if (chartype == characteristic::Date)
  {
    DLineEdit *editor = new DLineEdit(parent);
    return editor;
  }
  return 0;
}

void CustCharacteristicDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
  if (editor->inherits("QComboBox"))
  {
    QString value = index.model()->data(index, Qt::DisplayRole).toString();

    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    int curIdx = comboBox->findText(value);

    if(curIdx != -1)
      comboBox->setCurrentIndex(curIdx);
    else
      comboBox->setEditText(value);
  }
  else if (editor->inherits("DLineEdit"))
  {
    QDate date = index.model()->data(index, Qt::UserRole).toDate();

    DLineEdit *dlineedit = static_cast<DLineEdit*>(editor);
    dlineedit->setDate(date);
  }
}

void CustCharacteristicDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
  if (editor->inherits("QComboBox"))
  {
    QComboBox *comboBox = static_cast<QComboBox*>(editor);

    model->setData(index, comboBox->currentText());
  }
  else if (editor->inherits("DLineEdit"))
  {
    DLineEdit *dlineedit = static_cast<DLineEdit*>(editor);

    model->setData(index, dlineedit->date());
  }
}

void CustCharacteristicDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
  editor->setGeometry(option.rect);
}

