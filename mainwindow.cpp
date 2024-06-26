/***************************************************************************
**  This file is part of Serial Port Plotter    Ver 2022                  **
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

#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include <x86intrin.h>



#include "dialog.hpp"
#include "ui_mainwindow.h"
#include <QDialog>
#include <QMessageBox>
#include <QPixmap>
// #include <QLineEdit>

// import QtQuick;.Extras 1.4;

/**
 * @brief Constructor
 * @param parent
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),

      // int spinPoints=600;

      /* Populate colors */
      line_colors{
          /* For channel data (gruvbox palette) */
          /* Light */
          QColor("#ED1A1A"), // Power TOP
          QColor("#6030F0"), // Power BOT
          QColor("#EDE255"), // Power PCB
          QColor("#F08922"), // Temp TOP
          QColor("#C837F0"), // Temp BOT
          QColor("#54F06C"), // Temp PCB
          QColor("#F76E1B"), // Diff TOP
          QColor("#B231F7"), // Diff BOT
          QColor("#75F759"), // Diff PCB
          QColor("#C74A20"), // Profile TOP
          QColor("#7F32C7"), // Profile BOT
          QColor("#699a32"), // Profile PCB
          QColor("#689d6a"),
          QColor("#d65d0e"),
      },
      gui_colors{
          /* Monochromatic for axes and ui */
          QColor(48, 47, 47, 255),    /**<  0: qdark ui dark/background color */
          QColor(80, 80, 80, 255),    /**<  1: qdark ui medium/grid color */
          QColor(170, 170, 170, 255), /**<  2: qdark ui light/text color */
          QColor(48, 47, 47,
                 200) /**<  3: qdark ui dark/background color w/transparency */
      },

      /* Main vars */

      connected(false), plotting(false), dataPointNumber(0), channels(0),
      serialPort(nullptr), STATE(WAIT_START), NUMBER_OF_POINTS(600) {

  // Dialog *dialog=new Dialog();

  ui->setupUi(this);

  dialog = new Dialog(this);
  dialog->setWindowTitle("Settings");
  dialog->setModal(false);

  /* Init UI and populate UI controls */
  createUI();

  /* Setup plot area and connect controls slots */
  setupPlot();

  /* Wheel over plot when plotting */
  connect(ui->plot, SIGNAL(mouseWheel(QWheelEvent *)), this,
          SLOT(on_mouse_wheel_in_plot(QWheelEvent *)));
  connect (ui->plot->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(slotRangeChanged(QCPRange)));

  /* Slot for printing coordinates */
  connect(ui->plot, SIGNAL(mouseMove(QMouseEvent *)), this,
          SLOT(onMouseMoveInPlot(QMouseEvent *)));

  /* Channel selection */
  connect(ui->plot, SIGNAL(selectionChangedByUser()), this,
          SLOT(channel_selection()));
  connect(ui->plot,
          SIGNAL(legendDoubleClick(QCPLegend *, QCPAbstractLegendItem *,
                                   QMouseEvent *)),
          this,
          SLOT(legend_double_click(QCPLegend *, QCPAbstractLegendItem *,
                                   QMouseEvent *)));




  /* Connect update timer to replot slot */
  connect(&updateTimer, SIGNAL(timeout()), this, SLOT(replot()));

  connect(dialog, SIGNAL(buttonPressed()), this, SLOT(profilread()));

  connect(dialog, SIGNAL(buttonPressed1()), this,
          SLOT(on_pushButton_RIGHT_clicked()));

  connect(dialog, SIGNAL(buttonPressed2()), this,
          SLOT(on_pushButton_LEFT_clicked()));

  connect(this, SIGNAL(sendData(QString)), dialog, SLOT(recieveData(QString)));

  connect(dialog, SIGNAL(SendParameter(QString)), this,
          SLOT(send_data_to_com(QString)));

  connect(dialog, SIGNAL(buttonPressed3(QString)), this,
          SLOT(unblock(QString)));
  connect(dialog, SIGNAL(buttonPressed4()), this, SLOT(unblock2()));

  connect(dialog, SIGNAL(toggle_EDIT_MODE(bool)), this, SLOT(switch_EDIT_MODE(bool)));

  /* Slots for editing graphs values*/
 // connect(ui->plot->graph(10), SIGNAL(plottableClick(QCPAbstractPlottable*,int,QMouseEvent*)), this, SLOT(plottableClicked(QCPAbstractPlottable*,int,QMouseEvent)));
  connect(ui->plot, SIGNAL(plottableDoubleClick(QCPAbstractPlottable *,int,QMouseEvent *)), this, SLOT(plottableDoubleClicked(QCPAbstractPlottable *,int,QMouseEvent *)));
  connect(ui->plot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(handleMousePress(QMouseEvent*)));
  connect(ui->plot, SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(handleMouseRelease(QMouseEvent*)));


  m_csvFile = nullptr;

  QSound *dong = new QSound("qrc:/serial_port_plotter/dong.wav");
}

/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/**
 * @brief Destructor
 */
MainWindow::~MainWindow() {
  closeCsvFile();

  if (serialPort != nullptr) {
    delete serialPort;
  }
  delete ui;
}
/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/**
 * @brief Create remaining elements and populate the controls
 */
void MainWindow::createUI() {

 // ui->pushButton_CANCEL->setEnabled(false);
  ui->pushButton_2->setVisible(false);
  ui->ready->setVisible(false);
  ui->manulabutton->setVisible(false);

  ui->manualTemp->setVisible(false);
  ui->HotStart->setEnabled(false);
  ui->HotStart->setVisible(false);

  ui->pushButton_UP->setEnabled(false);
 // ui->pushButton_UP->setStyleSheet("QPushButton { background-color: grey; }\n");

  ui->pushButton_DOWN->setEnabled(false);
  //ui->pushButton_DOWN->setStyleSheet("QPushButton { background-color: grey; }\n");

  // ui->pushButton_UP->setStyleSheet("QPushButton:enabled { background-color:
  // rgb(150,200,250); }\n"
  //                                  "QPushButton:enabled { color: rgb(0,0,0);
  //                                  }\n");

  // ui->pushButton_CANCEL->setStyleSheet("QPushButton { background-color: grey;
  // }\n"); ui->pushButton_CANCEL->setEnabled(false);

  ui->PlotControlsBox->setVisible(false); // Hide uPlotControlsBox
  ui->HeaterControlsBox->setVisible(false); // Hide Heater controls


  /* Check if there are any ports at all; if not, disable controls and return */
  tmr = new QTimer(this);
  ui->actionSave_Settings->setEnabled(false);
  if (QSerialPortInfo::availablePorts().size() == 0) {
    enable_com_controls(false);
    ui->statusBar->showMessage("No ports detected.");
    ui->actionRecord_PNG->setEnabled(false);
    connect(tmr, SIGNAL(timeout()), this, SLOT(on_pushButton_clicked()));
    tmr->start(3000);
    return;
  }
  /* List all available serial ports and populate ports combo box */
  for (QSerialPortInfo port : QSerialPortInfo::availablePorts()) {
    ui->comboPort->addItem(port.portName());
    UpdatePortControls();
    enable_com_controls(true);
    ui->actionConnect->trigger(); // autoconnect if arduino on same  port
    dialog->show();
 // dialog->buttonPressed();
    dialog->close();
  }
}
void MainWindow::UpdatePortControls() { /* Populate baud rate combo box with
                                           standard rates */
 // ui->comboBaud->addItem("1200");
 // ui->comboBaud->addItem("2400");
 // ui->comboBaud->addItem("4800");
  ui->comboBaud->addItem("9600");
 // ui->comboBaud->addItem("19200");
 // ui->comboBaud->addItem("38400");
  ui->comboBaud->addItem("57600");
  ui->comboBaud->addItem("115200");
  /* And some not-so-standard */
 // ui->comboBaud->addItem("128000");
 // ui->comboBaud->addItem("153600");
 // ui->comboBaud->addItem("230400");
 // ui->comboBaud->addItem("256000");
 // ui->comboBaud->addItem("460800");
 // ui->comboBaud->addItem("921600");

  // ui->comboMod->addItem ("921600");

  /* Select 115200 bits by default */
  // ui->comboBaud->setCurrentIndex (3);

  /* Populate data bits combo box */
  // ui->comboData->addItem ("8 bits");
  // ui->comboData->addItem ("7 bits");

  /* Populate parity combo box */
  // ui->comboParity->addItem ("none");
  // ui->comboParity->addItem ("odd");
  // ui->comboParity->addItem ("even");

  /* Populate stop bits combo box */
  // ui->comboStop->addItem ("1 bit");
  // ui->comboStop->addItem ("2 bits");

  /* Initialize the listwidget */
  ui->listWidget_Channels->clear();

  // try to load settings, or populate with default value
  ui->actionLoad_Settings->trigger();

}
/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/**
 * @brief Setup the plot area
 */
