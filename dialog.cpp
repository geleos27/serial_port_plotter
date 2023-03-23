#include "dialog.hpp"
#include "ui_dialog.h"
#include <QDialog>
// #include <mainwindow.hpp>
// #include <QChar>
// #include <QString>
// #include <QByteArray>
// #include <ctype.h>
// #include <iostream>
// #include <stdio.h>
#include <QCloseEvent>
// #include <QStyle>

Dialog::Dialog(QWidget *parent) : QDialog(parent), ui(new Ui::Dialog) {
  ui->setupUi(this);
  ui->SEND->setEnabled(false);
  ui->savefile->setEnabled(false);
  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
  ui->kd1->setValidator(new QIntValidator(0, 255, this));
  ui->kd2->setValidator(new QIntValidator(0, 255, this));
  ui->kd3->setValidator(new QIntValidator(0, 255, this));
  ui->ki1->setValidator(new QIntValidator(0, 255, this));
  ui->ki2->setValidator(new QIntValidator(0, 255, this));
  ui->ki3->setValidator(new QIntValidator(0, 255, this));
  ui->kp1->setValidator(new QIntValidator(0, 255, this));
  ui->kp2->setValidator(new QIntValidator(0, 255, this));
  ui->kp3->setValidator(new QIntValidator(0, 255, this));
  ui->max_pcb_delta->setValidator(new QIntValidator(0, 255, this));
  ui->profile_steps->setValidator(new QIntValidator(0, 30, this));
  ui->max_correction_top->setValidator(new QIntValidator(0, 255, this));
  ui->max_correction_bottom->setValidator(new QIntValidator(0, 255, this));
  ui->hold_lenght->setValidator(new QIntValidator(0, 255, this));
  ui->participation_rate_top->setValidator(new QIntValidator(0, 100, this));
  ui->aliasprofile->setValidator(new QRegExpValidator(
      QRegExp("[A-Za-z0-9 -]{1,19}"))); // Валидатор по регулярному выражению
                                        // длинной от 2 до 20 символов
                                        // латинского алфавита и цифр
  ui->time_step_top->setValidator(new QRegExpValidator(QRegExp("[0-9,]*")));
  ui->temperature_step_top->setValidator(
      new QRegExpValidator(QRegExp("[0-9,]*")));

  ui->time_step_bottom->setValidator(new QRegExpValidator(QRegExp("[0-9,]*")));
  ui->temperature_step_bottom->setValidator(
      new QRegExpValidator(QRegExp("[0-9,]*")));

  ui->time_step_pcb->setValidator(new QRegExpValidator(QRegExp("[0-9,]*")));
  ui->temperature_step_pcb->setValidator(
      new QRegExpValidator(QRegExp("[0-9,]*")));
  ui->read->click();
}

Dialog::~Dialog() { delete ui; }

void Dialog::closeEvent(QCloseEvent *) { on_buttonBox_rejected(); }

// прием команд из первой формы и парсинг команд по окошкам
void Dialog::recieveData(QString str) {
  if (readflag == 1) {
    QStringList data = str.split(':');
    // if(data[0]=="profile_steps"){ui->profile_steps->setText(data[1]);}
    if (data[0] == "profile") {
      ui->profile->setText(data[1]);
    }

    // if( data[1].at( 0 ) == ' ' ) {data[1].remove( 0, 1 );}
    if (data[1].at(0) == ' ') {
      data[1].remove(QRegularExpression("^ "));
    }

    setSettingsValue(data[0], data[1]);
    str.clear();
  }
  ui->readwriteOK->setText("Reading profile settings OK");
}

void Dialog::setSettingsValue(QString name, QString value) {
  QLineEdit *edit = this->findChild<QLineEdit *>(name);
  if (edit != nullptr) {
    edit->clear();
    edit->setText(value);
  }
}

