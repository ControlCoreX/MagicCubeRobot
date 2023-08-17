#include "Widget.h"
#include "WSocketTool.h"
#include <QDebug>
#include <QMessageBox>
#include <QtSerialPort/QSerialPortInfo>
#include <QPushButton>


Widget::Widget(QWidget* parent) : QWidget(parent), ui(new Ui::WidgetClass)
{
    ui->setupUi(this);

    this->setWindowTitle("zyx-twb");//!< 设置窗口标题.

    ui->combo_baudrate->addItem("9600");
    ui->combo_baudrate->addItem("38400");
    ui->combo_baudrate->addItem("115200");
    ui->combo_baudrate->setCurrentIndex(2);//!< 设置下拉框默认值

    serialPort = new QSerialPort(this);//创建一个串口类对象

    QStringList serialPorts;
    foreach(const QSerialPortInfo & info, QSerialPortInfo::availablePorts())
    {
        serialPorts << info.portName();
    }
    ui->combo_port_num->addItems(serialPorts);

    this->port_state = false;

    ui->lineEdit_discr->setReadOnly(true);
    ui->lineEdit_sloving->setReadOnly(true);

    //! 连接 打开串口 按钮和 初始化串口的槽函数. 实现按下 打开按钮 的槽函数之后, 就初始化串口的功能.
    connect(ui->pushButton_open_com, &QPushButton::clicked, this, &Widget::init_serial_port);

    connect(ui->pushButton_test_network, &QPushButton::clicked, [=]() {
    WSocketTool wSocketTool;
    string value = "bdrfuululululrddrubbflfbdbbfdrrdbdurlrudlffurfrdfblbfl";
    string url = "/?" + value;
    wSocketTool.sendGetData(url);
    string result = wSocketTool.receiptData();
    QMessageBox::information(this, "test network", QString::fromLocal8Bit(result.c_str()));
        });

    //! 连接准备接收和接收槽函数
    connect(serialPort, &QSerialPort::readyRead, this, &Widget::receivedData);

    //! 连接清除按钮和lineEdit.
    connect(ui->pushButton_clear_discr, &QPushButton::clicked, ui->lineEdit_discr, &QLineEdit::clear);
    connect(ui->pushButton_clear_sloving, &QPushButton::clicked, ui->lineEdit_sloving, &QLineEdit::clear);
}


//串口初始化槽函数. 把串口参数就定为: 1位停止位, 8位数据位, 无校验.
void Widget::init_serial_port(void)
{
    this->port_state = !this->port_state;//!< 状态翻转.

    if (this->port_state == true)
    {
        QSerialPort::BaudRate baud;

        //!< 让用户选择波特率.
        if (ui->combo_baudrate->currentText() == "9600")
            baud = QSerialPort::Baud9600;
        else if (ui->combo_baudrate->currentText() == "38400")
            baud = QSerialPort::Baud38400;
        else//(ui->combo_baudrate->currentText() == "115200")
            baud = QSerialPort::Baud115200;

        //!< 进行串口配置.
        serialPort->setPortName(ui->combo_port_num->currentText());
        serialPort->setBaudRate(baud);
        serialPort->setStopBits(QSerialPort::OneStop);
        serialPort->setDataBits(QSerialPort::Data8);
        serialPort->setParity(QSerialPort::NoParity);

        //打开串口
        if (serialPort->open(QIODevice::ReadWrite) == true)
        {
            //            QMessageBox::information(this,"提示","串口打开成功");
            ui->pushButton_open_com->setText("CLOSE");
        }
        else//串口打开失败
        {
            QMessageBox::critical(this, "NOTE", "serial port open failed!");
        }
    }
    else
    {
        serialPort->close();
        ui->pushButton_open_com->setText("OPEN");
    }

}


//! 从串口接收到数据时就会自动执行该函数.
void Widget::receivedData(void)
{
    QByteArray serial_recv_data;
    static QByteArray serial_recv_data_buf;

    serial_recv_data = serialPort->readAll();
    if (serial_recv_data.length() < 54)
    {
        serial_recv_data_buf.append(serial_recv_data);
        if (serial_recv_data_buf.length() == 54)
        {
            QString serial_recv = QString(serial_recv_data_buf);

            ui->lineEdit_discr->setText(serial_recv);//!< 将从串口接收到的数据显示在lineEdit中.

            //qDebug() << serial_recv;

            WSocketTool wSocketTool;
            string url = "/?" + serial_recv.toStdString();
            wSocketTool.sendGetData(url);
            string result = wSocketTool.receiptData();

            QString inet_recv = QString::fromLocal8Bit(result.c_str());

            ui->lineEdit_sloving->setText(inet_recv);//!< 将从网络接收到的数据显示在lineEdit中.

            serialPort->write(inet_recv.toLatin1().data());//!< 将解法通过串口发送出去.

            serial_recv_data.clear();
            serial_recv_data_buf.clear();
        }
    }
}


Widget::~Widget()
{
    delete ui;
}