void MainWindow::setupPlot() {

  /* Remove everything from the plot */
  ui->plot->clearItems();

  /* Background for the plot area */
  ui->plot->setBackground(gui_colors[0]);

  /* Used for higher performance (see QCustomPlot real time example) */
  ui->plot->setNotAntialiasedElements(QCP::aeAll);
  QFont font;
  font.setStyleStrategy(QFont::NoAntialias);
  ui->plot->legend->setFont(font);

  /** See QCustomPlot examples / styled demo **/
  /* X Axis: Style */
  ui->plot->xAxis->grid()->setPen(QPen(gui_colors[2], 1, Qt::DotLine));
  ui->plot->xAxis->grid()->setSubGridPen(QPen(gui_colors[1], 1, Qt::DotLine));
  ui->plot->xAxis->grid()->setSubGridVisible(true);
  ui->plot->xAxis->setBasePen(QPen(gui_colors[2]));
  ui->plot->xAxis->setTickPen(QPen(gui_colors[2]));
  ui->plot->xAxis->setSubTickPen(QPen(gui_colors[2]));
  ui->plot->xAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);
  ui->plot->xAxis->setTickLabelColor(gui_colors[2]);
  ui->plot->xAxis->setTickLabelFont(font);

  // ui->plot->xAxis->setTickLabelType(QCPAxis::ltDateTime);   // Подпись
  // координат по Оси X в качестве Даты и Времени
  // ui->plot->xAxis->setDateTimeFormat("hh:mm");  // Устанавливаем формат даты
  // и времени

  /* Range */


 // ui->plot->xAxis->setRange (dataPointNumber - ui->spinPoints->value(), dataPointNumber);
  ui->plot->xAxis->setRange (ui->spinPoints->value(), dataPointNumber); // 0 on left

  // setColor(palette().WindowText, Qt::blue);
  ui->lcdChannelTemp_2->setStyleSheet("QLCDNumber{color:#B231F7}"); // Bottom
  ui->lcdChannelTemp_3->setStyleSheet("QLCDNumber{color:#E05123}"); // top
  ui->lcdChannelTemp->setStyleSheet("QLCDNumber{color:#A3E05C}"); // PCB

  //ui->plot->xAxis->setRange (dataPointNumber - spinPoints1, dataPointNumber);
  // //Reverse
  //ui->plot->xAxis->setRange(spinPoints1, dataPointNumber); // 0 on left
   

  /* Y Axis */
  ui->plot->yAxis->grid()->setPen(QPen(gui_colors[2], 1, Qt::DotLine));
  ui->plot->yAxis->grid()->setSubGridPen(QPen(gui_colors[1], 1, Qt::DotLine));
  ui->plot->yAxis->grid()->setSubGridVisible(true);
  ui->plot->yAxis->setBasePen(QPen(gui_colors[2]));
  ui->plot->yAxis->setTickPen(QPen(gui_colors[2]));
  ui->plot->yAxis->setSubTickPen(QPen(gui_colors[2]));
  ui->plot->yAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);
  ui->plot->yAxis->setTickLabelColor(gui_colors[2]);
  ui->plot->yAxis->setTickLabelFont(font);
 /* Range */
    ui->plot->yAxis->setRange (ui->spinAxesMin->value(), ui->spinAxesMax->value());
    /* User can change Y axis tick step with a spin box */
    //ui->plot->yAxis->setAutoTickStep (false);
    ui->plot->yAxis->ticker()->setTickCount(ui->spinYStep->value());

    /* User interactions Drag and Zoom are allowed only on X axis, Y is fixed manually by UI control */
    ui->plot->setInteraction (QCP::iRangeDrag, true);
   // ui->plot->setInteraction (QCP::iMultiSelect , true);
    ui->plot->setInteraction (QCP::iSelectPlottables, false);

   // ui->plot->graph(3)->selectable(QCP::SelectionType(QCP::stSingleData));
    ui->plot->setInteraction (QCP::iSelectLegend, true);
    ui->plot->axisRect()->setRangeDrag (Qt::Horizontal);
    ui->plot->axisRect()->setRangeZoom (Qt::Horizontal);


  /* Legend */
  QFont legendFont;
  legendFont.setPointSize(9);
  // ui->plot->legend->setVisible (true); // показать легенду
  ui->plot->legend->setVisible(false); // скрыть легенду
  ui->plot->legend->setFont(legendFont);
  ui->plot->legend->setBrush(gui_colors[3]);
  ui->plot->legend->setBorderPen(gui_colors[2]);
  /* By default, the legend is in the inset layout of the main axis rect. So
   * this is how we access it to change legend placement */
  ui->plot->axisRect()->insetLayout()->setInsetAlignment( 0, Qt::AlignTop | Qt::AlignLeft); // вставить квадрат с именами и выровнять
  // if(clkb==1){ui->plot->axisRect()->insetLayout()->setInsetAlignment (0,
  // Qt::AlignTop|Qt::AlignLeft);}

}
/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/**
 * @brief Enable/disable COM controls
 * @param enable true enable, false disable
 */
void MainWindow::enable_com_controls(bool enable) {
  /* Com port properties */
  ui->comboBaud->setEnabled(enable);
  // ui->comboData->setEnabled (enable);
  // ui->comboParity->setEnabled (enable);
  ui->comboPort->setEnabled(enable);
  // ui->comboStop->setEnabled (enable);

  /* Toolbar elements */
  ui->actionConnect->setEnabled(enable);
  ui->actionPause_Plot->setEnabled(!enable);
  ui->actionDisconnect->setEnabled(!enable);

  enable_heater_controls(!enable);
 
  loadSettings();
}

void MainWindow::enable_heater_controls(bool enable) {
  ui->pushButton_CANCEL->setEnabled(enable);
  ui->pushButton_OK->setEnabled(enable);
  ui->pushButton_UP->setEnabled(enable);
  ui->pushButton_LEFT->setEnabled(enable);
  ui->pushButton_RIGHT->setEnabled(enable);
  ui->pushButton_DOWN->setEnabled(enable);
  // ui->pushButton_2->setEnabled (enable);
}

void MainWindow::enable_advanced_controls(bool enable) {
  ui->PlotControlsBox->setVisible(enable);
 
}
/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/**
 * @brief Open the inside serial port; connect its signals
 * @param portInfo
 * @param baudRate
 * @param dataBits
 * @param parity
 * @param stopBits
 */
void MainWindow::openPort(QSerialPortInfo portInfo, int baudRate,
                          QSerialPort::DataBits, QSerialPort::Parity,
                          QSerialPort::StopBits) {
  serialPort = new QSerialPort(portInfo, nullptr); // Create a new serial port

  connect(this, SIGNAL(portOpenOK()), this,
          SLOT(portOpenedSuccess())); // Connect port signals to GUI slots
  connect(this, SIGNAL(portOpenFail()), this, SLOT(portOpenedFail()));
  connect(this, SIGNAL(portClosed()), this, SLOT(onPortClosed()));
  connect(this, SIGNAL(newData(QStringList)), this,
          SLOT(onNewDataArrived(QStringList)));
  connect(serialPort, SIGNAL(readyRead()), this, SLOT(readData()));
  //  connect (serialPort, SIGNAL(bytesWritten()), this, SLOT(writeData()));
  connect(this, SIGNAL(newData(QStringList)), this,
          SLOT(saveStream(QStringList)));

  if (serialPort->open(QIODevice::ReadWrite)) {

    QSerialPort::DataBits dataBits;
    dataBits = QSerialPort::Data8;

    QSerialPort::Parity parity;
    parity = QSerialPort::NoParity;

    QSerialPort::StopBits stopBits;
    stopBits = QSerialPort::OneStop;

    serialPort->setBaudRate(baudRate);
    serialPort->setParity(parity);
    serialPort->setDataBits(dataBits);
    serialPort->setStopBits(stopBits);
    // serialPort->setDataBits(QSerialPort::Data8);
    serialPort->setFlowControl(QSerialPort::NoFlowControl);
    // serialPort->setParity(QSerialPort::NoParity);
    // serialPort->setStopBits(QSerialPort::OneStop);

    emit portOpenOK();
  } else {
    emit portOpenedFail();
    qDebug() << serialPort->errorString();
  }
}
/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/**
 * @brief Slot for closing the port
 */
void MainWindow::onPortClosed() {
  // qDebug() << "Port closed signal received!";
  updateTimer.stop();
  connected = false;
  plotting = false;

  //--
  closeCsvFile();

  disconnect(serialPort, SIGNAL(readyRead()), this, SLOT(readData()));
  //  disconnect (serialPort, SIGNAL(readySend()), this, SLOT(writeData()));
  disconnect(this, SIGNAL(portOpenOK()), this,
             SLOT(portOpenedSuccess())); // Disconnect port signals to GUI slots
  disconnect(this, SIGNAL(portOpenFail()), this, SLOT(portOpenedFail()));
  disconnect(this, SIGNAL(portClosed()), this, SLOT(onPortClosed()));
  disconnect(this, SIGNAL(newData(QStringList)), this,
             SLOT(onNewDataArrived(QStringList)));
  disconnect(this, SIGNAL(newData(QStringList)), this,
             SLOT(saveStream(QStringList)));
  ui->PortControlsBox->setVisible(true);

  ui->HeaterControlsBox->setVisible(false);
  ui->PlotControlsBox->setVisible(false);
  ui->gridGroupBox->setVisible(false);
  ui->actionSave_Settings->setEnabled(false);
  ui->pushButton_2->setVisible(false);
  ui->ready->setVisible(false);
  ui->manualTemp->setVisible(false);
  ui->manulabutton->setVisible(false);
  ui->SimpleExpert->setVisible(false);

  // ui->spinmanual->setVisible(false);
  setupPlot();
}
/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/**
 * @brief Port Combo Box index changed slot; displays info for selected port
 * when combo box is changed
 * @param arg1
 */
