#pragma once

#include <QtWidgets/QWidget>
#include <QtSerialPort/QSerialPort>
#include "ui_Widget.h"

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget* parent = nullptr);
    ~Widget();


    QSerialPort* serialPort;



private:
    Ui::WidgetClass* ui;
    bool port_state;



public slots:
    void init_serial_port(void);

    void receivedData(void);

};
