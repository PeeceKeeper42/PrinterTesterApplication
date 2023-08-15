#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QString>
#include <QTimer>
#include <String>
#include <stdexcept>

#include "Window.h"
#include "ui_window.h"
#include <iostream>
#include <windows.h>
#include <QNetworkInterface>

QString static trimStringToFitLabel(QString prefix, std::string str);


Window::Window(QWidget* parent) : QWidget(parent)
{	
	m_ui = new Ui::Window();

	connect(m_ui->cmbboxChooseCOM, SIGNAL(activated(int)), this, SLOT(on_cmbboxChooseCOM_activated(int)));
	connect(m_ui->btnChooseFile, SIGNAL(clicked()), this, SLOT(on_btnChooseFile_clicked()));
	connect(m_ui->btnUpdateCOMPorts, SIGNAL(clicked()), this, SLOT(on_btnUpdateCOMPorts_clicked()));
	connect(m_ui->btnStartTest, SIGNAL(clicked()), this, SLOT(on_btnStartTest_clicked()));
	connect(m_timeoutTimer, &QTimer::timeout, this, &Window::handleTimeout);
	connect(m_selectedPort, &QSerialPort::readyRead, this, &Window::handleReadyRead);


	m_ui->setupUi(this);

	m_transmissionPaused = false;
	m_timeoutTimer = new QTimer(this);
	m_timeoutTimer->setInterval(m_timeoutMS);
	m_bytesWritten = 0;

	loadPicture(1);
	on_btnUpdateCOMPorts_clicked();

}

Window::~Window()
{
	if (m_selectedPort != nullptr)
		delete m_selectedPort;
	delete m_ui;
}

void Window:: on_btnChooseFile_clicked()
{
	try 
	{
		QString filePath = QFileDialog::getOpenFileName(
			0,
			QStringLiteral("������� ����"),
			QStringLiteral("�:\\"),
			QStringLiteral("*.png *.jpg *.bmp *.jpeg *.doc *.docx *.pdf"));

		QFile file(filePath);
		
		m_ui->labelFilename->setText(trimStringToFitLabel(QStringLiteral("��� �����:"), QFileInfo(file.fileName()).fileName().toStdString()));
		if (file.open(QIODevice::ReadOnly))
		{
			m_file.clear();
			m_file = file.readAll();
			file.close();
		}
	}
	catch (const std::exception& ex)
	{

		QMessageBox::information(
			this,
			QStringLiteral("������"),
			QStringLiteral("��� ������ ����� ��� ������ ��������� ������.").arg(ex.what()),
			QMessageBox::Ok);

		qDebug() << QStringLiteral("��� ������ ����� ��� ������ ��������� ������.") << ex.what();
		m_ui->labelFilename->setText(QStringLiteral("��� �����:"));
		m_file.clear();
	}
}

void Window::on_cmbboxChooseCOM_activated(int index)
{
	try
	{
	delete m_selectedPort;
	m_selectedPort = new QSerialPort(m_portList[index]);

	m_ui->labelCOMportName->setText(QStringLiteral("�������� COM-�����:\n") + m_selectedPort->portName());
	}
	catch (std::exception& ex) {
		
		QMessageBox::information(
			this,
			QStringLiteral("������"),
			QStringLiteral("��� ������ COM-����� ��������� ������.").arg(ex.what()),
			QMessageBox::Ok);

		qDebug() << QStringLiteral("��� ������ COM-����� ��������� ������.") << ex.what();
		m_ui->labelCOMportName->setText(QStringLiteral("�������� COM-�����:\n"));

		if (m_selectedPort != nullptr)
		{
			delete m_selectedPort;
			m_selectedPort = nullptr;
		}
			
	}
}

void Window::on_btnUpdateCOMPorts_clicked()
{
	try 
	{
		m_portList =  QSerialPortInfo::availablePorts();
		m_ui->cmbboxChooseCOM->setEnabled(true);
		m_ui->cmbboxChooseCOM->clear();

		if (!m_portList.isEmpty()) 
		{
			for (const QSerialPortInfo& portInfo : m_portList) {
				m_ui->cmbboxChooseCOM->addItem(portInfo.portName());
			}
		}
		else 
		{
			m_ui->cmbboxChooseCOM->addItem(QStringLiteral("��� ��������� COM-������."));
			m_ui->cmbboxChooseCOM->setCurrentIndex(0);
			m_ui->cmbboxChooseCOM->setEnabled(false);

			if (m_selectedPort != nullptr)
			{
				delete m_selectedPort;
				m_selectedPort = nullptr;
			}
		}
	}
	catch (const std::exception& ex)
	{
		QMessageBox::information(
			this,
			QStringLiteral("������"),
			QStringLiteral("��� ���������� ������ COM-������ ��������� ������.").arg(ex.what()),
			QMessageBox::Ok);

		qDebug() << QStringLiteral("��� ���������� ������ COM-������ ��������� ������.") << ex.what();
		m_ui->labelCOMportName->setText(QStringLiteral("�������� COM-�����:\n"));

		m_portList.clear();
		m_ui->cmbboxChooseCOM->clear();
		m_ui->cmbboxChooseCOM->setEnabled(false);
	}
}

