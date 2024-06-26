/***************************************************************************
**  This file is part of Serial Port Plotter                              **
**                                                                        **
**                                                                        **
**  Serial Port Plotter is a program for plotting integer data from       **
**  serial port using Qt and QCustomPlot                                  **
**                                                                        **
**  This program is free software: you can redistribute it and/or modify  **
**  it under the terms of the GNU General Public License as published by  **
**  the Free Software Foundation, either version 3 of the License, or     **
**  (at your option) any later version.                                   **
**                                                                        **
**  This program is distributed in the hope that it will be useful,       **
**  but WITHOUT ANY WARRANTY; without even the implied warranty of        **
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
**  GNU General Public License for more details.                          **
**                                                                        **
**  You should have received a copy of the GNU General Public License     **
**  along with this program.  If not, see http://www.gnu.org/licenses/.   **
**                                                                        **
****************************************************************************
**           Author: Borislav                                             **
**           Contact: b.kereziev@gmail.com                                **
**           Date: 29.12.14                                               **
****************************************************************************/

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include "dialog.hpp"
#include "helpwindow.hpp"
#include "qcustomplot/qcustomplot.h"
#include <QCloseEvent>
#include <QEvent>
#include <QLineEdit>
#include <QMainWindow>
#include <QSerialPortInfo>
#include <QSettings>
#include <QSound>
#include <QTimer>
#include <QtSerialPort/QtSerialPort>
#include <QMap>
#include <QStringList>

#define START_MSG '$'
#define CLEAR_MSG '#'
#define END_MSG ';'
#define DONG_MSG '@'
#define COMMAND_MSG '!'

#define WAIT_START 1
#define IN_MESSAGE 2
#define IN_COMMAND 3
#define UNDEFINED 4
#define WAIT_COMMAND 5
#define IN_STATUS 6

#define CUSTOM_LINE_COLORS 14
#define GCP_CUSTOM_LINE_COLORS 4

namespace Ui {
class MainWindow;
}

// class Dialog;

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  void closeEvent(QCloseEvent *event); // close event to save settings TODO,
                                       // CHANGE TO BUTTON
  ~MainWindow();
  // QString uartex;

  struct ProfileData {                                  // Structure for storing current profile data
      QMap<QString, QString> values;
      void addValue(const QString &key, const QString &data) {
          values[key] = data;
      }
  };
  ProfileData profiledata;

  bool profileEditEnable = false;


private slots:
  void on_comboPort_currentIndexChanged(
      const QString &arg1); // Slot displays message on status bar
  void portOpenedSuccess(); // Called when port opens OK
  void portOpenedFail();    // Called when port fails to open
  void onPortClosed();      // Called when closing the port
  void replot();            // Slot for repainting the plot
  void
  onNewDataArrived(QStringList newData); // Slot for new data from serial port
  void
  saveStream(QStringList newData); // Save the received data to the opened file
  void
  on_spinAxesMin_valueChanged(int arg1); // Changing lower limit for the plot
  void
  on_spinAxesMax_valueChanged(int arg1); // Changing upper limit for the plot
  void readData();                       // Slot for inside serial port
  void
  on_spinYStep_valueChanged(int arg1); // Spin box for changing Y axis tick step
  void on_actionRecord_PNG_triggered(); // Button for saving JPG
  void onMouseMoveInPlot(
      QMouseEvent *event); // Displays coordinates of mouse pointer when clicked
                           // in plot in status bar
  void
  on_spinPoints_valueChanged(int arg1); // Spin box controls how many data
                                        // points are collected and displayed
  void on_mouse_wheel_in_plot(
      QWheelEvent *event); // Makes wheel mouse works while plotting

  /* Used when a channel is selected (plot or legend) */
  void channel_selection(void);
  void legend_double_click(QCPLegend *legend, QCPAbstractLegendItem *item,
                           QMouseEvent *event);

  void on_actionConnect_triggered();
  void on_actionDisconnect_triggered();
  void on_actionHow_to_use_triggered();
  void on_actionPause_Plot_triggered();
  void on_actionClear_triggered();
  void on_actionRecord_stream_triggered();
  void on_actionLoad_Settings_triggered();
  void on_actionSave_Settings_triggered();

  void on_pushButton_TextEditHide_clicked();

  void on_pushButton_ShowallData_clicked();

  void on_pushButton_SendToCom_clicked(); // to send data over COM

  void on_pushButton_AutoScale_clicked();

  void on_pushButton_ResetVisible_clicked();

  void on_pushButton_OK_clicked();
  void on_pushButton_UP_clicked();
  void on_pushButton_DOWN_clicked();
  void on_pushButton_LEFT_clicked();
  void on_pushButton_RIGHT_clicked();
  void on_pushButton_CANCEL_clicked();

  void on_listWidget_Channels_itemDoubleClicked(QListWidgetItem *item);

  void on_pushButton_clicked();
  void UpdatePortControls();

  // void on_comboMod(const QString &arg1);

  void on_pushButton_2_clicked();

  // void on_pushButton_profil_clicked();

  void profilread();

  void on_SimpleExpert_clicked(); // Simple \ Expert viev

  void send_data_to_com(QString newdata);

  void unblock(QString epromdata);

  void unblock2();

  void on_HotStart_clicked();

  void on_manulabutton_clicked();

  //void on_graph_point_clicked();

  void handleMousePress(QMouseEvent* event);
  void handleMouseRelease(QMouseEvent* event);

  void plot_All_Profile(); // Print 3 graphs of loaded thermalprofile
  void clear_plottables_graph(); // clear graphs arrived from station
  void clear_profile_graph(); // clear profile graphs
  void plottableDoubleClicked(QCPAbstractPlottable *plottable, int dataIndex, QMouseEvent *event);
  void exportPlotToProfileData();
  void switch_EDIT_MODE(bool mode);

