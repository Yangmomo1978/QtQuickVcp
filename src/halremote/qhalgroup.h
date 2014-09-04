#ifndef QHALGROUP_H
#define QHALGROUP_H

#include <QQuickItem>
#include <QHash>
#include <QTimer>
#include <QJsonObject>
#include "qhalsignal.h"
#include <nzmqt/nzmqt.hpp>
#include <google/protobuf/text_format.h>
#include "message.pb.h"

#if defined(Q_OS_IOS)
namespace gpb = google_public::protobuf;
#else
namespace gpb = google::protobuf;
#endif

using namespace nzmqt;

class QHalGroup : public QQuickItem
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QString halgroupUri READ halgroupUri WRITE setHalgroupUri NOTIFY halgroupUriChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(bool ready READ ready WRITE setReady NOTIFY readyChanged)
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)
    Q_PROPERTY(State connectionState READ connectionState NOTIFY connectionStateChanged)
    Q_PROPERTY(ConnectionError error READ error NOTIFY errorChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)
    Q_PROPERTY(QQuickItem *containerItem READ containerItem WRITE setContainerItem NOTIFY containerItemChanged)
    Q_PROPERTY(QJsonObject values READ values NOTIFY valuesChanged)
    Q_ENUMS(State ConnectionError)

public:
    explicit QHalGroup(QQuickItem *parent = 0);
    ~QHalGroup();

    enum SocketState {
        Down = 1,
        Trying = 2,
        Up = 3
    };

    enum State {
        Disconnected = 0,
        Connecting = 1,
        Connected = 2,
        Error = 3
    };

    enum ConnectionError {
        NoError = 0,
        HalGroupError = 1,
        TimeoutError = 2,
        SocketError = 3
    };

    virtual void componentComplete();

    QString halgroupUri() const
    {
        return m_halgroupUri;
    }

    QString name() const
    {
        return m_name;
    }

    bool ready() const
    {
        return m_ready;
    }

    State connectionState() const
    {
        return m_connectionState;
    }

    ConnectionError error() const
    {
        return m_error;
    }

    QString errorString() const
    {
        return m_errorString;
    }

    QQuickItem * containerItem() const
    {
        return m_containerItem;
    }

    QJsonObject values() const
    {
        return m_values;
    }

    bool isConnected() const
    {
        return m_connected;
    }

public slots:

    void setHalgroupUri(QString arg)
    {
        if (m_halgroupUri != arg) {
            m_halgroupUri = arg;
            emit halgroupUriChanged(arg);
        }
    }

    void setName(QString arg)
    {
        if (m_name != arg) {
            m_name = arg;
            emit nameChanged(arg);
        }
    }

    void setReady(bool arg);

    void setContainerItem(QQuickItem * arg)
    {
        if (m_containerItem != arg) {
            m_containerItem = arg;
            emit containerItemChanged(arg);
        }
    }

private:
    QString     m_halgroupUri;
    QString     m_name;
    bool        m_ready;
    bool        m_connected;
    SocketState m_sState;
    State       m_connectionState;
    ConnectionError m_error;
    QString     m_errorString;
    QQuickItem  *m_containerItem;
    QJsonObject m_values;
    bool        m_componentCompleted;

    PollingZMQContext *m_context;
    ZMQSocket   *m_halgroupSocket;
    QTimer      *m_halgroupHeartbeatTimer;
    bool        m_halgroupPingOutstanding;
    // more efficient to reuse a protobuf Message
    pb::Container   m_rx;
    pb::Container   m_tx;
    QMap<QString, QHalSignal*> m_signalsByName;
    QHash<int, QHalSignal*>    m_signalsByHandle;
    QList<QHalSignal*>         m_localSignals;

    QList<QHalSignal*> recurseObjects(const QObjectList &list) const;
    void start();
    void stop();
    void startHalgroupHeartbeat(int interval);
    void stopHalgroupHeartbeat();
    void refreshHalgroupHeartbeat();
    void updateState(State state);
    void updateError(ConnectionError error, const QString &errorString);

private slots:
    void signalUpdate(const pb::Signal &remoteSignal, QHalSignal *localSignal);

    void halgroupMessageReceived(const QList<QByteArray> &messageList);
    void pollError(int errorNum, const QString &errorMsg);
    void halgroupHeartbeatTimerTick();

    void addSignals();
    void removeSignals();
    void unsyncSignals();
    bool connectSockets();
    void disconnectSockets();
    void subscribe();
    void unsubscribe();

signals:
    void halgroupUriChanged(QString arg);
    void nameChanged(QString arg);
    void readyChanged(bool arg);
    void connectionStateChanged(State arg);
    void errorChanged(ConnectionError arg);
    void errorStringChanged(QString arg);
    void containerItemChanged(QQuickItem * arg);
    void valuesChanged(QJsonObject arg);
    void connectedChanged(bool arg);
};

#endif // QHALGROUP_H