void MainWindow::on_comboPort_currentIndexChanged(const QString &arg1) {
  QSerialPortInfo selectedPort(arg1); // Dislplay info for selected port
  ui->statusBar->showMessage(selectedPort.description());
}
/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/**
 * @brief Slot for port opened successfully
 */
void MainWindow::portOpenedSuccess() {
  // qDebug() << "Port opened signal received!";
  setupPlot(); // Create the QCustomPlot area
  ui->statusBar->showMessage("Connected!");
  enable_com_controls(false); // Disable controls if port is open

  if (ui->actionRecord_stream->isChecked()) {
    //--> Create new CSV file with current date/timestamp
    openCsvFile();
  }
  /* Lock the save option while recording */
  ui->actionRecord_stream->setEnabled(false);

  ui->PortControlsBox->setVisible(false); // Hide Port settings
  ui->PlotControlsBox->setVisible(false);
  ui->HeaterControlsBox->setVisible(true);   // Show Heater Controls
  ui->actionSave_Settings->setEnabled(true); // Enable Save Setings
  ui->pushButton_2->setVisible(true);
   

  //ui->spinmanual->setVisible(true);

  
  ui->manualTemp->setVisible(false);
  ui->manulabutton->setVisible(true);
  ui->HotStart->setEnabled(false);
  ui->HotStart->setVisible(true);
  ui->SimpleExpert->setVisible(true);

 /*
    ui->pushButton_OK->setStyleSheet(
        "QPushButton:enabled { background-color: rgb(0,250,0); }\n"
        "QPushButton:enabled { color: rgb(0,0,0); }\n");

    ui->pushButton_CANCEL->setStyleSheet(
        "QPushButton { background-color: grey; }\n");

    ui->pushButton_LEFT->setStyleSheet(
        "QPushButton:enabled { background-color: rgb(250,250,0); }\n"
        "QPushButton:enabled { color: rgb(0,0,0); }\n");

    ui->pushButton_RIGHT->setStyleSheet(
        "QPushButton:enabled { background-color: rgb(250,250,0); }\n"
        "QPushButton:enabled { color: rgb(0,0,0); }\n");

    // ui->pushButton_UP
    // ui->pushButton_DOWN

    ui->HotStart->setStyleSheet("QPushButton { background-color: grey; }\n");

    ui->manulabutton->setStyleSheet(
        "QPushButton:enabled { background-color: rgb(0,250,0); }\n"
        "QPushButton:enabled { color: rgb(0,0,0); }\n");

*/

  updateTimer.start(20); // Slot is refreshed 20 times per second
  connected = true;      // Set flags
  plotting = true;
  // setupPlot();
  profilread();
}
/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/**
 * @brief Slot for fail to open the port
 */
void MainWindow::portOpenedFail() {
  // qDebug() << "Port cannot be open signal received!";
  ui->statusBar->showMessage("Cannot open port!");
}
/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/**
 * @brief Replot
 */
void MainWindow::replot() {
  //ui->plot->xAxis->setRange (dataPointNumber - ui->spinPoints->value(), dataPointNumber);
  ui->plot->replot();
}
/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/**
 * @brief Slot for new data from serial port . Data is comming in QStringList
 * and needs to be parsed
 * @param newData
 */
void MainWindow::onNewDataArrived(QStringList newData) {
  static int data_members = 0;
  static int channel = 0;
  static int i = 0;
  volatile bool you_shall_NOT_PASS = false;

  /* When a fast baud rate is set (921kbps was the first to starts to bug),
     this method is called multiple times (2x in the 921k tests), so a flag
     is used to throttle
     TO-DO: Separate processes, buffer data (1) and process data (2) */
  while (you_shall_NOT_PASS) {
  }
  you_shall_NOT_PASS = true;

  if (plotting) {
    /* Get size of received list */
    data_members = newData.size();
    if (data_members > 9) {data_members = 9;} // limit number of plotted channels to 9

    /* Parse data */
    for (i = 0; i < data_members; i++) {
      /* Update number of axes if needed */
      while (ui->plot->plottableCount() <= channel) {
        /* Add new channel data */

        ui->plot->addGraph(); //  добавить   график
        ui->plot->graph()->setPen(line_colors[channels % CUSTOM_LINE_COLORS]);
        QString channelscount =
            QString("Count %1").arg(m_prefs.channelnames.size());
        if (i < (m_prefs.channelnames.size())) // /3 because have x3 elements
        {
          ui->plot->graph()->setName(m_prefs.channelnames.value(i).channelName);
          ui->plot->graph()->setVisible(
              m_prefs.channelnames.value(i).channelVisibie);
          // ui->statusBar->showMessage("channelscount");
        } else {
          ui->plot->graph()->setName(QString("Channel %1").arg(channels + 1));
        }

        ui->plot->graph()->setVisible(
            m_prefs.channelnames.value(i).channelVisibie);
        if (ui->plot->legend->item(channels)) {
          ui->plot->legend->item(channels)->setTextColor(
              line_colors[channels % CUSTOM_LINE_COLORS]);
        }
        ui->listWidget_Channels->addItem(ui->plot->graph()->name());
        ui->listWidget_Channels->item(channel)->setForeground(
            QBrush(line_colors[channels % CUSTOM_LINE_COLORS]));
        channels++;

        ui->statusBar->showMessage(channelscount);
      }

      /* [TODO] Method selection and plotting */
      /* X-Y */
      if (ui->plot->plottableCount() == data_members) { //если у нас графиков столько, сколько приходит данных то добавляем статичные графики
          for (i = 9; i < 12; i++) {
          ui->plot->addGraph();
          ui->plot->graph()->setName(m_prefs.channelnames.value(i).channelName);
          }
      }
      /* Rolling (v1.0.0 compatible) */
      else {
        /* Add data to Graph 0 */
        ui->plot->graph(channel)->addData (dataPointNumber, newData[channel].toDouble()); // add piont to correspoding channel
        ui->lcdChannelTemp->display(newData[5]);    //PCB
        ui->lcdChannelTemp_2->display(newData[4]);  // Bottom
        ui->lcdChannelTemp_3->display(newData[3]);  // Top
        /* Increment data number and channel */
        channel++;
      }
    }

    /* Post-parsing */
    /* X-Y */
    if (0) {

    }
    /* Rolling (v1.0.0 compatible) */
    else {
      dataPointNumber++;
      channel = 0;
    }
  }
  you_shall_NOT_PASS = false;
}
/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/**
 * @brief Slot for spin box for plot minimum value on y axis
 * @param arg1
 */
 void MainWindow::on_spinAxesMin_valueChanged(int arg1)
{
 //arg1=0;
   ui->plot->yAxis->setRangeLower (arg1);
   ui->plot->replot();
}
/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/**
 * @brief Slot for spin box for plot maximum value on y axis
 * @param arg1
 */
 void MainWindow::on_spinAxesMax_valueChanged(int arg1)
{
 //arg1=600;
    ui->plot->yAxis->setRangeUpper (arg1);
    ui->plot->replot();
}
/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/**
 * @brief Read data for inside serial port
 */
