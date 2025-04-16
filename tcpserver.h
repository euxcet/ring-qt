#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QList>

class TcpServer : public QTcpServer {
    Q_OBJECT
public:
    TcpServer(QObject *parent = nullptr);
    ~TcpServer();
    void send(const QByteArray& data);

signals:
    void dataFromClient(QByteArray data);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void clientConnected();
    void clientDisconnected();
    void readyRead();
    // void error(QAbstractSocket::SocketError socketError);

private:
    QList<QTcpSocket*> m_clients;
};

#endif // TCPSERVER_H
