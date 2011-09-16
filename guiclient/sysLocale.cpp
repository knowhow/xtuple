/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "sysLocale.h"

#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#define DEBUG true

sysLocale::sysLocale(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_alternate,      SIGNAL(editingFinished()), this, SLOT(sUpdateColors()));
  connect(_costScale,      SIGNAL(valueChanged(int)), this, SLOT(sUpdateSamples()));
  connect(_country,        SIGNAL(newID(int)),        this, SLOT(sUpdateSamples()));
  connect(_currencyScale,  SIGNAL(valueChanged(int)), this, SLOT(sUpdateSamples()));
  connect(_emphasis,       SIGNAL(editingFinished()), this, SLOT(sUpdateColors()));
  connect(_error,          SIGNAL(editingFinished()), this, SLOT(sUpdateColors()));
  connect(_expired,        SIGNAL(editingFinished()), this, SLOT(sUpdateColors()));
  connect(_extPriceScale,  SIGNAL(valueChanged(int)), this, SLOT(sUpdateSamples()));
  connect(_future,         SIGNAL(editingFinished()), this, SLOT(sUpdateColors()));
  connect(_language,       SIGNAL(newID(int)),        this, SLOT(sUpdateCountries()));
  connect(_purchPriceScale,SIGNAL(valueChanged(int)), this, SLOT(sUpdateSamples()));
  connect(_qtyScale,       SIGNAL(valueChanged(int)), this, SLOT(sUpdateSamples()));
  connect(_qtyPerScale,    SIGNAL(valueChanged(int)), this, SLOT(sUpdateSamples()));
  connect(_salesPriceScale,SIGNAL(valueChanged(int)), this, SLOT(sUpdateSamples()));
  connect(_percentScale,   SIGNAL(valueChanged(int)), this, SLOT(sUpdateSamples()));
  connect(_buttonBox,      SIGNAL(accepted()),        this, SLOT(sSave()));
  connect(_uomRatioScale,  SIGNAL(valueChanged(int)), this, SLOT(sUpdateSamples()));
  connect(_warning,        SIGNAL(editingFinished()), this, SLOT(sUpdateColors()));

  _localeid = -1;
}

sysLocale::~sysLocale()
{
  // no need to delete child widgets, Qt does it all for us
}

void sysLocale::languageChange()
{
  retranslateUi(this);
}

enum SetResponse sysLocale::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("locale_id", &valid);
  if (valid)
  {
    _localeid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _code->setFocus();

      q.exec("SELECT NEXTVAL('locale_locale_id_seq') AS _locale_id");
      if (q.first())
        _localeid = q.value("_locale_id").toInt();
      else
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return UndefinedError;
      }
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _description->setFocus();
    }
    else if (param.toString() == "copy")
    {
      _mode = cCopy;

      q.prepare("SELECT copyLocale(:locale_id) AS _locale_id;");
      q.bindValue(":locale_id", _localeid);
      q.exec();
      if (q.first())
      {
        _localeid = q.value("_locale_id").toInt();
	populate();
      }
      else if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return UndefinedError;
      }

      _code->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _code->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _language->setEnabled(FALSE);
      _country->setEnabled(FALSE);
      _currencyScale->setEnabled(FALSE);
      _salesPriceScale->setEnabled(FALSE);
      _purchPriceScale->setEnabled(FALSE);
      _extPriceScale->setEnabled(FALSE);
      _costScale->setEnabled(FALSE);
      _qtyScale->setEnabled(FALSE);
      _qtyPerScale->setEnabled(FALSE);
      _uomRatioScale->setEnabled(FALSE);
      _comments->setReadOnly(TRUE);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
      _buttonBox->setFocus();
    }
  }

  return NoError;
}