void MainWindow::readData() {
  if (serialPort->bytesAvailable()) {        // If any bytes are available
    QByteArray data = serialPort->readAll(); // Read all data in QByteArray

    if (!data.isEmpty() && data.length()>4) { // If the byte array is not empty
      //  char *temp = data.data(); // Get a '\0'-terminated char* to the data
    //      unsigned char *temp = (unsigned char*)data.data();

        unsigned char *temp = reinterpret_cast<unsigned char *>(data.data());
  //    unsigned char *tempal = reinterpret_cast<unsigned char *>(data.data());
  //    unsigned char *tempst = reinterpret_cast<unsigned char *>(data.data());

      if (!filterDisplayedData) { // Merge recieved data if it come in parts,
                                  // print when first \r found
        receivedDataRaw.append(data);
        for (int i = 0; receivedDataRaw[i] != '\0'; i++) {
          if (receivedDataRaw[i] == '\r') {  // clean
            if (!receivedDataRaw.startsWith('\n') &&
                !receivedDataRaw.startsWith('\r')) {
              ui->textEdit_UartWindow->append(receivedDataRaw.left(i));
              // emit sendData(ui->textEdit_UartWindow->toPlainText()); //
              // вызываем сигнал, в котором передаём данные
              receivedDataRaw.remove(0, i); // print in textEdit from 0 to i
            } else {
              receivedDataRaw.remove(
                  0, 1); // if found more than 1 \n or \r in row - delete them
                         // receivedDataRaw=receivedDataRaw.trimmed();
            }
            if (receivedDataRaw[i + 1] == '\0')
              i = 0;
          }

          if (i >= receivedDataRaw.size()) {
            break;
          }
        }
      }

      for (int i = 0; temp[i] != '\0'; i++) { // Iterate over the char*
        switch (STATE) { // Switch the current state of the message
        case WAIT_START: // If waiting for start [$], examine each char
          if (temp[i] ==
              START_MSG) { // If the char is $, change STATE to IN_MESSAGE
            receivedData
                .clear(); // Clear temporary QString that holds the message
            STATE = IN_MESSAGE;
            break; // Break out of the switch
          }
          if (temp[i] ==
              COMMAND_MSG) { // If the char is !, change STATE to IN_COMMAND
            receivedData
                .clear(); // Clear temporary QString that holds the message
            STATE = IN_COMMAND;
            break; // Break out of the switch
          }

          if (temp[i] ==
              DONG_MSG) { // If the char is @, change STATE to IN_STATUS
            statusData.clear();
            receivedData
                .clear(); // Clear temporary QString that holds the message
            STATE = IN_STATUS;
            break; // Break out of the switch
          }

          break; // If waiting for start [$], examine each char

        case IN_MESSAGE:              // If state is IN_MESSAGE

          if (temp[i] == CLEAR_MSG) { // If recieve # symbol IN-MESSAGE

            clear_plottables_graph();
            receivedData.clear();
            STATE = WAIT_START;
          }
          if (temp[i] == DONG_MSG) { //  @ symbol IN-MESSAGE

           // dong->play();

            ui->statusBar->showMessage("SOUND!");
            receivedData.clear();
            STATE = WAIT_START;
          }
          if (temp[i] ==
              END_MSG) { // If char examined is ;, switch state to END_MSG
            STATE = WAIT_START;
            QStringList incomingData = receivedData.split(
                ' '); // Split string received from port and put it into list
            if (filterDisplayedData) {
              ui->textEdit_UartWindow->append(receivedData);
              receivedData.clear();
            }
            emit newData(
                incomingData); // Emit signal for data received with the list
            receivedData.clear();
            if ((PREV_STATE == IN_COMMAND || PREV_STATE == IN_STATUS)){STATE = PREV_STATE;}
            break;
          }
          if (isdigit(temp[i]) || isspace(temp[i]) || temp[i] == '-' ||
              temp[i] == '.') {
            /* If examined char is a digit, and not '$' or ';', append it to
             * temporary string */
            receivedData.append(temp[i]);
          }
          break;

        case IN_COMMAND: // If state is IN_COMMAND after recieve "!"
            if (temp[i] ==
              START_MSG) { // If char examined is $, - we have interrupt with temperatures data.
            PREV_STATE = STATE;
            STATE = IN_MESSAGE;
            }
            if (temp[i] ==
              END_MSG) { // If char examined is ;, switch state to END_MSG
            STATE = WAIT_START;
            if (receivedData.contains(':')) {
              emit sendData(receivedData); // emit signal

              QStringList data = receivedData.split(
                  ':'); // Split string received from port and put it into list

              profiledata.addValue(data[0],data[1]);

              if (data[0] == "Set Temp") {
                ui->manualTemp->display(data[1]);
              }

              if (data[0] == "profile") {
                ui->profile->setText(data[1]);
              }

              if (data[0] == "aliasprofile") {
                ui->aliasprofile->setText(data[1]);
              }
              if (data[0] == "profile_transfer_finished"){
                  if(ui->plot->plottableCount() >= 9) {

                      plot_All_Profile();
                  }
              }
              if (data[0] == "second"){

              dataPointNumber = (data[1]).toInt();
              }

              data.clear();
            }

            receivedData.clear();
            PREV_STATE = WAIT_START;
            break;
          }
          if (isalpha(temp[i]) || isdigit(temp[i]) || temp[i] == ':' ||
              temp[i] == ',' || temp[i] == '_' || temp[i] == '-' ||
              temp[i] == '.' || isspace(temp[i])) {
            receivedData.append(temp[i]);
          }
          break;

        case IN_STATUS:
            if (temp[i] ==
              START_MSG) { // If char examined is $, - we have interrupt with temperatures data.
            PREV_STATE = STATE;
            STATE = IN_MESSAGE;
            }
          if (temp[i] == END_MSG) { //  switch state to END_MSG
            STATE = WAIT_START;

            if (statusData == '0') {
              ui->ready->setVisible(false); // IDLE

            } else if (statusData == '1') {
              ui->ready->setVisible(true);
              ui->ready->setText("WARMUP...");
              dataPointNumber = 0;   // Lock graph redraw

              for (int j = 0; j <= 8; j++) {   // Draw round point at Y Axis during Warmup

                  ui->plot->graph(j)->setLineStyle(QCPGraph::lsNone);                   // Change 0-8 plot to fat point
                  ui->plot->graph(j)->setPen(line_colors[j % CUSTOM_LINE_COLORS]);
                  ui->plot->graph(j)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 12));
                  double lastValue = ui->plot->graph(j)->data()->at(1)->value;
                  ui->plot->graph(j)->data()->clear();
                  ui->plot->graph(j)->setData({0,2}, {lastValue});
                 // ui->plot->graph(j)->setData(dataPointNumber, lastValue);


              }

            } else if (statusData == '2') {
              ui->ready->setVisible(true);
              ui->ready->setText("RUN");
              ui->pushButton_UP->setEnabled(true);
              ui->pushButton_UP->setStyleSheet(
                  "QPushButton:enabled { background-color: rgb(50,150,250); }\n"
                  "QPushButton:enabled { color: rgb(0,0,0); }\n");
              ui->pushButton_DOWN->setEnabled(true);
              ui->pushButton_DOWN->setStyleSheet(
                  "QPushButton:enabled { background-color: rgb(50,150,250); }\n"
                  "QPushButton:enabled { color: rgb(0,0,0); }\n");
            //  plot_All_Profile(); // Вывести график термопрофиля
              for (int j=0; j<=8; j++){  // Change all plott to point
                  ui->plot->graph(j)->setLineStyle(QCPGraph::lsLine);
                  ui->plot->graph(j)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone , 1));
              }

            } else if (statusData == '3') {
              ui->ready->setVisible(true);
              ui->ready->setText("AUTO PAUSE");

            } else if (statusData == '4') {
              ui->ready->setVisible(true);
              ui->ready->setText("MANUAL");
              ui->manualTemp->setVisible(true);

            } else if (statusData == '5') {
              ui->ready->setVisible(true);
              ui->ready->setText("FINISH !");
              for (int j=0; j<=8; j++){  // Change all plott to point
                  ui->plot->graph(j)->setLineStyle(QCPGraph::lsLine);
                  ui->plot->graph(j)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone , 1));
              }

            } else if (statusData == '6') {
              ui->ready->setVisible(false);

    //        } else if (statusData == '7') {
    //                    ui->error->setVisible(true);
            }


            statusData.clear();
            PREV_STATE = WAIT_START;
          }

          if (isdigit(temp[i])) {
            statusData.append(temp[i]);
          }
          break;

        default:
          break;
        }
      }
    }
  }
}
/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/**
 * @brief Number of axes combo; when changed, display axes colors in status bar
 * @param index
 */
// void MainWindow::on_comboAxes_currentIndexChanged(int index)
//{
//     if(index == 0) {
//       ui->statusBar->showMessage("Axis 1: Red");
//     } else if(index == 1) {
//         ui->statusBar->showMessage("Axis 1: Red; Axis 2: Yellow");
//     } else {
//         ui->statusBar->showMessage("Axis 1: Red; Axis 2: Yellow; Axis 3:
//         Green");
//     }
// }
/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/**
 * @brief Spin box for changing the Y Tick step
 * @param arg1
 */
 void MainWindow::on_spinYStep_valueChanged(int arg1)
{
     ui->plot->yAxis->ticker()->setTickCount(arg1);
     ui->plot->replot();
     ui->spinYStep->setValue(ui->plot->yAxis->ticker()->tickCount());
}
/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/**
 * @brief Save a PNG image of the plot to current EXE directory
 */
void MainWindow::on_actionRecord_PNG_triggered() {
  ui->plot->savePng(QString::number(dataPointNumber) + ".png", 1920, 1080, 2,
                    50);
  ui->statusBar->showMessage("PNG Saved.");
}
/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */


/**
 * @brief Send plot wheelmouse to spinbox
 * @param event
 */
void MainWindow::on_mouse_wheel_in_plot (QWheelEvent *event)
{
  QWheelEvent inverted_event = QWheelEvent(event->position(), event->globalPosition(),
                                           -event->pixelDelta(), -event->angleDelta(),
                                           0, Qt::Vertical, event->buttons(), event->modifiers());
  QApplication::sendEvent (ui->spinPoints, &inverted_event);
}
/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/**
 * @brief Select both line and legend (channel)
 * @param plottable
 * @param event
 */
void MainWindow::channel_selection(void) {
  /* synchronize selection of graphs with selection of corresponding legend
   * items */
  for (int i = 0; i < ui->plot->graphCount(); i++) {
    QCPGraph *graph = ui->plot->graph(i);
    QCPPlottableLegendItem *item = ui->plot->legend->itemWithPlottable(graph);
    if (item->selected()) {
      item->setSelected(true);
      //          graph->set (true);
    } else {
      item->setSelected(false);
      //        graph->setSelected (false);
    }
  }
}
/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/**
 * @brief Rename a graph by double clicking on its legend item
 * @param legend
 * @param item
 */
