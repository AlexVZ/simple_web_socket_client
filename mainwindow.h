
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWebSockets/QWebSocket>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    virtual void keyPressEvent(QKeyEvent *e);

private:
    Ui::MainWindow *ui;

    bool m_connected;
   QWebSocket *m_web_socket;
   int m_connect_status;

   void connect_game_server();
   void disconnect_game_server();

   void setAllEnabled(bool enabled);

   QString trim_host(QString src_host);

private slots:
   void on_actionExit_triggered();

   void connectButton_clicked(bool);
   void sendButton_clicked(bool);

   void is_error(QAbstractSocket::SocketError socketError);
   void state_changed(QAbstractSocket::SocketState state);
   void processTextMessage(QString message);
};

#endif // MAINWINDOW_H
