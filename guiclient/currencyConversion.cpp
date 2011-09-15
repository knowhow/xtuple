/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "currencyConversion.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include "xcombobox.h"

// perhaps this should be a generalized XDoubleValidator, but for now
// it should match the definition of the curr_rate column in curr_rate table
// with one difference - the number of decimal places should be one less than
// the scale of curr_rate.curr_rate (see Mantis bug 3901).

class RateValidator : public QDoubleValidator {
  public:
      RateValidator(QObject*, const char* = 0);
      void fixup(QString& ) const;
};

RateValidator::RateValidator(QObject* parent, const char* name) :
  QDoubleValidator(0.00001, 99999.99999, 5, parent)
{
  if (name)
    setObjectName(name);
}

void RateValidator::fixup ( QString & input ) const
{
  if (input.isEmpty())
      return;

  double rateDouble = input.toDouble();
  if (rateDouble < bottom())
      rateDouble = bottom();
  else if (rateDouble > top())
      rateDouble = top();

  input.setNum(rateDouble, 'f', decimals());
}

currencyConversion::currencyConversion(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(_sClose()));
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(_sSave()));
  connect(_rate, SIGNAL(lostFocus()), this, SLOT(sFixRate()));

  _currency->setType(XComboBox::CurrenciesNotBase);
  _rate->setValidator(new RateValidator (_rate) );
}

currencyConversion::~currencyConversion()
{
  // no need to delete child widgets, Qt does it all for us
}

void currencyConversion::languageChange()
{
  retranslateUi(this);
}

enum SetResponse currencyConversion::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("curr_rate_id", &valid);
  if (valid)
  {
      _curr_rate_id = param.toInt();
  }
  else
      _curr_rate_id = 0;

  param = pParams.value("curr_id", &valid);
  if (valid)
  {
      _curr_id = param.toInt();
  }
  else
  {
      _curr_id = 0;
  }

  populate();

  param = pParams.value("mode", &valid);
  if (valid)
  {
      if (param.toString() == "new")
      {
          _mode = cNew;
          _currency->setFocus();
      }
    else if (param.toString() == "edit")
    {
        _mode = cEdit;
        _currency->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      
      _currency->setEnabled(FALSE);
      _rate->setEnabled(FALSE);
      _dateCluster->setEnabled(FALSE);
      
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
      _buttonBox->setFocus();
    }
  }

  return NoError;
}

void currencyConversion::_sClose()
{
  done(_curr_rate_id);
}