// кнопка чтение профиля из eeprom
void Dialog::on_read_clicked() {
  ui->read->setEnabled(false);
  ui->readfile->setEnabled(false);
  ui->readwriteOK->setText("Reading eeprom...");
  readflag = 1;
  emit buttonPressed(); //отправить команду станции на вывод профиля в порт
  ui->SEND->setEnabled(true);
  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
  ui->savefile->setEnabled(true);
}

// сменить профиль
void Dialog::on_RIGHT_clicked() {
  ui->read->setEnabled(true);
  ui->clearbutton->click();
  emit buttonPressed1();
  ui->read->click();
}

// сменить профиль
void Dialog::on_LEFT_clicked() {
  ui->read->setEnabled(true);
  ui->clearbutton->click();
  emit buttonPressed2();
  ui->read->click();
}

// запись профиля в епром и закрытие редактора профиля
void Dialog::on_buttonBox_accepted() {

  QMessageBox msgBox;
  QString eprofil = ui->profile->text();
  msgBox.setText(QString("Save to memory Profile %1 ?").arg(eprofil));
  msgBox.setInformativeText("Rewrite EEPROM section?");
  msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Cancel);
  msgBox.setIcon(QMessageBox::Information);
  msgBox.setDefaultButton(QMessageBox::Save);
  int ret = msgBox.exec();
  QString epromdata = ui->profile->text();
  epromdata.prepend("N ");
  switch (ret) {
  case QMessageBox::Save:
    ui->SEND->click();
    emit buttonPressed3(epromdata);
    epromdata.clear();
    break;

  case QMessageBox::Cancel:
    on_buttonBox_rejected();
    break;
  default:
    // should never be reached
    break;
  }
}

// запись профиля в память станции
void Dialog::on_SEND_clicked() {
  ui->SEND->setEnabled(false);
  ui->read->setEnabled(true);
  readflag = 0;
  ui->readwriteOK->setText("Sending...");
  QList<QWidget *> allWidgets =
      this->findChildren<QWidget *>(); // получить список всех дочерних виджетов
  QList<QWidget *>::iterator it;
  for (it = allWidgets.begin(); it != allWidgets.end();
       it++) { // перебираем по кругу все виджеты
    auto type = qobject_cast<QLineEdit *>(
        *it); // смотрим какого типа виджет, берем только QLineEdit
    if (nullptr != type) {
      QString paramdata = (*it)->objectName(); // get objectName
      paramdata.append(" ");                   // add space
      paramdata.append((type)->text());        // add text data
      if (((type)->text() != "") || ((type)->text() != " ")) {

        emit SendParameter(paramdata);
        paramdata.clear();
      }

      //   emit SendParameter(paramdata);
      paramdata.clear();
    }
  }
  emit SendParameter("N " + ui->profile->text());

  allWidgets.clear();
  ui->clearbutton->click();
  ui->readwriteOK->setText("Sending profile settings OK");
 // ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
  ui->savefile->setEnabled(true);
  ui->readfile->setEnabled(true);
}

void Dialog::on_buttonBox_rejected() {

  ui->clearbutton->click();
  emit buttonPressed4();
}

