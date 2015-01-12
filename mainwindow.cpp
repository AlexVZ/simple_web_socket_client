
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->mainToolBar->setVisible(false);

    ui->webSocketServerIP->setText(tr("127.000.000.001"));
    ui->webSocketServerPort->setText(tr("1111"));

    connect(ui->connectButton, SIGNAL(clicked(bool)), this, SLOT(connectButton_clicked(bool)));
    connect(ui->sendButton, SIGNAL(clicked(bool)), this, SLOT(sendButton_clicked(bool)));

    m_web_socket = new QWebSocket();
    connect(m_web_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(is_error(QAbstractSocket::SocketError)));
    connect(m_web_socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(state_changed(QAbstractSocket::SocketState)));
    connect(m_web_socket, SIGNAL(textMessageReceived(QString)), this, SLOT(processTextMessage(QString)));

    setAllEnabled(false);

    ui->statusBar->showMessage(tr("Check server IP and port and press Connect button."));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
    //ui->statusBar->showMessage(tr("e->key(): %1").arg(e->key())); // 1049 0x419; 1062 0x426
    if( ((e->key() == Qt::Key_X || e->key() == 0x427) && (e->modifiers() == Qt::AltModifier)) || // Alt+X and Alt+Ч
        ((e->key() == Qt::Key_Q || e->key() == 0x419) && (e->modifiers() == Qt::ControlModifier)) || // Ctrl+Q and Ctrl+Й
        ((e->key() == Qt::Key_W || e->key() == 0x426) && (e->modifiers() == Qt::ControlModifier)) ) { // Ctrl+W and Ctrl+Ц
        on_actionExit_triggered();
    }
}

void MainWindow::on_actionExit_triggered()
{
    close();
}

void MainWindow::setAllEnabled(bool enabled)
{
    ui->webSocketServerIP->setEnabled(!enabled);
    ui->webSocketServerPort->setEnabled(!enabled);
    ui->sendButton->setEnabled(enabled);
}

void MainWindow::connectButton_clicked(bool)
{
    setAllEnabled(false);
    ui->webSocketServerIP->setEnabled(false);
    ui->webSocketServerPort->setEnabled(false);
    ui->connectButton->setEnabled(false);

    if(m_connected) {
        ui->statusBar->showMessage(tr("Disconnecting..."));
        disconnect_game_server();
    } else {
        ui->statusBar->showMessage(tr("Connecting..."));
        connect_game_server();
    }
}

void MainWindow::sendButton_clicked(bool)
{
    QString message = ui->lineEdit->text();
    if(message.size() == 0) {
        ui->statusBar->showMessage(tr("Empty message string; message NOT sent..."));
        return;
    }
    ui->plainTextEdit->appendPlainText(tr("Outgoing message: ") + message);
    m_web_socket->sendTextMessage(message);
}

void MainWindow::connect_game_server()
{
    QString host = ui->webSocketServerIP->text();
    QString port = ui->webSocketServerPort->text();

    QString trimed_host = trim_host(host);
    QString conn_url = tr("ws://")+trimed_host+tr(":%1").arg(port);
    m_web_socket->open(QUrl(conn_url));
}

void MainWindow::disconnect_game_server()
{
    m_web_socket->close();
}

void MainWindow::is_error(QAbstractSocket::SocketError socketError)
{
    QString msg = tr("Unknown error: %1.").arg(socketError);
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        msg = tr("The remote host closed connection.");
        break;
    case QAbstractSocket::HostNotFoundError:
        msg = tr("The host was not found.");
        break;
    case QAbstractSocket::ConnectionRefusedError:
        msg = tr("The connection was refused by the peer.");
        break;
    default:
        msg = tr("The following error occurred: %1.").arg(m_web_socket->errorString());
    }
    ui->statusBar->showMessage(msg);

    setAllEnabled(false);
    ui->webSocketServerIP->setEnabled(true);
    ui->webSocketServerPort->setEnabled(true);
    ui->connectButton->setEnabled(true);
}

void MainWindow::state_changed(QAbstractSocket::SocketState state)
{
    m_connect_status = state;
    if(m_connect_status == 3) {
        m_connected = true;
        ui->connectButton->setEnabled(true);
        ui->connectButton->setText(tr("Disconnect"));
        ui->statusBar->showMessage(tr("Connected"));
        setAllEnabled(true);
    } else if(state == 0 || state == 6) {
        m_connected = false;
        ui->webSocketServerIP->setEnabled(true);
        ui->webSocketServerPort->setEnabled(true);
        ui->connectButton->setEnabled(true);
        ui->connectButton->setText(tr("Connect"));
        ui->statusBar->showMessage(tr("Disconnected"));
    }
}

void MainWindow::processTextMessage(QString message)
{
    ui->plainTextEdit->appendPlainText(tr("Incomming message: ") + message);
}

QString MainWindow::trim_host(QString src_host)
{
    QString trimmed = tr("");
    // 192.168.000.001 -> 192.168..1
    bool wait_zero = true;
    for(int i = 0; i < src_host.size(); i++) {
        if(wait_zero) {
            if(src_host[i] != '0') {
                trimmed += src_host[i];
                wait_zero = false;
            }
            if(src_host[i] == '.') {
                wait_zero = true;
            }
        } else {
            trimmed += src_host[i];
            if(src_host[i] == '.') {
                wait_zero = true;
            }
        }
    }
    // 192.168..1 -> 192.168.0.1
    src_host = trimmed;
    trimmed = tr("");
    for(int i = 0; i < src_host.size(); i++) {
        if(i > 0) {
            if(src_host[i] == '.' && src_host[i-1] == '.') {
                trimmed += "0";
            }
            trimmed += src_host[i];
        } else {
            trimmed += src_host[i];
        }
    }
    return trimmed;
}
