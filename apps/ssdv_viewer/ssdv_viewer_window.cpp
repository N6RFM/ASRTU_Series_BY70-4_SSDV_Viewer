#include "ssdv_viewer_window.h"

#include "pmt_frame_decoder.h"
#include "ssdv_image_window.h"
#include "version.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QMetaObject>
#include <QPushButton>
#include <QSettings>
#include <QStandardPaths>
#include <QVBoxLayout>

namespace
{
constexpr int kReconnectIntervalMs = 3000;
constexpr auto kSessionDirectorySettingsKey = "session_directory";

QString defaultSessionDirectory()
{
    QString base =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (base.isEmpty())
        base = QDir::home().filePath(QStringLiteral(".local/share/ASRTU"));
    return base;
}
} // namespace

SsdvViewerWindow::SsdvViewerWindow(QWidget* parent) : QWidget(parent)
{
    setWindowTitle(QCoreApplication::translate("ASRTU", "SSDV 图像查看器") +
                   QStringLiteral(" v") +
                   QStringLiteral(ASRTU_SSDV_VIEWER_VERSION));

    session_directory_ =
        QSettings().value(QLatin1String(kSessionDirectorySettingsKey))
            .toString();
    if (session_directory_.isEmpty())
        session_directory_ = defaultSessionDirectory();
    QDir().mkpath(session_directory_);

    auto* layout = new QVBoxLayout(this);

    auto* form = new QFormLayout;
    host_ = new QLineEdit(QStringLiteral("127.0.0.1"), this);
    port_ = new QLineEdit(QStringLiteral("9985"), this);
    port_->setValidator(new QIntValidator(1, 65535, port_));
    form->addRow(QCoreApplication::translate("ASRTU", "TCP 主机"), host_);
    form->addRow(QCoreApplication::translate("ASRTU", "TCP 端口"), port_);
    layout->addLayout(form);

    auto* buttonRow = new QHBoxLayout;
    connect_button_ =
        new QPushButton(QCoreApplication::translate("ASRTU", "连接"), this);
    buttonRow->addWidget(connect_button_);
    layout->addLayout(buttonRow);

    status_label_ = new QLabel(
        QCoreApplication::translate("ASRTU", "未连接"), this);
    layout->addWidget(status_label_);
    frame_count_label_ = new QLabel(
        QCoreApplication::translate("ASRTU", "已接收帧数：0"), this);
    layout->addWidget(frame_count_label_);

    auto* sessionRow = new QHBoxLayout;
    session_label_ = new QLabel(
        QCoreApplication::translate("ASRTU", "图像保存目录：%1")
            .arg(session_directory_),
        this);
    session_label_->setWordWrap(true);
    session_label_->setStyleSheet(QStringLiteral("color:#667788;"));
    browse_button_ =
        new QPushButton(QCoreApplication::translate("ASRTU", "浏览..."), this);
    sessionRow->addWidget(session_label_, 1);
    sessionRow->addWidget(browse_button_);
    layout->addLayout(sessionRow);

    auto* versionLabel = new QLabel(
        QStringLiteral("ASRTU_SSDV_Viewer v") +
            QStringLiteral(ASRTU_SSDV_VIEWER_VERSION),
        this);
    versionLabel->setStyleSheet(QStringLiteral("color:#99a3ad;"));
    versionLabel->setAlignment(Qt::AlignRight);
    layout->addWidget(versionLabel);

    setMinimumWidth(380);

    ssdv_window_ = new SsdvImageWindow(this);
    createSsdvReceiver();
    ssdv_window_->setClearCallback([this] {
        if (ssdv_receiver_)
            ssdv_receiver_->clear();
    });

    connect(browse_button_, &QPushButton::clicked, this,
            &SsdvViewerWindow::browseForDirectory);

    reconnect_timer_.setInterval(kReconnectIntervalMs);
    reconnect_timer_.setSingleShot(true);

    connect(connect_button_, &QPushButton::clicked, this, [this] {
        if (want_connected_) {
            want_connected_ = false;
            reconnect_timer_.stop();
            socket_.disconnectFromHost();
            connect_button_->setText(
                QCoreApplication::translate("ASRTU", "连接"));
            status_label_->setText(
                QCoreApplication::translate("ASRTU", "未连接"));
        } else {
            want_connected_ = true;
            connect_button_->setText(
                QCoreApplication::translate("ASRTU", "断开"));
            connectToHost();
        }
    });
    connect(&reconnect_timer_, &QTimer::timeout, this, [this] {
        if (want_connected_)
            connectToHost();
    });
    connect(&socket_, &QTcpSocket::readyRead, this,
            &SsdvViewerWindow::handleReadyRead);
    connect(&socket_, &QTcpSocket::connected, this, [this] {
        status_label_->setText(
            QCoreApplication::translate("ASRTU", "已连接：%1:%2")
                .arg(host_->text().trimmed(), port_->text().trimmed()));
    });
    connect(&socket_, &QTcpSocket::disconnected, this,
            &SsdvViewerWindow::handleDisconnected);
    connect(&socket_,
            QOverload<QAbstractSocket::SocketError>::of(
                &QTcpSocket::errorOccurred),
            this, [this](QAbstractSocket::SocketError) {
                handleSocketError();
            });
}

