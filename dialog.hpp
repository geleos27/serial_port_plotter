#ifndef DIALOG_HPP
#define DIALOG_HPP

#include <QCloseEvent>
#include <QDialog>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QStringList>
#include <QTextStream>
#include <QValidator>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog {
  Q_OBJECT

public:
  explicit Dialog(QWidget *parent = nullptr);
  ~Dialog();
  Ui::Dialog *ui;

signals:

  void buttonPressed();
  void buttonPressed1();
  void buttonPressed2();
  void buttonPressed3(QString data);
  void buttonPressed4();
  void toggle_EDIT_MODE(bool mode);

  void SendParameter(QString data);

  // void txtalias();

private slots:

  void on_read_clicked();

  void on_RIGHT_clicked();

  void on_LEFT_clicked();

  void recieveData(QString str);

  void on_buttonBox_accepted();

  void on_SEND_clicked();

  void on_buttonBox_rejected();

  void on_clearbutton_clicked();

  void on_readfile_clicked();

  void on_savefile_clicked();

  void on_EDIT_MODE_clicked();

  /*
   *
   *   void on_aliasprofile_textChanged(const QString &text);

  void on_kd1_textChanged(const QString &text);

  void on_ki1_textChanged(const QString &text);

  void on_kd2_textChanged(const QString &text);

  void on_kd3_textChanged(const QString &text);

  void on_ki2_textChanged(const QString &text);

  void on_ki3_textChanged(const QString &text);

  void on_kp1_textChanged(const QString &text);

  void on_kp2_textChanged(const QString &text);

  void on_kp3_textChanged(const QString &text);

  void on_time_step_top_textChanged(const QString &text);

  void on_temperature_step_top_textChanged(const QString &text);

  void on_time_step_bottom_textChanged(const QString &text);

  void on_temperature_step_bottom_textChanged(const QString &text);

  void on_time_step_pcb_textChanged(const QString &text);

  void on_temperature_step_pcb_textChanged(const QString &text);

  // void on_aliasprofile_textEdited(const QString &textnew);

  void on_aliasprofile_editingFinished();

  // void on_aliasprofile_selectionChanged();

  //  void on_aliasprofile_returnPressed();

  void on_kp1_editingFinished();

  void on_ki1_editingFinished();

  void on_kd1_editingFinished();

  void on_kp2_editingFinished();

  void on_ki2_editingFinished();

  void on_kd2_editingFinished();

  void on_kp3_editingFinished();

  void on_ki3_editingFinished();

  void on_kd3_editingFinished();

  void on_time_step_top_editingFinished();

  void on_temperature_step_top_editingFinished();

  void on_time_step_bottom_editingFinished();

  void on_temperature_step_bottom_editingFinished();

  void on_time_step_pcb_editingFinished();

  void on_temperature_step_pcb_editingFinished();

  void on_max_pcb_delta_editingFinished();

  void on_profile_steps_editingFinished();

  void on_profile_steps_textChanged(const QString &text);

  void on_max_pcb_delta_textChanged(const QString &text);

  void on_max_correction_top_textChanged(const QString &text);

  void on_max_correction_top_editingFinished();

  void on_max_correction_bottom_textChanged(const QString &text);

  void on_max_correction_bottom_editingFinished();

  void on_hold_lenght_textChanged(const QString &text);

  void on_hold_lenght_editingFinished();

  // void on_lineEdit_5_textChanged(const QString &text);

  void on_participation_rate_top_textChanged(const QString &text);

  void on_participation_rate_top_editingFinished();
  */

private:
 // Ui::Dialog *ui;

  bool stopflag = 0;

  bool readflag = 0;

  bool profileEditEnable = false;

  void closeEvent(QCloseEvent *bar);

  void setSettingsValue(QString line, QString value);
};

#endif // DIALOG_H