void Window::on_btnStartTest_clicked() {
	try 
	{
	if (m_file.isEmpty() || m_file.isNull() || m_selectedPort == nullptr)
		throw std::invalid_argument("Error while testing: invalid arguments");
	m_ui->btnChooseFile->setEnabled(false);
	m_ui->btnUpdateCOMPorts->setEnabled(false);
	m_ui->cmbboxChooseCOM->setEnabled(false);
	m_ui->btnStartTest->setEnabled(false);
	
	setUpInfo(m_currentTestNumber);
	loadPicture(m_currentTestNumber);
	setUpCOMPort(m_currentTestNumber);
	if (m_currentTestNumber != 3)
	startTransmittingDataRTS();
	else
	startTransmittingDataXonXoff();

	Sleep(10);

	if (m_currentTestNumber == 3)
	{
		QMessageBox::information(
			this,
			QStringLiteral("���������� �����"),
			QStringLiteral("���� ������� ��� ��������. ��������� ����� �������."),
			QMessageBox::Ok);
		this->close();//window closing;
	}
	m_currentTestNumber++;
	m_ui->btnStartTest->setEnabled(true);
	}
	catch (const std::exception& ex)
	{
		QMessageBox::information(
			this,
			QStringLiteral("������"),
			QStringLiteral("��� ���������� ������������ ��������� ������.\n��������� �������� ������.").arg(ex.what()),
			QMessageBox::Ok);

		qDebug() << QStringLiteral("��� ���������� ������������ ��������� ������.") << ex.what();
		this->close();//window closing;
	}
}