void MainWindow::legend_double_click(QCPLegend *legend,
                                     QCPAbstractLegendItem *item,
                                     QMouseEvent *event) {
  Q_UNUSED(legend)
  Q_UNUSED(event)
  /* Only react if item was clicked (user could have clicked on border padding
   * of legend where there is no item, then item is 0) */
  if (item) {
    QCPPlottableLegendItem *plItem =
        qobject_cast<QCPPlottableLegendItem *>(item);
    bool ok;
    QString newName = QInputDialog::getText(
        this, "Change channel name", "New name:", QLineEdit::Normal,
        plItem->plottable()->name(), &ok, Qt::Popup);
    if (ok) {
      plItem->plottable()->setName(newName);
      for (int i = 0; i < ui->plot->graphCount(); i++) {
        ui->listWidget_Channels->item(i)->setText(ui->plot->graph(i)->name());
      }
      ui->plot->replot();
    }
  }
}
/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/**
 * @brief Spin box controls how many data points are collected and displayed
 * @param arg1
 */
 void MainWindow::on_spinPoints_valueChanged (int arg1)
{
     Q_UNUSED(arg1)
    ui->plot->xAxis->setRange (ui->spinPoints->value(), 0);
    ui->plot->replot();
}
/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/**
 * @brief Shows a window with instructions
 */
void MainWindow::on_actionHow_to_use_triggered() {
  helpWindow = new HelpWindow(this);
  helpWindow->setWindowTitle("How to use this application");
  // helpWindow->show();
  helpWindow->exec();
}

/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/**
 * @brief Connects to COM port or restarts plotting
 */
void MainWindow::on_actionConnect_triggered() {
  if (connected) {
    /* Is connected, restart if paused */
    if (!plotting) {       // Start plotting
      updateTimer.start(); // Start updating plot timer
      plotting = true;
      ui->actionConnect->setEnabled(false);
      ui->actionPause_Plot->setEnabled(true);
      ui->statusBar->showMessage("Plot continued!");
      ui->actionRecord_PNG->setEnabled(true);
    }
  } else {
    /* If application is not connected, connect */
    /* Get parameters from controls first */
    QSerialPortInfo portInfo(
        ui->comboPort
            ->currentText()); // Temporary object, needed to create QSerialPort
    int baudRate =
        ui->comboBaud->currentText().toInt(); // Get baud rate from combo box
    // int dataBitsIndex = ui->comboData->currentIndex(); // Get index of data
    // bits combo box int parityIndex = ui->comboParity->currentIndex(); // Get
    // index of parity combo box int stopBitsIndex =
    // ui->comboStop->currentIndex();                                // Get
    // index of stop bits combo box
    QSerialPort::DataBits dataBits;
    QSerialPort::Parity parity;
    QSerialPort::StopBits stopBits;

    /* Set data bits according to the selected index */
    //  switch (dataBitsIndex)
    //   {
    //    case 0:
    dataBits = QSerialPort::Data8;
    //     break;
    //   default:
    //     dataBits = QSerialPort::Data7;
    //   }

    /* Set parity according to the selected index */
    //  switch (parityIndex)
    //   {
    //   case 0:
    parity = QSerialPort::NoParity;
    //     break;
    //   case 1:
    //     parity = QSerialPort::OddParity;
    //     break;
    //   default:
    //    parity = QSerialPort::EvenParity;
    //  }

    /* Set stop bits according to the selected index */
    // switch (stopBitsIndex)
    //   {
    //   case 0:
    stopBits = QSerialPort::OneStop;
    //    break;
    //  default:
    //    stopBits = QSerialPort::TwoStop;
    //  }

    /* Use local instance of QSerialPort; does not crash */
    serialPort = new QSerialPort(portInfo, nullptr);

    /* Open serial port and connect its signals */

    openPort(portInfo, baudRate, dataBits, parity, stopBits);
    // openPort (QSerialPort::ReadWrite);
  }
}
/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/**
 * @brief Keep COM port open but pause plotting
 */
void MainWindow::on_actionPause_Plot_triggered() {
  if (plotting) {
    updateTimer.stop(); // Stop updating plot timer
    plotting = false;
    ui->actionConnect->setEnabled(true);
    ui->actionPause_Plot->setEnabled(false);
    ui->statusBar->showMessage("Plot paused, new data will be ignored");
  }
}
/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/**
 * @brief Start saving data to file
 */
void MainWindow::on_actionRecord_stream_triggered() {
  if (ui->actionRecord_stream->isChecked()) {
    ui->statusBar->showMessage("Data will be stored in csv file");
  } else {
    ui->statusBar->showMessage("Data will not be stored anymore");
  }
}
/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/**
 * @brief Closes COM port and stop plotting
 */
void MainWindow::on_actionDisconnect_triggered() {
  if (connected) {

    ui->pushButton_CANCEL->click(); //Stop any process in arduino
    serialPort->close();  // Close serial port
    emit portClosed();    // Notify application
    delete serialPort;    // Delete the pointer
    serialPort = nullptr; // Assign NULL to dangling pointer

    ui->statusBar->showMessage("Disconnected!");

    connected = false; // Set connected status flag to false
    ui->actionConnect->setEnabled(true);

    plotting = false; // Not plotting anymore
    ui->actionPause_Plot->setEnabled(false);
    ui->actionDisconnect->setEnabled(false);
    ui->actionRecord_stream->setEnabled(true);
    receivedData.clear(); // Clear received string

    ui->actionRecord_PNG->setEnabled(false);
    enable_com_controls(true);
    dataPointNumber = 0;
  }
}
/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/**
 * @brief Clear all channels data and reset plot area
 *
 * This function will not delete the channel itself (legend will stay)
 * implemented Solve issue #7: press clear button delete data but not labels
 */
void MainWindow::on_actionClear_triggered() {
  ui->plot->clearPlottables();
  ui->listWidget_Channels->clear();
  ui->textEdit_UartWindow->clear();
  ui->plot->replot();
  channels = 0;
  dataPointNumber = 0;
  // if(clkb==1){ui->plot->axisRect()->insetLayout()->setInsetAlignment (0,
  // Qt::AlignTop|Qt::AlignLeft);}

  emit setupPlot();
  ui->plot->replot();
}
/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/**
 * @brief Open a new CSV file to save received data
 *
 */
void MainWindow::openCsvFile(void) {
  m_csvFile =
      new QFile(QDateTime::currentDateTime().toString("yyyy-MM-d-HH-mm-ss-") +
                "data-out.csv");
  if (!m_csvFile)
    return;
  if (!m_csvFile->open(QIODevice::ReadWrite | QIODevice::Text))
    return;
}
/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/**
 * @brief Open a new CSV file to save received data
 *
 */
void MainWindow::closeCsvFile(void) {
  if (!m_csvFile)
    return;
  m_csvFile->close();
  if (m_csvFile)
    delete m_csvFile;
  m_csvFile = nullptr;
}
/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/**
 * @brief Open a new CSV file to save received data
 *
 */
void MainWindow::saveStream(QStringList newData) {
  if (!m_csvFile)
    return;
  if (ui->actionRecord_stream->isChecked()) {
    QTextStream out(m_csvFile);
    foreach (const QString &str, newData) {
      out << str << ",";
    }
    out << "\n";
  }
}

/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void MainWindow::on_pushButton_TextEditHide_clicked() {
  if (ui->pushButton_TextEditHide->isChecked()) {
    ui->textEdit_UartWindow->setVisible(false);
    ui->textSend_UartWindow->setVisible(false);
    ui->pushButton_TextEditHide->setText("Show Console");
  } else {
    ui->textEdit_UartWindow->setVisible(true);
    ui->textSend_UartWindow->setVisible(true);
    ui->pushButton_TextEditHide->setText("Hide Console");
  }
}

void MainWindow::on_pushButton_ShowallData_clicked() {
  if (ui->pushButton_ShowallData->isChecked()) {
    filterDisplayedData = false;
    ui->pushButton_ShowallData->setText("Filter Incoming Data");
  } else {
    filterDisplayedData = true;
    ui->pushButton_ShowallData->setText("Show All Incoming Data");
  }
}

void MainWindow::on_pushButton_SendToCom_clicked() // Send data to com
{
  if (connected) {
    QByteArray dataBuf;
    QString data = ui->textSend_UartWindow->toPlainText().toUtf8();
    if (data.isEmpty()) {
      ui->statusBar->showMessage("Nothing to send");
      return;
    }
    dataBuf = data.append("\r\n").toUtf8();
    data.clear();
    if (serialPort != nullptr && connected) {
      serialPort->write(dataBuf);
      serialPort->flush();

      serialPort->waitForBytesWritten(50);
      serialPort->waitForReadyRead(50);

      ui->statusBar->showMessage("Sended: " + dataBuf);
      dataBuf.clear();
      ui->textSend_UartWindow->clear();
    }

  } else {
    ui->statusBar->showMessage("Cant Send, Port not open");
  }
}

// void MainWindow::setSettingsValue(QString name, QString value){
//     QLineEdit * edit = centralWidget()->findChild<QLineEdit *>(name);
// emit sendData(name.);     //emit signal

//    if (edit != nullptr)
//    { edit->setText(value);}
// emit sendData(edit->text());     //emit signal
//}