void currencyConversion::_sSave()
{
  if (! _currency->isValid())
  {
      QMessageBox::warning(this, tr("Missing Currency"),
                           tr("Please specify a currency for this exchange rate."));
      _currency->setFocus();
      return;
  }

  if (_rate->toDouble() == 0)
  {
    QMessageBox::warning(this, tr("No Rate Specified"),
      tr("You must specify a Rate that is not zero.") );
    return;
  }
  
  if (!_dateCluster->startDate().isValid())
  {
      QMessageBox::warning( this, tr("Missing Start Date"),
                            tr("Please specify a Start Date for this exchange rate."));
      _dateCluster->setFocus();
      return;
  }

  if (!_dateCluster->endDate().isValid())
  {
      QMessageBox::warning( this, tr("Missing End Date"),
                            tr("Please specify an End Date for this exchange rate. "));
      _dateCluster->setFocus();
      return;
  }

  if (_dateCluster->startDate() > _dateCluster->endDate())
  {
      QMessageBox::warning(this, tr("Invalid Date Order"),
                          tr("The Start Date for this exchange rate is "
                             "later than the End Date.\n"
                             "Please check the values of these dates."));
      _dateCluster->setFocus();
      return;
  }
  
  q.prepare( "SELECT count(*) AS numberOfOverlaps "
             "FROM curr_rate "
             "WHERE curr_id = :curr_id"
             "  AND curr_rate_id != :curr_rate_id"
             "  AND ( (curr_effective BETWEEN :curr_effective AND :curr_expires OR"
             "         curr_expires BETWEEN :curr_effective AND :curr_expires)"
             "   OR   (curr_effective <= :curr_effective AND"
             "         curr_expires   >= :curr_expires) );" );
  q.bindValue(":curr_rate_id", _curr_rate_id);
  q.bindValue(":curr_id", _currency->id());
  q.bindValue(":curr_effective", _dateCluster->startDate());
  q.bindValue(":curr_expires", _dateCluster->endDate());
  q.exec();
  if (q.first())
  {
    if (q.value("numberOfOverlaps").toInt() > 0)
    {
      QMessageBox::warning(this, tr("Invalid Date Range"),
                          tr("The date range overlaps with  "
                             "another date range.\n"
                             "Please check the values of these dates."));
      _dateCluster->setFocus();
      return;
    }
  }

  QString inverter("");
  if (_metrics->value("CurrencyExchangeSense").toInt() == 1)
      inverter = "1 / ";

  QString sql;
  if (_mode == cNew)
      sql = QString("INSERT INTO curr_rate "
                    "(curr_id, curr_rate, curr_effective, curr_expires) "
                    "VALUES "
                    "(:curr_id, %1 CAST(:curr_rate AS NUMERIC), "
                    " :curr_effective, :curr_expires)")
                  .arg(inverter);
  else if (_mode == cEdit)
      sql = QString("UPDATE curr_rate SET "
                    "curr_id = :curr_id, "
                    "curr_rate = %1 CAST(:curr_rate AS NUMERIC), "
                    "curr_effective = :curr_effective, "
                    "curr_expires = :curr_expires "
                    "WHERE curr_rate_id = :curr_rate_id")
                    .arg(inverter);


  q.prepare(sql);
  q.bindValue(":curr_rate_id", _curr_rate_id);
  q.bindValue(":curr_id", _currency->id());
  q.bindValue(":curr_rate", _rate->toDouble());
  q.bindValue(":curr_effective", _dateCluster->startDate());
  q.bindValue(":curr_expires", _dateCluster->endDate());
  
  q.exec();

  if (q.lastError().type() != QSqlError::NoError)
  {
      QMessageBox::critical(this, tr("A System Error occurred at %1::%2.")
                            .arg(__FILE__)
                            .arg(__LINE__),
                            q.lastError().databaseText());
      return;
  }

  done(_curr_rate_id);
}

void currencyConversion::populate()
{
  QString rateString;

  if (_curr_rate_id)
  {
      QString inverter("");
      if (_metrics->value("CurrencyExchangeSense").toInt() == 1)
          inverter = "1 / ";
      QString sql = QString("SELECT curr_id, %1 curr_rate AS curr_rate, "
                            "curr_effective, curr_expires "
                            "FROM curr_rate "
                            "WHERE curr_rate_id = :curr_rate_id;")
                            .arg(inverter);
      q.prepare(sql);
      q.bindValue(":curr_rate_id", _curr_rate_id);
      q.exec();
      {
          if (q.first())
          {
              _currency->setId(q.value("curr_id").toInt());
              _dateCluster->setStartDate(q.value("curr_effective").toDate());
              _dateCluster->setEndDate(q.value("curr_expires").toDate());
              _rate->setText(rateString.setNum(q.value("curr_rate").toDouble(), 'f', 5));
          }
      }
  }
  if (_curr_id)
  {
      _currency->setId(_curr_id);
  }
}

void currencyConversion::sFixRate()
{
  QString rateStr = _rate->text();
  RateValidator* rateValidator = new RateValidator(_rate);
  int dummy = 0;

  if (rateValidator->validate(rateStr, dummy) == QValidator::Intermediate)
  {
      rateValidator->fixup(rateStr);
      _rate->setText(rateStr);
  }
}