signals:
  void portOpenFail();            // Emitted when cannot open port
  void portOpenOK();              // Emitted when port is open
  void portClosed();              // Emitted when port is closed
  void newData(QStringList data); // Emitted when new data has arrived
  void sendData(QString str);

private:
  Ui::MainWindow *ui;
  // Ui::Dialog ui;

  /* Line colors */
  QColor line_colors[CUSTOM_LINE_COLORS];
  QColor gui_colors[GCP_CUSTOM_LINE_COLORS];

  /* Main info */
  bool connected;      // Status connection variable
  bool plotting;       // Status plotting variable
  int dataPointNumber; // Keep track of data points
  /* Channels of data (number of graphs) */
  int channels;

  bool alflag;

  /* Data format */
  int data_format;

  /* Textbox Related */
  bool filterDisplayedData = false;

  /* Listview Related */
  QStringListModel *channelListModel;
  QStringList channelStrList;

  //-- CSV file to save data
  QFile *m_csvFile = nullptr;
  void openCsvFile(void);
  void closeCsvFile(void);

  QTimer *tmr; // timer for update COM-Ports.

  /* Preferences  TODO Fix number of saved prefs*/

  struct LegendChannelNames {
    QString channelName;
    bool channelVisibie;
  };

  struct ButtonNames {
    QString buttonName;
    QString buttonCommand;
    bool buttonVisible;
  };

  struct SPreferences {
    int port;   // last port used
    int baud;   // last baudrate item used
    int data;   // last data length used
    int parity; // last parity used
    int stop;   // last stop bit number
    int spinPoints;
    int spinYStep;   // last value used
    int spinAxesMin; // last value used
    int spinAxesMax; // last value used
    bool TextEditHide;
    bool SimpleExpert;
    bool ShowallData;
    bool Record_stream;
    int LCD_Channel;
    bool ChannelOnLCDVisible;
    QList<LegendChannelNames> channelnames; // ChannelNames Structure
    QList<ButtonNames> buttonnames;         // ButtonNames Structure
  };
  SPreferences m_prefs; // preferences stucture

  //int spinPoints1;
  //int spinYStep1;
  //int spinAxesMin1;
  //int spinAxesMax1;
  //bool spin; // in header
  //int delta;
  //int clkb = 1;
  //int frstch = 0;

  QTimer updateTimer;        // Timer used for replotting the plot
  QTime timeOfFirstData;     // Record the time of the first data point
  double timeBetweenSamples; // Store time between samples
  QSerialPort *serialPort;   // Serial port; runs in this thread
  QString receivedData;      // Used for reading from the port
  QString receivedDataRaw;
  QString receivedWord;
  QString receivedProfile;
  QString statusData;

  // void setSettingsValue(QString line, QString value);

  int STATE; // State of recieiving message from port
  int PREV_STATE; // State before interruption
  int STATEAL;

  int NUMBER_OF_POINTS; // Number of points plotted
  HelpWindow *helpWindow;
  Dialog *dialog;

  void createUI();                       // Populate the controls
  void enable_com_controls(bool enable); // Enable/disable controls
  void enable_heater_controls(bool enable);
  void enable_advanced_controls(bool enable);
  void setupPlot(); // Setup the QCustomPlot
                    // Open the inside serial port with these parameters
  void openPort(QSerialPortInfo portInfo, int baudRate, QSerialPort::DataBits,
                QSerialPort::Parity, QSerialPort::StopBits);
  void loadSettings(); // load settings to populate preferences from config file
  void saveSettings(); // save preferences in config file
                       // void closeEvent(QCloseEvent *bar);

  QVector<double> timeStampsVector;
  QVector<double> temperaturesVector;
  void plot_Profile(QString timeStamps, QString temperatures, int name);

  QCPAbstractPlottable* selectedPlottable = nullptr;
  int selectedDataIndex = -1;


};

#endif // MAINWINDOW_HPP