void MainWindow::on_pushButton_OK_clicked() {
  QString data = ui->textSend_UartWindow->toPlainText();
  ui->textSend_UartWindow->clear();
  ui->textSend_UartWindow->insertPlainText("S");
  ui->pushButton_SendToCom->click();
  ui->textSend_UartWindow->insertPlainText(data);

  ui->pushButton_CANCEL->setStyleSheet(
      "QPushButton:enabled { background-color: rgb(250,0,0); }\n"
      "QPushButton:enabled { color: rgb(0,0,0); }\n");

  ui->pushButton_OK->setStyleSheet("QPushButton { background-color: grey; }\n");

  ui->pushButton_LEFT->setStyleSheet(
      "QPushButton { background-color: grey; }\n");
  ui->pushButton_RIGHT->setStyleSheet(
      "QPushButton { background-color: grey; }\n");

  ui->pushButton_RIGHT->setEnabled(false);
  ui->pushButton_LEFT->setEnabled(false);
  ui->pushButton_OK->setEnabled(false);
  ui->pushButton_CANCEL->setEnabled(true);

  // ui->pushButton_2->setEnabled(false);
  ui->pushButton_2->setEnabled(false);
  ui->pushButton_2->setStyleSheet("QPushButton { background-color: grey; }\n");

  ui->HotStart->setEnabled(false);
  ui->HotStart->setStyleSheet("QPushButton { background-color: grey; }\n");

  ui->manulabutton->setEnabled(false);
  ui->manulabutton->setStyleSheet("QPushButton { background-color: grey; }\n");

  data.clear();
}
void MainWindow::on_pushButton_UP_clicked() {
  QString data = ui->textSend_UartWindow->toPlainText();
  ui->textSend_UartWindow->clear();
  ui->textSend_UartWindow->insertPlainText("U");
  ui->pushButton_SendToCom->click();
  ui->textSend_UartWindow->insertPlainText(data);
  data.clear();
}
void MainWindow::on_pushButton_DOWN_clicked() {
  QString data = ui->textSend_UartWindow->toPlainText();
  ui->textSend_UartWindow->clear();
  ui->textSend_UartWindow->insertPlainText("D");
  ui->pushButton_SendToCom->click();
  ui->textSend_UartWindow->insertPlainText(data);
  data.clear();
}
void MainWindow::on_pushButton_LEFT_clicked() {
  QString data = ui->textSend_UartWindow->toPlainText();
  ui->textSend_UartWindow->clear();
  ui->textSend_UartWindow->insertPlainText("L");
  ui->pushButton_SendToCom->click();
  ui->textSend_UartWindow->insertPlainText("P");
  ui->pushButton_SendToCom->click();
  ui->textSend_UartWindow->insertPlainText(data);
  //alflag = 1;
  receivedWord.clear();
  receivedProfile.clear();
  data.clear();
}
void MainWindow::on_pushButton_RIGHT_clicked() {
  QString data = ui->textSend_UartWindow->toPlainText();
  ui->textSend_UartWindow->clear();
  ui->textSend_UartWindow->insertPlainText("R");
  ui->pushButton_SendToCom->click();
  ui->textSend_UartWindow->clear();
  ui->textSend_UartWindow->insertPlainText("P");
  ui->pushButton_SendToCom->click();
  ui->textSend_UartWindow->insertPlainText(data);
  //alflag = 1;
  receivedWord.clear();
  receivedProfile.clear();
  data.clear();
}
void MainWindow::on_pushButton_CANCEL_clicked() {
  QString data = ui->textSend_UartWindow->toPlainText();
  ui->textSend_UartWindow->clear();
  ui->textSend_UartWindow->insertPlainText("E");
  ui->pushButton_SendToCom->click();
  ui->textSend_UartWindow->insertPlainText(data);

  ui->pushButton_OK->setStyleSheet(
      "QPushButton:enabled { background-color: rgb(0,250,0); }\n"
      "QPushButton:enabled { color: rgb(0,0,0); }\n");

  ui->pushButton_CANCEL->setStyleSheet(
      "QPushButton { background-color: grey; }\n");

  ui->manualTemp->setVisible(false);
  ui->pushButton_RIGHT->setEnabled(true);
  ui->pushButton_LEFT->setEnabled(true);
  //ui->pushButton_CANCEL->setEnabled(false);
  ui->pushButton_OK->setEnabled(true);

  ui->pushButton_LEFT->setStyleSheet(
      "QPushButton:enabled { background-color: rgb(250,250,0); }\n"
      "QPushButton:enabled { color: rgb(0,0,0); }\n");

  ui->pushButton_RIGHT->setStyleSheet(
      "QPushButton:enabled { background-color: rgb(250,250,0); }\n"
      "QPushButton:enabled { color: rgb(0,0,0); }\n");

  // ui->pushButton_2->setEnabled(true);
  ui->pushButton_2->setEnabled(true);
  ui->pushButton_2->setStyleSheet("");
  ui->ready->setVisible(false);
  ui->HotStart->setEnabled(true);
  ui->HotStart->setStyleSheet(
      "QPushButton:enabled { background-color: rgb(250,150,50); }\n"
      "QPushButton:enabled { color: rgb(0,0,0); }\n");

  ui->pushButton_UP->setEnabled(false);
  ui->pushButton_UP->setStyleSheet("QPushButton { background-color: grey; }\n");

  ui->pushButton_DOWN->setEnabled(false);
  ui->pushButton_DOWN->setStyleSheet(
      "QPushButton { background-color: grey; }\n");

  ui->manulabutton->setVisible(true);
  ui->manulabutton->setStyleSheet(
      "QPushButton:enabled { background-color: rgb(0,250,0); }\n"
      "QPushButton:enabled { color: rgb(0,0,0); }\n");
  ui->manulabutton->setEnabled(true);

  data.clear();
}

void MainWindow::on_pushButton_AutoScale_clicked() {
   ui->plot->yAxis->rescale(true);
  //if (spinAxesMax1 == 450) {
   // spinAxesMax1 = 240;
   // ui->plot->yAxis->setRange(0, 240);
 // } else if (spinAxesMax1 == 240) {
  //  spinAxesMax1 = 450;
  //  ui->plot->yAxis->setRange(spinAxesMin1, 450);
 // };
}

void MainWindow::on_SimpleExpert_clicked() {  // Simple\expert
  // QString namebutton = ui->SimpleExpert->text();

  if (ui->SimpleExpert->isChecked()) {
    //clkb = 0;
    ui->plot->legend->setVisible(false);
    ui->listWidget_Channels->setVisible(false);
    ui->gridGroupBox->setVisible(false);
    ui->pushButton_ResetVisible->setVisible(false);
    ui->PlotControlsBox->setVisible(false); // Hide uPlotControlsBox
    ui->SimpleExpert->setText("EXPERT");
  } else {
    //clkb = 1;
    //ui->plot->legend->setVisible(true);
    ui->listWidget_Channels->setVisible(true);
    ui->gridGroupBox->setVisible(true);
    //ui->pushButton_ResetVisible->setVisible(true);
    ui->PlotControlsBox->setVisible(true); // Hide uPlotControlsBox
    ui->SimpleExpert->setText("SIMPLE");
  }
}

void MainWindow::on_pushButton_ResetVisible_clicked() {
  for (int i = 0; i < ui->plot->graphCount(); i++) {
    ui->plot->graph(i)->setVisible(true);
    ui->listWidget_Channels->item(i)->setBackground(Qt::NoBrush);
  }
}

void MainWindow::on_listWidget_Channels_itemDoubleClicked(
    QListWidgetItem *item) {
  int graphIdx = ui->listWidget_Channels->currentRow();

  if (ui->plot->graph(graphIdx)->visible()) {
    ui->plot->graph(graphIdx)->setVisible(false);
    item->setBackground(Qt::black);
  } else {
    ui->plot->graph(graphIdx)->setVisible(true);
    item->setBackground(Qt::NoBrush);
  }
  ui->plot->replot();
}

void MainWindow::on_pushButton_clicked() {
  ui->comboPort->clear();
  ui->comboBaud->clear();
  /* List all available serial ports and populate ports combo box */
  for (QSerialPortInfo port : QSerialPortInfo::availablePorts()) {
    ui->comboPort->addItem(port.portName());
    if (ui->comboPort->count() != 0) {
      UpdatePortControls();
      enable_com_controls(true);
      disconnect(tmr, SIGNAL(timeout()), this, SLOT(on_pushButton_clicked()));
      tmr->stop();
    }
  }
}

/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void MainWindow::on_actionLoad_Settings_triggered() {
  loadSettings();
  ui->comboPort->setCurrentIndex(m_prefs.port);
  ui->comboBaud->setCurrentIndex(m_prefs.baud);
  // ui->comboData->setCurrentIndex(m_prefs.data);
  // ui->comboParity->setCurrentIndex(m_prefs.parity);
  // ui->comboStop->setCurrentIndex(m_prefs.stop);
   ui->spinPoints->setValue(m_prefs.spinPoints);
   ui->spinYStep->setValue(m_prefs.spinYStep);
   ui->spinAxesMin->setValue(m_prefs.spinAxesMin);
   ui->spinAxesMax->setValue(m_prefs.spinAxesMax);
  if (ui->SimpleExpert->isChecked() != m_prefs.SimpleExpert) {
    ui->SimpleExpert->click();
   //setupPlot();
  }
  if (ui->pushButton_TextEditHide->isChecked() != m_prefs.TextEditHide) {
    ui->pushButton_TextEditHide->click();
  }
  if (ui->pushButton_ShowallData->isChecked() != m_prefs.ShowallData) {
    ui->pushButton_ShowallData->click();
  }
  if (ui->actionRecord_stream->isChecked() != m_prefs.Record_stream) {
    ui->actionRecord_stream->trigger();
  }
  ui->lcdChannelTemp->setVisible(m_prefs.ChannelOnLCDVisible);
  // ui->lcdChannelTemp_2->setPalette( Qt::green);
  ui->statusBar->showMessage("Settings Loaded");


}