// очистка всех окон
void Dialog::on_clearbutton_clicked() {
  ui->ki1->clear();
  ui->ki2->clear();
  ui->ki3->clear();
  ui->kp1->clear();
  ui->kp2->clear();
  ui->kp3->clear();
  ui->kd1->clear();
  ui->kd2->clear();
  ui->kd3->clear();
  ui->profile_steps->clear();
  ui->time_step_top->clear();
  ui->temperature_step_top->clear();
  ui->time_step_bottom->clear();
  ui->temperature_step_bottom->clear();
  ui->time_step_pcb->clear();
  ui->temperature_step_pcb->clear();
  ui->aliasprofile->clear();
  ui->max_pcb_delta->clear();
  ui->profile_steps->clear();
  ui->max_correction_top->clear();
  ui->max_correction_bottom->clear();
  ui->hold_lenght->clear();
  ui->participation_rate_top->clear();
  ui->savefile->setEnabled(false);
  ui->readwriteOK->setText("All data clear");
  ui->SEND->setEnabled(false);
  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
  ui->readfile->setEnabled(true);
  ui->read->setEnabled(true);
  ui->aliasprofile->setStyleSheet("");
  ui->kd1->setStyleSheet("");
  ui->kd2->setStyleSheet("");
  ui->kd3->setStyleSheet("");
  ui->ki1->setStyleSheet("");
  ui->ki2->setStyleSheet("");
  ui->ki3->setStyleSheet("");
  ui->kp1->setStyleSheet("");
  ui->kp2->setStyleSheet("");
  ui->kp3->setStyleSheet("");
  ui->time_step_top->setStyleSheet("");
  ui->temperature_step_top->setStyleSheet("");
  ui->time_step_bottom->setStyleSheet("");
  ui->temperature_step_bottom->setStyleSheet("");
  ui->time_step_pcb->setStyleSheet("");
  ui->temperature_step_pcb->setStyleSheet("");
  ui->max_pcb_delta->setStyleSheet("");
  ui->profile_steps->setStyleSheet("");
  ui->max_correction_top->setStyleSheet("");
  ui->max_correction_bottom->setStyleSheet("");
  ui->hold_lenght->setStyleSheet("");
  ui->participation_rate_top->setStyleSheet("");
}

// чтение из файла
void Dialog::on_readfile_clicked() {
  ui->readwriteOK->setText("Reading from file...");
  QString fileName = QFileDialog::getOpenFileName(
      this, "OpenFile", "./profiles", tr("Text files (*.txt)"));
  QFile file(fileName);
  if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream stream(&file);
    QStringList lines;
    QStringList data;
    QString line;
    do {
      line = stream.readLine();
      lines.append(line);
      if (line.contains(':')) {
        data = line.split(':');
        // if( data[1].at( 0 ) == ' ' ) {data[1].remove( 0, 1 );}
        if (data[1].at(0) == ' ') {
          data[1].remove(QRegularExpression("^ "));
        }
        setSettingsValue(data[0], data[1]);
      }
    } while (!line.isNull());

    lines.clear();
    line.clear();
    file.close();
  }

  ui->savefile->setEnabled(true);
  ui->SEND->setEnabled(true);
  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
  ui->read->setEnabled(false);
  ui->readfile->setEnabled(false);
  ui->readwriteOK->setText("Reading from file OK");
}

void Dialog::on_savefile_clicked() {
  QString fileName = QFileDialog::getSaveFileName(
      this, "OpenFile", "./profiles/" + ui->aliasprofile->text(),
      tr("Text files (*.txt)"));
  QFile file(fileName);
  if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QTextStream out(&file);
    out << "aliasprofile:" << ui->aliasprofile->text() << "\n";
    out << "ki1:" << ui->ki1->text() << "\n";
    out << "ki2:" << ui->ki2->text() << "\n";
    out << "ki3:" << ui->ki3->text() << "\n";
    out << "kp1:" << ui->kp1->text() << "\n";
    out << "kp2:" << ui->kp2->text() << "\n";
    out << "kp3:" << ui->kp3->text() << "\n";
    out << "kd1:" << ui->kd1->text() << "\n";
    out << "kd2:" << ui->kd2->text() << "\n";
    out << "kd3:" << ui->kd3->text() << "\n";
    out << "time_step_top:" << ui->time_step_top->text() << "\n";
    out << "temperature_step_top:" << ui->temperature_step_top->text() << "\n";
    out << "time_step_bottom:" << ui->time_step_bottom->text() << "\n";
    out << "temperature_step_bottom:" << ui->temperature_step_bottom->text()
        << "\n";
    out << "time_step_pcb:" << ui->time_step_pcb->text() << "\n";
    out << "temperature_step_pcb:" << ui->temperature_step_pcb->text() << "\n";
    out << "max_pcb_delta:" << ui->max_pcb_delta->text() << "\n";
    out << "profile_steps:" << ui->profile_steps->text() << "\n";
    out << "max_correction_top:" << ui->max_correction_top->text() << "\n";
    out << "max_correction_bottom:" << ui->max_correction_bottom->text()
        << "\n";
    out << "hold_lenght:" << ui->hold_lenght->text() << "\n";
    out << "participation_rate_top:" << ui->participation_rate_top->text()
        << "\n";

    file.close();
  }

  ui->readwriteOK->setText("Saving profile to file OK.");
  return;
}

