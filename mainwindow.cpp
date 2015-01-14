/****************************************************************************
**
** Copyright (C) 2014 Alex
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_connected(false),
    m_dh_completed(false)
{
    ui->setupUi(this);

    ui->mainToolBar->setVisible(false);

    ui->webSocketServerIP->setText(tr("127.000.000.001"));
    ui->webSocketServerPort->setText(tr("1111"));

    connect(ui->connectButton, SIGNAL(clicked(bool)), this, SLOT(connectButton_clicked(bool)));
    connect(ui->sendButton, SIGNAL(clicked(bool)), this, SLOT(sendButton_clicked(bool)));
    connect(ui->dhStartButton, SIGNAL(clicked(bool)), this, SLOT(dhStartButton_clicked(bool)));

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
    ui->dhStartButton->setEnabled(enabled);
}

void MainWindow::connectButton_clicked(bool)
{
    setAllEnabled(false);

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
        ui->plainTextEdit->appendPlainText(tr("Empty message string; message NOT sent..."));
        return;
    }
    if(m_dh_completed == false) {
        ui->plainTextEdit->appendPlainText(tr("Key exchange not completed; message NOT sent..."));
        return;
    }

    QString encoded_message = tr("cmd=coded&msg=%1").arg(m_aes256_helper.encrypt(message, m_dh_helper.get_key()));
    if(encoded_message.size() > 0) {
        ui->plainTextEdit->appendPlainText(tr("Outgoing message: '%1'; encoded: '%2'").arg(message).arg(encoded_message));
        m_web_socket->sendTextMessage(encoded_message);
    } else {
        ui->plainTextEdit->appendPlainText(tr("Outgoing message ('%1') encode error! Message NOT sent!").arg(message));
    }
}

void MainWindow::dhStartButton_clicked(bool)
{
    QString message = tr("cmd=begin");
    ui->plainTextEdit->appendPlainText(tr("Outgoing message: '%1'").arg(message));
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
    ui->dhStartButton->setEnabled(false);
}

void MainWindow::state_changed(QAbstractSocket::SocketState state)
{
    m_connect_status = state;
    if(m_connect_status == 3) {
        m_connected = true;
        ui->connectButton->setEnabled(true);
        ui->dhStartButton->setEnabled(true);
        ui->connectButton->setText(tr("Disconnect"));
        ui->statusBar->showMessage(tr("Connected"));
        setAllEnabled(true);
    } else if(state == 0 || state == 6) {
        m_connected = false;
        ui->webSocketServerIP->setEnabled(true);
        ui->webSocketServerPort->setEnabled(true);
        ui->dhStartButton->setEnabled(false);
        ui->connectButton->setEnabled(true);
        ui->connectButton->setText(tr("Connect"));
        ui->statusBar->showMessage(tr("Disconnected"));
    }
}

void MainWindow::processTextMessage(QString incomming_message)
{
    ui->plainTextEdit->appendPlainText(tr("Incomming message: ") + incomming_message);
    QStringList args_res_data = incomming_message.split(tr("&"));
    QStringList pair_data;
    QMap<QString, QString> get_args;
    for(int i = 0; i < args_res_data.size(); i++) {
        pair_data = args_res_data[i].split(tr("="));
        if(pair_data.size() == 2) {
            get_args[pair_data[0]] = pair_data[1];
        }
    }

    QString outgoing_message = handle_request(get_args);

    if(outgoing_message.size() > 0) {
        ui->plainTextEdit->appendPlainText(tr("Outgoing message: %1").arg(outgoing_message));
        m_web_socket->sendTextMessage(outgoing_message);
    }
}
QString MainWindow::handle_request(QMap<QString, QString>  &get_args)
{
    if(get_args.size() == 0) {
        return tr("");
    }
    if(get_args[tr("result")] != tr("ok")) {
        return tr("");
    }
    if(get_args[tr("cmd")] == tr("begin")) {
        QString dh_params = m_dh_helper.startB(get_args[tr("p")], get_args[tr("g")], get_args[tr("pub_key")]);
        return tr("cmd=key&%1").arg(dh_params);
    } else if(get_args[tr("cmd")] == tr("key")) {
        QString msg_decrypted;
        if(m_aes256_helper.decrypt(get_args[tr("check_msg")], msg_decrypted, m_dh_helper.get_key())) { // check MITM
            QString secret_string = m_dh_helper.get_secret_string();
            if(secret_string.size() == 0) {
                return tr("");
            }
            if(msg_decrypted == secret_string) {
                m_dh_completed = true;
                return tr("cmd=coded&msg=%1").arg(m_aes256_helper.encrypt(secret_string, m_dh_helper.get_key()));
            }
        }
        return tr("");
    } else if(get_args[tr("cmd")] == tr("coded")) {
        if(m_dh_completed) {
            QString get_args_decrypted_str;
            if(m_aes256_helper.decrypt(get_args[tr("msg")], get_args_decrypted_str, m_dh_helper.get_key())) {
                ui->plainTextEdit->appendPlainText(tr(" decoded message: ") + get_args_decrypted_str);
                QMap<QString, QString> get_args_decrypted;
                QStringList args_res_data = get_args_decrypted_str.split(tr("&"));
                QStringList pair_data;
                for(int i = 0; i < args_res_data.size(); i++) {
                    pair_data = args_res_data[i].split(tr("="));
                    if(pair_data.size() == 2) {
                        get_args_decrypted[pair_data[0]] = pair_data[1];
                    }
                }
            }
        } else {
            return tr("");
        }
    }
    return tr("");
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