/**
 * @brief Load settings from config file and populate preferences
 *
 */
void MainWindow::loadSettings() {
  QSettings settings("serial_port_plotter.ini", QSettings::IniFormat);
  m_prefs.port = settings.value("port", 0).toInt();
  m_prefs.baud = settings.value("baud", 3).toInt();
  // m_prefs.data = settings.value("data", 0).toInt();
  // m_prefs.parity = settings.value("parity", 0).toInt();
  // m_prefs.stop = settings.value("stop", 0).toInt();
  m_prefs.spinPoints = settings.value("spinPoints", 600).toInt();
  m_prefs.spinYStep = settings.value("spinYStep", 10).toInt();
  m_prefs.spinAxesMin = settings.value("spinAxesMin", -100).toInt();
  m_prefs.spinAxesMax = settings.value("spinAxesMax", 400).toInt();
  m_prefs.TextEditHide = settings.value("TextEditHide", true).toBool();
  m_prefs.ShowallData = settings.value("ShowallData", true).toBool();
  m_prefs.Record_stream = settings.value("Record_stream", true).toBool();
  m_prefs.LCD_Channel = settings.value("ChannelOnLCD", 2).toInt();
  m_prefs.ChannelOnLCDVisible =
      settings.value("ChannelOnLCDVisible", true).toBool();
  m_prefs.SimpleExpert = settings.value("SimpleExpert", true).toBool();

  m_prefs.channelnames.clear();

  int size =
      settings.beginReadArray("channelnames"); // Read channelNames in cycle
  // for (int i = 0; i < 6; ++i) {

  for (int i = 0; i < size; ++i) {
    settings.setArrayIndex(i);
    LegendChannelNames legendchannelnames;
    legendchannelnames.channelName = settings.value("channelName").toString();
    legendchannelnames.channelVisibie =
        settings.value("channelVisibie").toBool();
    m_prefs.channelnames.append(legendchannelnames);
  }
  settings.endArray();

}

/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void MainWindow::on_actionSave_Settings_triggered() {
  saveSettings();
  ui->statusBar->showMessage("Settings Saved");
}

/**
 * @brief Load settings from config file and populate preferences
 *
 */
void MainWindow::saveSettings() {
  QSettings settings("serial_port_plotter.ini", QSettings::IniFormat);
  settings.setValue("port", ui->comboPort->currentIndex());
  settings.setValue("baud", ui->comboBaud->currentIndex());
  // settings.setValue("data", ui->comboData->currentIndex());
  // settings.setValue("parity", ui->comboParity->currentIndex());
  // settings.setValue("stop", ui->comboStop->currentIndex());
  settings.setValue("spinPoints", ui->spinPoints->value());
  settings.setValue("spinYStep", ui->spinYStep->value());
  settings.setValue("spinAxesMin", ui->spinAxesMin->value());
  settings.setValue("spinAxesMax", ui->spinAxesMax->value());
  settings.setValue("TextEditHide", ui->pushButton_TextEditHide->isChecked());
  settings.setValue("ShowallData", ui->pushButton_ShowallData->isChecked());
  settings.setValue("Record_stream", ui->actionRecord_stream->isChecked());
  settings.setValue("ChannelOnLCD", m_prefs.LCD_Channel);
  settings.setValue("ChannelOnLCDVisible", m_prefs.ChannelOnLCDVisible);
  settings.setValue("SimpleExpert", ui->SimpleExpert->isChecked());

  settings.beginWriteArray("channelnames");
  int max_size =
      ui->plot->graph(9) ? m_prefs.channelnames.size() : ui->plot->graphCount();

  for (int i = 0; i < max_size; ++i) {
    settings.setArrayIndex(i);
    settings.setValue("channelName", ui->plot->graph(i)->name());
    settings.setValue("channelVisibie", ui->plot->graph(i)->visible());
  }
  settings.endArray();
}
/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/**
 * @brief t the close event, save settings
 *
 */
void MainWindow::closeEvent(QCloseEvent *event) {
  //  saveSettings();

  on_actionDisconnect_triggered();
  event->accept();
}

void MainWindow::on_pushButton_2_clicked() // Profile Settings Button
{
  // hide();                                       // hide main window
  ui->textSend_UartWindow->clear();
  ui->textSend_UartWindow->insertPlainText("X"); // Send Silence connamd
  ui->pushButton_SendToCom->click();

  //profilread();
 // dialog->buttonPressed();
  dialog->show();
 // dialog->exec(); // block main window reaction
}


void MainWindow::profilread() {
  QString data = ui->textSend_UartWindow->toPlainText();
  ui->textSend_UartWindow->clear();
  ui->textSend_UartWindow->insertPlainText("P");
  ui->pushButton_SendToCom->click();
  ui->textSend_UartWindow->insertPlainText(data);
  //alflag = 1;
  receivedWord.clear();
  data.clear();
}

void MainWindow::send_data_to_com(QString newdata) {
  ui->textSend_UartWindow->clear();
  ui->textSend_UartWindow->append(newdata);
  ui->pushButton_SendToCom->click();
  ui->textEdit_UartWindow->append(newdata);

  newdata.clear();
}

void MainWindow::unblock(QString epromdata) {
  ui->textSend_UartWindow->clear();
  ui->textSend_UartWindow->append(epromdata);
  ui->textEdit_UartWindow->append(epromdata);
  ui->pushButton_SendToCom->click();
  epromdata.clear();
  ui->textSend_UartWindow->clear();
  ui->textSend_UartWindow->insertPlainText("W");
  ui->pushButton_SendToCom->click();
}

void MainWindow::unblock2() {
  ui->textSend_UartWindow->clear();
  ui->textSend_UartWindow->insertPlainText("W");
  ui->pushButton_SendToCom->click();
}

void MainWindow::on_HotStart_clicked() {
  ui->HotStart->setEnabled(false);
  ui->HotStart->setStyleSheet("QPushButton { background-color: grey; }\n");

  ui->pushButton_OK->setEnabled(false);
  ui->pushButton_OK->setStyleSheet("QPushButton { background-color: grey; }\n");

 // ui->pushButton_CANCEL->setEnabled(true);
  ui->pushButton_CANCEL->setStyleSheet(
      "QPushButton:enabled { background-color: rgb(250,0,0); }\n"
      "QPushButton:enabled { color: rgb(0,0,0); }\n");

  ui->pushButton_LEFT->setStyleSheet(
      "QPushButton { background-color: grey; }\n");
  ui->pushButton_RIGHT->setStyleSheet(
      "QPushButton { background-color: grey; }\n");

  ui->pushButton_RIGHT->setEnabled(false);
  ui->pushButton_LEFT->setEnabled(false);

  ui->manulabutton->setVisible(false);
  // ui->spinmanual->setVisible(false);

  QString data = ui->textSend_UartWindow->toPlainText();
  ui->textSend_UartWindow->clear();
  ui->textSend_UartWindow->insertPlainText("J");
  ui->pushButton_SendToCom->click();
  ui->textSend_UartWindow->insertPlainText(data);
  data.clear();
}

void MainWindow::on_manulabutton_clicked() {

  ui->pushButton_CANCEL->setStyleSheet(
      "QPushButton:enabled { background-color: rgb(250,0,0); }\n"
      "QPushButton:enabled { color: rgb(0,0,0); }\n");

  ui->pushButton_OK->setStyleSheet("QPushButton { background-color: grey; }\n");

  ui->pushButton_LEFT->setStyleSheet(
      "QPushButton { background-color: grey; }\n");
  ui->pushButton_RIGHT->setStyleSheet(
      "QPushButton { background-color: grey; }\n");

  ui->pushButton_RIGHT->setEnabled(false);
  ui->pushButton_LEFT->setEnabled(false);
  ui->pushButton_OK->setEnabled(false);
  //ui->pushButton_CANCEL->setEnabled(true);

  ui->pushButton_UP->setEnabled(true);
  ui->pushButton_UP->setStyleSheet(
      "QPushButton:enabled { background-color: rgb(50,150,250); }\n"
      "QPushButton:enabled { color: rgb(0,0,0); }\n");
  ui->pushButton_DOWN->setEnabled(true);
  ui->pushButton_DOWN->setStyleSheet(
      "QPushButton:enabled { background-color: rgb(50,150,250); }\n"
      "QPushButton:enabled { color: rgb(0,0,0); }\n");

  ui->pushButton_2->setEnabled(false);
  ui->pushButton_2->setStyleSheet("QPushButton { background-color: grey; }\n");

  ui->HotStart->setEnabled(false);
  ui->HotStart->setStyleSheet("QPushButton { background-color: grey; }\n");

  ui->manulabutton->setStyleSheet("QPushButton { background-color: grey; }\n");
  ui->manulabutton->setEnabled(false);

  QString data = ui->textSend_UartWindow->toPlainText();
  ui->textSend_UartWindow->clear();
  ui->textSend_UartWindow->insertPlainText("M 3,100"); // Уставка 100 градусов плате
  ui->pushButton_SendToCom->click();
  ui->textSend_UartWindow->insertPlainText(data);
  data.clear();
}