void Window::loadPicture(uint num)
{
	QString imagePath = QString::fromStdString('a' + std::to_string(num) + ".jpg");
	try
	{		
		QPixmap pixmap(imagePath);

		if (!pixmap.isNull()) {
			pixmap = pixmap.scaled(m_ui->labelImageHolder->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
			m_ui->labelImageHolder->setPixmap(pixmap);
		}
		else {
			throw std::invalid_argument("Can't open file.");
			
		}
	}
	catch (const std::exception& ex)
	{
		QMessageBox::information(
			this,
			QStringLiteral("������"),
			QStringLiteral("��� �������� �����������-���������� ��������� ������."),
			QMessageBox::Ok);

		qDebug() << QStringLiteral("��� �������� ����������� ���������� ��������� ������.") << ex.what();
		m_ui->labelImageHolder->setText("������ �������� �����������-���������� " + imagePath);
	}
}

void Window::setUpCOMPort(uint num)
{
	switch (num) {
	case 1:
	{
		m_selectedPort->setBaudRate(QSerialPort::Baud9600);
		m_selectedPort->setDataBits(QSerialPort::Data8);
		m_selectedPort->setParity(QSerialPort::NoParity);
		m_selectedPort->setStopBits(QSerialPort::OneStop);
		m_selectedPort->setFlowControl(QSerialPort::HardwareControl);
		break;
	}
	case 2: 
	{
		m_selectedPort->setBaudRate(QSerialPort::Baud4800);
		m_selectedPort->setDataBits(QSerialPort::Data8);
		m_selectedPort->setParity(QSerialPort::NoParity);
		m_selectedPort->setStopBits(QSerialPort::OneStop);
		m_selectedPort->setFlowControl(QSerialPort::HardwareControl);
		break;
	}
	case 3:
	{
		m_selectedPort->setBaudRate(QSerialPort::Baud9600);
		m_selectedPort->setDataBits(QSerialPort::Data8);
		m_selectedPort->setParity(QSerialPort::NoParity);
		m_selectedPort->setStopBits(QSerialPort::OneStop);
		m_selectedPort->setFlowControl(QSerialPort::SoftwareControl);
		break;
	}
	default:
		throw std::invalid_argument("Wrong setting argument");
	}
	
}

void Window::setUpInfo(uint num) {
	switch (num)
	{
	case 0:
		m_ui->labelInfo->setText(QStringLiteral("���������� � COM-�����:\n�������� ������:\n���������� ���:\n�������� ��������:\n���� ���:\n��� ���������� �������:\n����� �����: 0 �� 3"));
		break;
	case 1:
		m_ui->labelInfo->setText(QStringLiteral("���������� � COM-�����:\n�������� ������: 9600\n���������� ���: 8\n�������� ��������: ���\n���� ���: 1\n��� ���������� �������: RTS\n����� �����: 1 �� 3"));
		break;
	case 2:
		m_ui->labelInfo->setText(QStringLiteral("���������� � COM-�����:\n�������� ������: 4800\n���������� ���: 8\n�������� ��������: ���\n���� ���: 1\n��� ���������� �������: RTS\n����� �����: 2 �� 3"));
		break;
	case 3:
		m_ui->labelInfo->setText(QStringLiteral("���������� � COM-�����:\n�������� ������: 9600\n���������� ���: 8\n�������� ��������: ���\n���� ���: 1\n��� ���������� �������: Xon/Xoff\n����� �����: 3 �� 3"));
		break;
	default:
		throw std::invalid_argument("Wrong setting argument");
	}
}

void Window::startTransmittingDataRTS()
{
	if (!m_selectedPort->isOpen())
		if (!m_selectedPort->open(QIODevice::WriteOnly))
			throw std::invalid_argument("�� ������� ������� COM ����.");

	qint64 bytesToWrite = m_file.size() - m_bytesWritten;
	qint64 bytesSent = m_selectedPort -> write(m_file.constData() + m_bytesWritten, bytesToWrite);
	if (bytesSent == -1)
	{
		throw std::invalid_argument("������ ��� �������� ������.");
	}
	else
	{
		m_bytesWritten += bytesSent; 

		if (m_bytesWritten == m_file.size())
		{
			m_selectedPort->close();
		}
		else
		{
			m_transmissionPaused = true; 
			m_timeoutTimer->start();  
		}
	}
}

void Window::startTransmittingDataXonXoff() 
{
	if (!m_selectedPort->isOpen())
		if (!m_selectedPort->open(QIODevice::WriteOnly))
			throw std::invalid_argument("�� ������� ������� COM ����.");

	qint64 bytesToWrite = m_file.size() - m_bytesWritten;
	qint64 bytesSent = m_selectedPort -> write(m_file.constData() + m_bytesWritten, bytesToWrite);
	if (bytesSent == -1)
	{
		throw std::invalid_argument("������ ��� �������� ������.");
	}
	else
	{
		m_bytesWritten += bytesSent;

		if (m_bytesWritten == m_file.size())
		{
			m_selectedPort->close();
		}
		else
		{
			m_transmissionPaused = true; 
			m_timeoutTimer->start();  

			if (m_selectedPort -> bytesToWrite() <= 1024)
			{
				m_selectedPort -> write(QByteArrayLiteral("\x13"));
				m_selectedPort -> waitForBytesWritten(100);
			}
		}
	}
}

void Window::rtsStateChanged(bool rtsState)
{
	if (rtsState)
	{
		if (m_transmissionPaused)
		{
			startTransmittingDataRTS();
			m_transmissionPaused = false;
			m_timeoutTimer->stop();  
		}
	}
	else
	{
		if (!m_transmissionPaused)
		{
			m_transmissionPaused = true;
			m_timeoutTimer->start();  
		}
	}
}

void Window::handleReadyRead()
{
	QByteArray receivedData = m_selectedPort -> readAll();

	if (m_selectedPort -> bytesToWrite() > 1024)
	{
		m_selectedPort -> write(QByteArrayLiteral("\x11"));
		m_selectedPort -> waitForBytesWritten(100);
	}
}

void Window::handleTimeout()
{
	if(m_transmissionPaused)
	{
		m_timeoutTimer->stop();  
		QMessageBox::information(
			this,
			QStringLiteral("������"),
			QStringLiteral("����� �������� �������������� ������ ���� ���������.\n��������� �������� ������."),
			QMessageBox::Ok);
		this->close();//window closing;
	}
}

QString static trimStringToFitLabel(QString prefix, std::string str)
{	
	 return str.substr(0, 23) == str ?
		 prefix.append(("\n" + str).c_str()) :
		 prefix.append(("\n" + str.substr(0, 23) + "...").c_str());
}




