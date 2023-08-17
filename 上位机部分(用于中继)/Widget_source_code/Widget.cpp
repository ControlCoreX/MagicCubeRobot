#include "Widget.h"
#include "WSocketTool.h"
#include <QDebug>
#include <QMessageBox>
#include <QtSerialPort/QSerialPortInfo>
#include <QPushButton>


Widget::Widget(QWidget* parent) : QWidget(parent), ui(new Ui::WidgetClass)
{
    ui->setupUi(this);

    this->setWindowTitle("zyx-twb");//!< ���ô��ڱ���.

    ui->combo_baudrate->addItem("9600");
    ui->combo_baudrate->addItem("38400");
    ui->combo_baudrate->addItem("115200");
    ui->combo_baudrate->setCurrentIndex(2);//!< ����������Ĭ��ֵ

    serialPort = new QSerialPort(this);//����һ�����������

    QStringList serialPorts;
    foreach(const QSerialPortInfo & info, QSerialPortInfo::availablePorts())
    {
        serialPorts << info.portName();
    }
    ui->combo_port_num->addItems(serialPorts);

    this->port_state = false;

    ui->lineEdit_discr->setReadOnly(true);
    ui->lineEdit_sloving->setReadOnly(true);

    //! ���� �򿪴��� ��ť�� ��ʼ�����ڵĲۺ���. ʵ�ְ��� �򿪰�ť �Ĳۺ���֮��, �ͳ�ʼ�����ڵĹ���.
    connect(ui->pushButton_open_com, &QPushButton::clicked, this, &Widget::init_serial_port);

    connect(ui->pushButton_test_network, &QPushButton::clicked, [=]() {
    WSocketTool wSocketTool;
    string value = "bdrfuululululrddrubbflfbdbbfdrrdbdurlrudlffurfrdfblbfl";
    string url = "/?" + value;
    wSocketTool.sendGetData(url);
    string result = wSocketTool.receiptData();
    QMessageBox::information(this, "test network", QString::fromLocal8Bit(result.c_str()));
        });

    //! ����׼�����պͽ��ղۺ���
    connect(serialPort, &QSerialPort::readyRead, this, &Widget::receivedData);

    //! ���������ť��lineEdit.
    connect(ui->pushButton_clear_discr, &QPushButton::clicked, ui->lineEdit_discr, &QLineEdit::clear);
    connect(ui->pushButton_clear_sloving, &QPushButton::clicked, ui->lineEdit_sloving, &QLineEdit::clear);
}


//���ڳ�ʼ���ۺ���. �Ѵ��ڲ����Ͷ�Ϊ: 1λֹͣλ, 8λ����λ, ��У��.
void Widget::init_serial_port(void)
{
    this->port_state = !this->port_state;//!< ״̬��ת.

    if (this->port_state == true)
    {
        QSerialPort::BaudRate baud;

        //!< ���û�ѡ������.
        if (ui->combo_baudrate->currentText() == "9600")
            baud = QSerialPort::Baud9600;
        else if (ui->combo_baudrate->currentText() == "38400")
            baud = QSerialPort::Baud38400;
        else//(ui->combo_baudrate->currentText() == "115200")
            baud = QSerialPort::Baud115200;

        //!< ���д�������.
        serialPort->setPortName(ui->combo_port_num->currentText());
        serialPort->setBaudRate(baud);
        serialPort->setStopBits(QSerialPort::OneStop);
        serialPort->setDataBits(QSerialPort::Data8);
        serialPort->setParity(QSerialPort::NoParity);

        //�򿪴���
        if (serialPort->open(QIODevice::ReadWrite) == true)
        {
            //            QMessageBox::information(this,"��ʾ","���ڴ򿪳ɹ�");
            ui->pushButton_open_com->setText("CLOSE");
        }
        else//���ڴ�ʧ��
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


//! �Ӵ��ڽ��յ�����ʱ�ͻ��Զ�ִ�иú���.
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

            ui->lineEdit_discr->setText(serial_recv);//!< ���Ӵ��ڽ��յ���������ʾ��lineEdit��.

            //qDebug() << serial_recv;

            WSocketTool wSocketTool;
            string url = "/?" + serial_recv.toStdString();
            wSocketTool.sendGetData(url);
            string result = wSocketTool.receiptData();

            QString inet_recv = QString::fromLocal8Bit(result.c_str());

            ui->lineEdit_sloving->setText(inet_recv);//!< ����������յ���������ʾ��lineEdit��.

            serialPort->write(inet_recv.toLatin1().data());//!< ���ⷨͨ�����ڷ��ͳ�ȥ.

            serial_recv_data.clear();
            serial_recv_data_buf.clear();
        }
    }
}


Widget::~Widget()
{
    delete ui;
}