void SsdvViewerWindow::createSsdvReceiver()
{
    ssdv_receiver_ = std::make_unique<SsdvReceiver>(
        session_directory_,
        [this](const SsdvImageUpdate& update) {
            QMetaObject::invokeMethod(
                this,
                [this, update] {
                    if (!ssdv_window_)
                        return;
                    ssdv_window_->updateImage(update);
                    if (!ssdv_window_->isVisible())
                        ssdv_window_->show();
                },
                Qt::QueuedConnection);
        },
        [this](const QString& line) {
            QMetaObject::invokeMethod(
                this, [this, line] { appendLog(line); },
                Qt::QueuedConnection);
        });
}

void SsdvViewerWindow::browseForDirectory()
{
    const QString chosen = QFileDialog::getExistingDirectory(
        this, QCoreApplication::translate("ASRTU", "选择图像保存目录"),
        session_directory_);
    if (chosen.isEmpty() || chosen == session_directory_)
        return;

    QDir().mkpath(chosen);
    session_directory_ = chosen;
    session_label_->setText(
        QCoreApplication::translate("ASRTU", "图像保存目录：%1")
            .arg(session_directory_));
    QSettings().setValue(QLatin1String(kSessionDirectorySettingsKey),
                          session_directory_);

    // A new destination invalidates any image already in progress under
    // the old one -- start the receiver fresh rather than let a partially
    // built image get saved to the wrong place.
    createSsdvReceiver();
}

void SsdvViewerWindow::connectToHost()
{
    if (socket_.state() != QAbstractSocket::UnconnectedState)
        return;
    const QString host = host_->text().trimmed().isEmpty()
                              ? QStringLiteral("127.0.0.1")
                              : host_->text().trimmed();
    const int port = port_->text().trimmed().isEmpty()
                          ? 9985
                          : port_->text().trimmed().toInt();
    status_label_->setText(
        QCoreApplication::translate("ASRTU", "正在连接 %1:%2...").arg(host).arg(port));
    buffer_.clear();
    socket_.connectToHost(host, static_cast<quint16>(port));
}

void SsdvViewerWindow::handleReadyRead()
{
    buffer_.append(socket_.readAll());
    // The upstream server (e.g. GNU Radio's network_socket_pdu in
    // TCP_SERVER mode with no preamble/tailer attached) sends fixed-length
    // frames back-to-back with no delimiter, so just chunk in place.
    while (static_cast<std::size_t>(buffer_.size()) >=
           asrtu::kTelemetryFrameBytes) {
        const QByteArray frame =
            buffer_.left(static_cast<int>(asrtu::kTelemetryFrameBytes));
        buffer_.remove(0, static_cast<int>(asrtu::kTelemetryFrameBytes));
        ++frames_received_;
        if (ssdv_receiver_)
            ssdv_receiver_->ingestFrame(frame);
    }
    frame_count_label_->setText(
        QCoreApplication::translate("ASRTU", "已接收帧数：%1")
            .arg(frames_received_));
}

void SsdvViewerWindow::handleDisconnected()
{
    status_label_->setText(QCoreApplication::translate("ASRTU", "连接已断开"));
    if (want_connected_)
        reconnect_timer_.start();
}

void SsdvViewerWindow::handleSocketError()
{
    status_label_->setText(
        QCoreApplication::translate("ASRTU", "连接错误：%1")
            .arg(socket_.errorString()));
    if (want_connected_ &&
        socket_.state() == QAbstractSocket::UnconnectedState)
        reconnect_timer_.start();
}

void SsdvViewerWindow::appendLog(const QString& line)
{
    status_label_->setText(line);
}