void sysLocale::sSave()
{
  if (_code->text().trimmed().length() == 0)
  {
    QMessageBox::critical( this, tr("Cannot Save Locale"),
                           tr("<p>You must enter a Code for this Locale before "
                              "you may save it.") );
    _code->setFocus();
    return;
  }

  q.prepare( "SELECT locale_id "
             "FROM locale "
             "WHERE ( (locale_id<>:locale_id)"
             " AND (UPPER(locale_code)=UPPER(:locale_code)) );");
  q.bindValue(":locale_id", _localeid);
  q.bindValue(":locale_code", _code->text().trimmed());
  q.exec();
  if (q.first())
  {
    QMessageBox::critical( this, tr("Cannot Create Locale"),
			   tr( "A Locale with the entered code already exists."
			       "You may not create a Locale with this code." ) );
    _code->setFocus();
    return;
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  QLocale sampleLocale = generateLocale();

  if (_mode == cNew)
  {
    q.prepare( "INSERT INTO locale "
               "( locale_id, locale_code, locale_descrip,"
               "  locale_lang_id, locale_country_id, "
               "  locale_dateformat, locale_timeformat, locale_timestampformat,"
	       "  locale_intervalformat, locale_qtyformat,"
               "  locale_curr_scale,"
               "  locale_salesprice_scale, locale_purchprice_scale,"
               "  locale_extprice_scale, locale_cost_scale,"
               "  locale_qty_scale, locale_qtyper_scale,"
               "  locale_uomratio_scale, locale_percent_scale, "
               "  locale_comments, "
               "  locale_error_color, locale_warning_color,"
               "  locale_emphasis_color, locale_altemphasis_color,"
               "  locale_expired_color, locale_future_color) "
               "VALUES "
               "( :locale_id, :locale_code, :locale_descrip,"
               "  :locale_lang_id, :locale_country_id,"
               "  :locale_dateformat, :locale_timeformat, :locale_timestampformat,"
	       "  :locale_intervalformat, :locale_qtyformat,"
               "  :locale_curr_scale,"
               "  :locale_salesprice_scale, :locale_purchprice_scale,"
               "  :locale_extprice_scale, :locale_cost_scale,"
               "  :locale_qty_scale, :locale_qtyper_scale,"
               "  :locale_uomratio_scale, :local_percent_scale, "
               "  :locale_comments,"
               "  :locale_error_color, :locale_warning_color,"
               "  :locale_emphasis_color, :locale_altemphasis_color,"
               "  :locale_expired_color, :locale_future_color);" );
  }
  else if ( (_mode == cEdit) || (_mode == cCopy) )
    q.prepare( "UPDATE locale "
                "SET locale_code=:locale_code,"
                "    locale_descrip=:locale_descrip,"
                "    locale_lang_id=:locale_lang_id,"
                "    locale_country_id=:locale_country_id,"
                "    locale_dateformat=:locale_dateformat,"
                "    locale_timeformat=:locale_timeformat,"
                "    locale_timestampformat=:locale_timestampformat,"
                "    locale_intervalformat=:locale_intervalformat,"
                "    locale_qtyformat=:locale_qtyformat,"
		"    locale_curr_scale=:locale_curr_scale,"
                "    locale_salesprice_scale=:locale_salesprice_scale,"
                "    locale_purchprice_scale=:locale_purchprice_scale,"
                "    locale_extprice_scale=:locale_extprice_scale,"
                "    locale_cost_scale=:locale_cost_scale,"
                "    locale_qty_scale=:locale_qty_scale,"
                "    locale_qtyper_scale=:locale_qtyper_scale,"
                "    locale_uomratio_scale=:locale_uomratio_scale,"
                "    locale_percent_scale=:locale_percent_scale,"
                "    locale_comments=:locale_comments,"
                "    locale_error_color=:locale_error_color,"
                "    locale_warning_color=:locale_warning_color,"
                "    locale_emphasis_color=:locale_emphasis_color,"
                "    locale_altemphasis_color=:locale_altemphasis_color,"
                "    locale_expired_color=:locale_expired_color,"
                "    locale_future_color=:locale_future_color "
                "WHERE (locale_id=:locale_id);" );

  q.bindValue(":locale_id",                _localeid);
  q.bindValue(":locale_code",              _code->text());
  q.bindValue(":locale_descrip",           _description->text());
  q.bindValue(":locale_lang_id",           _language->id());
  q.bindValue(":locale_country_id",        _country->id());
  q.bindValue(":locale_curr_scale",        _currencyScale->text());
  q.bindValue(":locale_salesprice_scale",  _salesPriceScale->text());
  q.bindValue(":locale_purchprice_scale",  _purchPriceScale->text());
  q.bindValue(":locale_extprice_scale",    _extPriceScale->text());
  q.bindValue(":locale_cost_scale",        _costScale->text());
  q.bindValue(":locale_qty_scale",         _qtyScale->text());
  q.bindValue(":locale_qtyper_scale",      _qtyPerScale->text());
  q.bindValue(":locale_uomratio_scale",    _uomRatioScale->text());
  q.bindValue(":locale_percent_scale",     _percentScale->text());
  q.bindValue(":locale_comments",          _comments->toPlainText());
  q.bindValue(":locale_error_color",       _error->text());
  q.bindValue(":locale_warning_color",     _warning->text());
  q.bindValue(":locale_emphasis_color",    _emphasis->text());
  q.bindValue(":locale_altemphasis_color", _alternate->text());
  q.bindValue(":locale_expired_color",     _expired->text());
  q.bindValue(":locale_future_color",      _future->text());
  q.bindValue(":locale_dateformat",     convert(sampleLocale.dateFormat(QLocale::ShortFormat)));
  q.bindValue(":locale_timeformat",     convert(sampleLocale.timeFormat(QLocale::ShortFormat)));
  q.bindValue(":locale_timestampformat",convert(sampleLocale.dateFormat(QLocale::ShortFormat)) +
                                  " " + convert(sampleLocale.timeFormat(QLocale::ShortFormat)));
  {
    QString intervalfmt = convert(sampleLocale.timeFormat(QLocale::ShortFormat).remove("ap", Qt::CaseInsensitive));
    intervalfmt.insert(intervalfmt.indexOf("HH") + 2, "24");
    q.bindValue(":locale_intervalformat", intervalfmt);
  }
  q.bindValue(":locale_qtyformat",      QString(sampleLocale.decimalPoint()) +
                                        QString(sampleLocale.negativeSign()) +
                                        QString(sampleLocale.groupSeparator()));
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_localeid);
}

