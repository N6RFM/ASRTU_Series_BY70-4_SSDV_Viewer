#pragma once

#include "ssdv_receiver.h"

#include <QByteArray>
#include <QTcpSocket>
#include <QTimer>
#include <QWidget>

#include <memory>

class QLabel;
class QLineEdit;
class QPushButton;
class SsdvImageWindow;

class SsdvViewerWindow final : public QWidget
{
public:
    explicit SsdvViewerWindow(QWidget* parent = nullptr);

private:
    void connectToHost();
    void handleReadyRead();
    void handleDisconnected();
    void handleSocketError();
    void appendLog(const QString& line);
    void browseForDirectory();
    void createSsdvReceiver();

    QLineEdit* host_ = nullptr;
    QLineEdit* port_ = nullptr;
    QPushButton* connect_button_ = nullptr;
    QPushButton* browse_button_ = nullptr;
    QLabel* status_label_ = nullptr;
    QLabel* frame_count_label_ = nullptr;
    QLabel* session_label_ = nullptr;

    QTcpSocket socket_;
    QByteArray buffer_;
    QTimer reconnect_timer_;
    bool want_connected_ = false;
    quint64 frames_received_ = 0;

    SsdvImageWindow* ssdv_window_ = nullptr;
    std::unique_ptr<SsdvReceiver> ssdv_receiver_;
    QString session_directory_;
};