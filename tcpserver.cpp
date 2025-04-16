#include "tcpserver.h"

TcpServer::TcpServer(QObject *parent) : QTcpServer(parent) {
    connect(this, &QTcpServer::newConnection, this, &TcpServer::clientConnected);
}

TcpServer::~TcpServer() {
    for (QTcpSocket* client: m_clients) {
        client->disconnectFromHost();
        client->waitForDisconnected();
        client->deleteLater();
    }
    m_clients.clear();
}

void TcpServer::incomingConnection(qintptr socketDescriptor) {
    QTcpSocket *client = new QTcpSocket(this);
    if (!client->setSocketDescriptor(socketDescriptor)) {
        client->deleteLater();
        return;
    }
    m_clients.append(client);
    connect(client, &QTcpSocket::disconnected, this, &TcpServer::clientDisconnected);
    connect(client, &QTcpSocket::readyRead, this, &TcpServer::readyRead);
}

void TcpServer::clientConnected() {
    qDebug() << "client connected";
}

void TcpServer::clientDisconnected() {
    qDebug() << "client disconnected";
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (client) {
        m_clients.removeOne(client);
        client->deleteLater();
    }
}

void TcpServer::readyRead() {
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (client && client->bytesAvailable() > 0) {
        QByteArray data = client->readAll();
        emit dataFromClient(data);
    }
}

void TcpServer::send(const QByteArray& data) {
    for (QTcpSocket *client: m_clients) {
        if (client->state() == QAbstractSocket::ConnectedState) {
            client->write(data);
            client->flush();
        }
    }
}