void sysLocale::close()
{
  qDebug("sysLocale::close()");
  if (_mode == cCopy && _localeid > 0)
  {
    q.prepare( "DELETE FROM locale "
               "WHERE (locale_id=:locale_id);" );
    q.bindValue(":locale_id", _localeid);
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  reject();
}
 
void sysLocale::sUpdateCountries()
{
  QLocale::Language localeLang = QLocale::C;

  if (_language->id() > 0)
  {
    q.prepare("SELECT lang_qt_number FROM lang WHERE (lang_id=:langid);");
    q.bindValue(":langid", _language->id());
    q.exec();
    if (q.first())
    {
      localeLang = QLocale::Language(q.value("lang_qt_number").toInt());
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  int currentCountry = _country->id();

  QList<QLocale::Country> clist = QLocale::countriesForLanguage(localeLang);

  _country->clear();
  if(!clist.isEmpty())
  {
    QString sql = "SELECT country_id, country_name, country_name"
                  "  FROM country"
                  " WHERE((country_qt_number IS NOT NULL)"
                  "   AND (country_qt_number IN (-1";
    for (int i = 0; i < clist.size(); ++i)
    {
       sql.append(",");
       sql.append(QString::number((int)clist.at(i)));
    }
    sql += ")))"
           " ORDER BY country_name;";
    _country->populate(sql, currentCountry);
  }

  sUpdateSamples();
}

void sysLocale::sUpdateSamples()
{
  QLocale sampleLocale = generateLocale();

  q.prepare("SELECT CURRENT_DATE AS dateSample, CURRENT_TIME AS timeSample,"
            "       CURRENT_TIMESTAMP AS timestampSample,"
            "       CURRENT_TIMESTAMP - CURRENT_DATE AS intervalSample,"
            "       987654321.987654321 AS doubleSample;");
  q.exec();
  if (q.first())
  {
    _dateSample->setText(sampleLocale.toString(q.value("dateSample").toDate(), QLocale::ShortFormat));
    _timeSample->setText(sampleLocale.toString(q.value("timeSample").toTime(), QLocale::ShortFormat));
    _timestampSample->setText(sampleLocale.toString(q.value("timestampSample").toDate(), QLocale::ShortFormat) +
                              " " +
                              sampleLocale.toString(q.value("timestampSample").toTime(), QLocale::ShortFormat));
    _intervalSample->setText(sampleLocale.toString(q.value("intervalSample").toTime(), QLocale::ShortFormat));

    _currencySample->setText(sampleLocale.toString(q.value("doubleSample").toDouble(), 'f', _currencyScale->value()));
    _salesPriceSample->setText(sampleLocale.toString(q.value("doubleSample").toDouble(), 'f', _salesPriceScale->value()));
    _purchPriceSample->setText(sampleLocale.toString(q.value("doubleSample").toDouble(), 'f', _purchPriceScale->value()));
    _extPriceSample->setText(sampleLocale.toString(q.value("doubleSample").toDouble(), 'f', _extPriceScale->value()));
    _costSample->setText(sampleLocale.toString(q.value("doubleSample").toDouble(), 'f', _costScale->value()));
    _qtySample->setText(sampleLocale.toString(q.value("doubleSample").toDouble(), 'f', _qtyScale->value()));
    _qtyPerSample->setText(sampleLocale.toString(q.value("doubleSample").toDouble(), 'f', _qtyPerScale->value()));
    _uomRatioSample->setText(sampleLocale.toString(q.value("doubleSample").toDouble(), 'f', _uomRatioScale->value()));
    _percentSample->setText(sampleLocale.toString(q.value("doubleSample").toDouble(), 'f', _percentScale->value()));
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void sysLocale::sUpdateColors()
{
  QList<QLineEdit*> colorwidgets;
  colorwidgets << _error        << _warning << _emphasis
               << _alternate    << _expired << _future;
  for (int i = 0; i < colorwidgets.size(); i++)
  {
    QLineEdit *widget = colorwidgets.at(i);
    QPalette colors = widget->palette();
    colors.setBrush(QPalette::Text, QBrush(QColor(widget->text())));
    widget->setPalette(colors);
  }
}

void sysLocale::populate()
{
  XSqlQuery popq;
  popq.prepare( "SELECT * "
             "FROM locale "
             "WHERE (locale_id=:locale_id);" );
  popq.bindValue(":locale_id", _localeid);
  popq.exec();
  if (popq.first())
  {
    _code->setText(popq.value("locale_code").toString());
    _description->setText(popq.value("locale_descrip").toString());
    _language->setId(popq.value("locale_lang_id").toInt());
    _country->setId(popq.value("locale_country_id").toInt());
    _currencyScale->setValue(popq.value("locale_curr_scale").toInt());
    _salesPriceScale->setValue(popq.value("locale_salesprice_scale").toInt());
    _purchPriceScale->setValue(popq.value("locale_purchprice_scale").toInt());
    _extPriceScale->setValue(popq.value("locale_extprice_scale").toInt());
    _costScale->setValue(popq.value("locale_cost_scale").toInt());
    _qtyScale->setValue(popq.value("locale_qty_scale").toInt());
    _qtyPerScale->setValue(popq.value("locale_qtyper_scale").toInt());
    _uomRatioScale->setValue(popq.value("locale_uomratio_scale").toInt());
    _percentScale->setValue(popq.value("locale_percent_scale").toInt());
    _comments->setText(popq.value("locale_comments").toString());
    _error->setText(popq.value("locale_error_color").toString());
    _warning->setText(popq.value("locale_warning_color").toString());
    _emphasis->setText(popq.value("locale_emphasis_color").toString());
    _alternate->setText(popq.value("locale_altemphasis_color").toString());
    _expired->setText(popq.value("locale_expired_color").toString());
    _future->setText(popq.value("locale_future_color").toString());

    sUpdateSamples();
    sUpdateColors();
  }
  if (popq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, popq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

// convert between Qt's date/time formatting strings and PostgreSQL's
QString sysLocale::convert(const QString &input)
{
  // create an array with a certain preallocated size
  QByteArray output(input.size()+1, '\0');
  // clear the array so we are starting at the beginning but still have the allocations
  output.clear();
  QString errMsg = tr("<p>.Could not translate Qt date/time formatting "
                      "string %1 to PostgreSQL date/time formatting "
                      "string. Error at or near character %2.");

  int i = 0;

  for (i = 0; i < input.size(); i++)
  {
    switch (input[i].toAscii())
    {
    case '\'':
      if (input[++i] == '\'')
        output.append('\'');
      else for (; input[i] != '\''; i++)
        output.append(input[i].toAscii());
      if (input[i] == '\'')
        i++;
      break;

    case 'd':
      if (input[i+1] != 'd')
      {
        output.append('D');
        output.append('D');
      }
      else if (input[i+2] != 'd')
      {
        output.append('D');
        output.append('D');
        i++;
      }
      else if (input[i+3] != 'd')
      {
        output.append('D');
        output.append('y');
        i += 2;
      }
      else if (input[i+3] == 'd')
      {
        output.append('D');
        output.append('a');
        output.append('y');
        i += 3;
      }
      break;

    case 'M':
      if (input[i+1] != 'M')
      {
        output.append('M');
        output.append('M');
      }
      else if (input[i+2] != 'M')
      {
        output.append('M');
        output.append('M');
        i++;
      }
      else if (input[i+3] != 'M')
      {
        output.append('M');
        output.append('o');
        output.append('n');
        i += 2;
      }
      else if (input[i+3] == 'M')
      {
        output.append('M');
        output.append('o');
        output.append('n');
        output.append('t');
        output.append('h');
        i += 3;
      }
      break;

    case 'y':
      if (input[i+1] != 'y')
      {
        systemError(this, errMsg.arg(input).arg(i), __FILE__, __LINE__);
        return "";
      }
      else if (input[i+2] != 'y')
      {
        output.append('Y');
        output.append('Y');
        i++;
      }
      else if (input[i+3] != 'y')
      {
        systemError(this, errMsg.arg(input).arg(i), __FILE__, __LINE__);
        return "";
      }
      else if (input[i+3] == 'y')
      {
        output.append('Y');
        output.append('Y');
        output.append('Y');
        output.append('Y');
        i += 3;
      }
      break;

    case 'h':
      {
        bool ampm = false;
        int j;
        for (j = i; j < input.size() && !ampm; j++)
        {
          if (input[j] == '\'')
            while (input[++j] != '\'' && j < input.size())
              ;
          ampm = ((input[j] == 'a' && input[j+1] == 'p') ||
                  (input[j] == 'A' && input[j+1] == 'P'));
        }
        output.append('H');
        output.append('H');
        if (ampm)
        {
          output.append('1');
          output.append('2');
        }
        else
        {
          output.append('2');
          output.append('4');
        }
        if (input[i+1] == 'h')
          i++;
      }
      break;

    case 'm':
      output.append('M');
      output.append('I');
      if (input[i+1] == 'm')
        i++;
      break;

    case 's':
      output.append('S');
      output.append('S');
      if (input[i+1] == 's')
        i++;
      break;

    case 'z':
      output.append('M');
      output.append('S');
      if (input[i+1] == 'z' && input[i+2] != 'z')
      {
        systemError(this, errMsg.arg(input).arg(i), __FILE__, __LINE__);
        return "";
      }
      else if (input[i+1] == 'z' && input[i+2] == 'z')
        i += 2;
      break;

    case 'A':
      output.append('A');
      if (input[i+1] == 'P')
      {
        output.append('M');
        i++;
      }
      break;

    case 'a':
      output.append('a');
      if (input[i+1] == 'p')
      {
        output.append('m');
        i++;
      }
      break;

    default:
      output.append(input[i].toAscii());
      break;
    }
  }

  //output.append('\0');

  if (DEBUG)
    qDebug() << "sysLocale::convert() " << input << " to " << output;

  return QString(output);
}

QLocale sysLocale::generateLocale()
{
  QLocale::Language localeLang = QLocale::C;
  QLocale::Country  localeCountry = QLocale::AnyCountry;

  if (_language->id() > 0)
  {
    q.prepare("SELECT lang_qt_number FROM lang WHERE (lang_id=:langid);");
    q.bindValue(":langid", _language->id());
    q.exec();
    if (q.first())
    {
      localeLang = QLocale::Language(q.value("lang_qt_number").toInt());
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return QLocale("C");
    }
  }

  if (_country->id() > 0)
  {
    q.prepare("SELECT country_qt_number "
              "FROM country "
              "WHERE (country_id=:countryid);");
    q.bindValue(":countryid", _country->id());
    q.exec();
    if (q.first())
    {
      localeCountry = QLocale::Country(q.value("country_qt_number").toInt());
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return QLocale("C");
    }
  }

  return QLocale(localeLang, localeCountry);
}