/*
void Dialog::on_aliasprofile_textChanged(const QString &text) {
  if (text.isEmpty()) {

  } else {
    ui->aliasprofile->setStyleSheet("color: black; background-color: yellow");
  }
}

void Dialog::on_kd1_textChanged(const QString &text) {
  // ui->kd1->setValidator( new QIntValidator(0, 255, this) );
  if (text.isEmpty()) {

  } else {

    ui->kd1->setStyleSheet("color: black; background-color: yellow");
  }
}

void Dialog::on_kd2_textChanged(const QString &text) {
  if (text.isEmpty()) {

  } else {
    ui->kd2->setStyleSheet("color: black; background-color: yellow");
  }
}

void Dialog::on_kd3_textChanged(const QString &text) {

  if (text.isEmpty()) {

  } else {
    ui->kd3->setStyleSheet("color: black; background-color: yellow");
  }
}

void Dialog::on_ki1_textChanged(const QString &text) {
  if (text.isEmpty()) {

  } else {
    ui->ki1->setStyleSheet("color: black; background-color: yellow");
  }
}

void Dialog::on_ki2_textChanged(const QString &text) {
  if (text.isEmpty()) {

  } else {
    ui->ki2->setStyleSheet("color: black; background-color: yellow");
  }
}

void Dialog::on_ki3_textChanged(const QString &text) {
  if (text.isEmpty()) {

  } else {
    ui->ki3->setStyleSheet("color: black; background-color: yellow");
  }
}

void Dialog::on_kp1_textChanged(const QString &text) {
  if (text.isEmpty()) {

  } else {
    ui->kp1->setStyleSheet("color: black; background-color: yellow");
  }
}

void Dialog::on_kp2_textChanged(const QString &text) {
  if (text.isEmpty()) {

  } else {
    ui->kp2->setStyleSheet("color: black; background-color: yellow");
  }
}

void Dialog::on_kp3_textChanged(const QString &text) {
  if (text.isEmpty()) {

  } else {
    ui->kp3->setStyleSheet("color: black; background-color: yellow");
  }
}

void Dialog::on_time_step_top_textChanged(const QString &text) {
  if (text.isEmpty()) {

  } else {
    ui->time_step_top->setStyleSheet("color: black; background-color: yellow");
  }
}

void Dialog::on_temperature_step_top_textChanged(const QString &text) {
  if (text.isEmpty()) {

  } else {
    ui->temperature_step_top->setStyleSheet(
        "color: black; background-color: yellow");
  }
}

void Dialog::on_time_step_bottom_textChanged(const QString &text) {
  if (text.isEmpty()) {

  } else {
    ui->time_step_bottom->setStyleSheet(
        "color: black; background-color: yellow");
  }
}

void Dialog::on_temperature_step_bottom_textChanged(const QString &text) {
  if (text.isEmpty()) {

  } else {
    ui->temperature_step_bottom->setStyleSheet(
        "color: black; background-color: yellow");
  }
}

void Dialog::on_time_step_pcb_textChanged(const QString &text) {
  if (text.isEmpty()) {

  } else {
    ui->time_step_pcb->setStyleSheet("color: black; background-color: yellow");
  }
}

void Dialog::on_temperature_step_pcb_textChanged(const QString &text) {
  if (text.isEmpty()) {

  } else {
    ui->temperature_step_pcb->setStyleSheet(
        "color: black; background-color: yellow");
  }
}

void Dialog::on_aliasprofile_editingFinished() {
  ui->aliasprofile->setStyleSheet("color: white; background-color: green");
}

void Dialog::on_kp1_editingFinished() {
  ui->kp1->setStyleSheet("color: white; background-color: green");
}

void Dialog::on_ki1_editingFinished() {
  ui->ki1->setStyleSheet("color: white; background-color: green");
}

void Dialog::on_kd1_editingFinished() {
  ui->kd1->setStyleSheet("color: white; background-color: green");
}

void Dialog::on_kp2_editingFinished() {
  ui->kp2->setStyleSheet("color: white; background-color: green");
}

void Dialog::on_ki2_editingFinished() {
  ui->ki2->setStyleSheet("color: white; background-color: green");
}

void Dialog::on_kd2_editingFinished() {
  ui->kd2->setStyleSheet("color: white; background-color: green");
}

void Dialog::on_kp3_editingFinished() {
  ui->kp3->setStyleSheet("color: white; background-color: green");
}

void Dialog::on_ki3_editingFinished() {
  ui->ki3->setStyleSheet("color: white; background-color: green");
}

void Dialog::on_kd3_editingFinished() {
  ui->kd3->setStyleSheet("color: white; background-color: green");
}

void Dialog::on_time_step_top_editingFinished() {
  ui->time_step_top->setStyleSheet("color: white; background-color: green");
}

void Dialog::on_temperature_step_top_editingFinished() {
  ui->temperature_step_top->setStyleSheet(
      "color: white; background-color: green");
}

void Dialog::on_time_step_bottom_editingFinished() {
  ui->time_step_bottom->setStyleSheet("color: white; background-color: green");
}

void Dialog::on_temperature_step_bottom_editingFinished() {
  ui->temperature_step_bottom->setStyleSheet(
      "color: white; background-color: green");
}

void Dialog::on_time_step_pcb_editingFinished() {
  ui->time_step_pcb->setStyleSheet("color: white; background-color: green");
}

void Dialog::on_temperature_step_pcb_editingFinished() {
  ui->temperature_step_pcb->setStyleSheet(
      "color: white; background-color: green");
}

void Dialog::on_max_pcb_delta_editingFinished() {
  ui->max_pcb_delta->setStyleSheet("color: white; background-color: green");
}

void Dialog::on_profile_steps_editingFinished() {
  ui->profile_steps->setStyleSheet("color: white; background-color: green");
}

void Dialog::on_profile_steps_textChanged(const QString &text) {
  if (text.isEmpty()) {

  } else {
    ui->profile_steps->setStyleSheet("color: black; background-color: yellow");
  }
}

void Dialog::on_max_pcb_delta_textChanged(const QString &text) {
  if (text.isEmpty()) {

  } else {
    ui->max_pcb_delta->setStyleSheet("color: black; background-color: yellow");
  }
}

void Dialog::on_max_correction_top_textChanged(const QString &text) {
  if (text.isEmpty()) {

  } else {
    ui->max_correction_top->setStyleSheet(
        "color: black; background-color: yellow");
  }
}

void Dialog::on_max_correction_top_editingFinished() {
  ui->max_correction_top->setStyleSheet(
      "color: white; background-color: green");
}

void Dialog::on_max_correction_bottom_textChanged(const QString &text) {
  if (text.isEmpty()) {

  } else {
    ui->max_correction_bottom->setStyleSheet(
        "color: black; background-color: yellow");
  }
}

void Dialog::on_max_correction_bottom_editingFinished() {
  ui->max_correction_bottom->setStyleSheet(
      "color: white; background-color: green");
}

void Dialog::on_hold_lenght_textChanged(const QString &text) {
  if (text.isEmpty()) {

  } else {
    ui->hold_lenght->setStyleSheet("color: black; background-color: yellow");
  }
}

void Dialog::on_hold_lenght_editingFinished() {
  ui->hold_lenght->setStyleSheet("color: white; background-color: green");
}

void Dialog::on_participation_rate_top_textChanged(const QString &text) {
  if (text.isEmpty()) {

  } else {
    ui->participation_rate_top->setStyleSheet(
        "color: black; background-color: yellow");
  }
}

void Dialog::on_participation_rate_top_editingFinished() {
  ui->participation_rate_top->setStyleSheet(
      "color: white; background-color: green");
}
*/
