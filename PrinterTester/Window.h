#pragma once
#include <QWidget>
#include <QString>
#include <QFile>
#include <QSerialPort>
#include <QSerialPortInfo>


namespace Ui
{
	class Window;
}


class Window : public QWidget
{
	Q_OBJECT

public:
	Window(QWidget* parent = nullptr);
	~Window();
public slots:
	void on_btnChooseFile_clicked();
	void on_cmbboxChooseCOM_activated(int num);
	void on_btnUpdateCOMPorts_clicked();
	void on_btnStartTest_clicked();
	void rtsStateChanged(bool rtsState);
	void handleReadyRead();

private:
	Ui::Window* m_ui = nullptr;
	QSerialPort* m_selectedPort = nullptr;

	QTimer* m_timeoutTimer = nullptr;
	const uint m_timeoutMS = 20000;//20 sec
	bool m_transmissionPaused;
	qint64 m_bytesWritten;
	uint m_currentTestNumber = 1;

	QByteArray m_file;
	QList<QSerialPortInfo> m_portList;
	

	void loadPicture(uint num);
	void setUpCOMPort(uint num);
	void setUpInfo(uint num);
	void startTransmittingDataRTS();
	void startTransmittingDataXonXoff();
	void handleTimeout();
};