void MainWindow::plot_Profile(QString timeStamps, QString temperatures, int name) {
    QStringList timeStampList = timeStamps.split(",", Qt::SkipEmptyParts);
    QStringList temperatureList = temperatures.split(",", Qt::SkipEmptyParts);
    QVector<double> timeStampsVector, temperaturesVector;  // Помещаем все данные в вектор для отрисовки графика
    int min_size = 0;

    if (timeStampList.size() != 0){
        if (timeStampList.size() > temperatureList.size()){
          min_size = temperatureList.size();} else { min_size = timeStampList.size();}

    // Convert the string arrays to vectors of doubles
        for (int i = 0; i < min_size; i++) {
            double timeStamp = timeStampList[i].toDouble();
            double temperature = temperatureList[i].toDouble();
            timeStampsVector.push_back(timeStamp);
            temperaturesVector.push_back(temperature);
        }
               ui->plot->graph(name)->setSelectable(QCP::SelectionType(QCP::stSingleData));
               //ui->plot->graph(name)->setSelectable(QCP::SelectionType(QCP::stDataRange));
               ui->plot->graph(name)->setData(timeStampsVector, temperaturesVector);
               ui->plot->graph(name)->setPen(line_colors[name % CUSTOM_LINE_COLORS]);
               ui->plot->graph(name)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 6));
               ui->plot->replot();
    }
}

//void MainWindow::plottableClicked(QCPAbstractPlottable *plottable, int dataIndex, QMouseEvent *event)
//{

//}

void MainWindow::plot_All_Profile(){

    clear_profile_graph();
    plot_Profile(profiledata.values["time_step_top"], profiledata.values["temperature_step_top"], 9);
    plot_Profile(profiledata.values["time_step_bottom"], profiledata.values["temperature_step_bottom"], 10);
    plot_Profile(profiledata.values["time_step_pcb"], profiledata.values["temperature_step_pcb"], 11);
    profiledata.values.clear();
};

void MainWindow::clear_profile_graph(){  // clear profile graph
    for (int i=9; i<11; i++){
    ui->plot->graph(i)->data()->clear();
    }

}

void MainWindow::clear_plottables_graph(){ // clear temeratures graph recieved from station
    for (int i=0; i<9; i++){
    ui->plot->graph(i)->data()->clear();
    }
    dataPointNumber = 0;
}

void MainWindow::handleMousePress(QMouseEvent* event) { // one time left click
    // Check if the left mouse button was pressed draw point stats
    if (profileEditEnable){
    if (event->button() == Qt::RightButton) {
      // Get the plot coordinates of the mouse click
       double xx = ui->plot->xAxis->pixelToCoord(event->pos().x());
       double yy = ui->plot->yAxis->pixelToCoord(event->pos().y());
       int dataIndex = selectedDataIndex;
        // If a data point was found, update its value
        if (dataIndex >= 0) {
           // selectedPlottable->interface1D()->dataMainKey(dataIndex) = x;
            QCPGraph *selected = ui->plot->selectedGraphs().first();
            (selected->data()->begin()+dataIndex)->value = yy;
            ui->plot->setCursor(Qt::ClosedHandCursor);
            ui->plot->replot();
        }
       // QString message1 = QString(" button move to: %1 temp: %2°  ").arg(xx).arg(yy);
       //  ui->statusBar->showMessage(message1);
    }
    }
    //else {ui->plot->deselectAll();}
}


void MainWindow::plottableDoubleClicked(QCPAbstractPlottable *plottable, int dataIndex, QMouseEvent *event){
    // ui->statusBar->showMessage("Click2!");
    double dataValue = plottable->interface1D()->dataMainValue(dataIndex);
    double datakey = plottable->interface1D()->dataMainKey(dataIndex);
   // QString name = plottable->name();
    double x0 = datakey; // time
    double y0 = dataValue; // temp

    selectedPlottable = plottable; // Store for Future use
    selectedDataIndex = dataIndex; // Plottable Selected point, Store for Future use

    //calculate show time,temp, dt, dT, dT\dt
    if (dataIndex >= 0){

        double xp = plottable->interface1D()->dataMainKey(dataIndex-1); // time
        double yp = plottable->interface1D()->dataMainValue(dataIndex-1); // temp

      //  double yn = plottable->interface1D()->dataMainValue(dataIndex+1);
      //  double xn = plottable->interface1D()->dataMainKey(dataIndex+1);
        QString message1 = QString("Time: %1 temp: %2°  ").arg(x0).arg(y0);
        QString message2 = QString("dT: %1sec dt: %2°  ").arg(x0-xp).arg(y0-yp);
        QString message3 = QString("dT/dt: %1°/sec.").arg((y0-yp)/(x0-xp));

       // DataViewDialog* dlg = new DataViewDialog();
        //QString metadata = dm.getMetadata(name,dataIndex);
        //QString metadata("1425322307:r:46.06:g:11.53:y:32.09");
        //QString metadata("A: 110.,138.6,164.8 ");
        //dlg->SetData(name,metadata,x0, y0, xp, yp, xn, yn); +y0+xp+yp+xn+yn
       // dlg->show();
     ui->statusBar->showMessage(message1+message2+message3);
    }
};


/**
 * @brief Prints coordinates of mouse pointer in status bar on mouse release
 * @param event
 */
void MainWindow::onMouseMoveInPlot(QMouseEvent *event) {

    int xx = int(ui->plot->xAxis->pixelToCoord(event->x()));
    int yy = int(ui->plot->yAxis->pixelToCoord(event->y()));
  if(profileEditEnable){
    if (event->buttons() & Qt::RightButton){
     // ui->statusBar->showMessage("right");
      int dataIndex = selectedDataIndex;
      if (dataIndex >= 0) {


         QCPGraph *selected = ui->plot->selectedGraphs().first(); //find what plottable have selected point
          (selected->data()->begin()+dataIndex)->value = yy; // assign Temperature value to selected point
         double x0 = selected->interface1D()->dataMainKey(dataIndex); // current time at point, not cursor position
         double xp = selected->interface1D()->dataMainKey(dataIndex-1); // time
         double yp = selected->interface1D()->dataMainValue(dataIndex-1); // temp
         QString message1 = QString("Time: %1 temp: %2°  ").arg(x0).arg(yy);
         QString message2 = QString("dT: %1sec dt: %2°  ").arg(x0-xp).arg(yy-yp);
         QString message3 = QString("dT/dt: %1°/sec.").arg((yy-yp)/(x0-xp));
          ui->plot->replot();
          ui->statusBar->showMessage(message1+message2+message3);
      }
    }
  }
  else{
      QString coordinates("X: %1 Y: %2");
      coordinates = coordinates.arg(xx).arg(yy);
      ui->statusBar->showMessage(coordinates);
  }
}
/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

    void MainWindow::handleMouseRelease(QMouseEvent* event) {
        if (profileEditEnable){// Check if the right mouse button was released
        if (event->button() == Qt::RightButton) {
            // Deselect all points and reset the cursor shape
            //ui->plot->setMouseTracking(false);
            ui->plot->setCursor(Qt::ArrowCursor);
            ui->statusBar->showMessage("Released");
            // Export plot data to plot_Profile function
            exportPlotToProfileData(); // Claim this function at the and of editing.

            selectedDataIndex = -1;
            selectedPlottable = nullptr;
            ui->plot->deselectAll();

            //plot_Profile(timeStamps, temperatures, selectedPlottable->name());
        }
        }

    }

    void MainWindow::exportPlotToProfileData() { // export data to settings window
        if (profileEditEnable){
        QMap<QString, QVector<QPair<double, double>>> graphData;
        foreach (QCPGraph *graph, ui->plot->selectedGraphs()) {
            QVector<QPair<double, double>> dataPoints;
            QString timeStamps, temperatureStamps;
            QString graphName = graph->name();
            QString temperatureText = QString("temperature_step_%1: ").arg(graphName);
            QString timeText = QString("time_step_%1: ").arg(graphName);

            for (int i = 0; i < graph->data()->size(); i++) {
                double timestamp = graph->data()->at(i)->key;
                double temperature = graph->data()->at(i)->value;
                timeStamps.append(QString::number(timestamp)+",");
                temperatureStamps.append(QString::number(temperature)+",");
                //dataPoints.append(QPair<double, double>(timestamp, temperature));
            }
            //graphData.insert(graphName, dataPoints);
            emit sendData(temperatureText+temperatureStamps);
            emit sendData(timeText+timeStamps);
           // profiledata.addValue("Plot Data", QVariant::fromValue(graphData).toString());
           // ui->statusBar->showMessage(profiledata.values[graphName]);
           //
        }
        }
    }
    void MainWindow::switch_EDIT_MODE(bool mode){
        profileEditEnable = mode;
        ui->plot->setInteraction (QCP::iSelectPlottables, mode);
    }
