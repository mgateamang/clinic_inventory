#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTcpSocket>
#include <QTimer>

namespace aidans {

class InventoryController : public QObject {
    Q_OBJECT

    Q_PROPERTY(QVariantList catalog READ catalog NOTIFY catalogChanged)
    Q_PROPERTY(QVariantList lowStockItems READ lowStockItems NOTIFY lowStockItemsChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(bool isBusy READ isBusy NOTIFY isBusyChanged)
    Q_PROPERTY(bool isServerOnline READ isServerOnline NOTIFY isServerOnlineChanged)
    Q_PROPERTY(QString serverUrl READ serverUrl NOTIFY serverUrlChanged)
    Q_PROPERTY(QString connectionStatus READ connectionStatus NOTIFY connectionStatusChanged)
    Q_PROPERTY(QString redisHost READ redisHost NOTIFY redisConfigChanged)
    Q_PROPERTY(int redisPort READ redisPort NOTIFY redisConfigChanged)
    Q_PROPERTY(bool isRedisConnected READ isRedisConnected NOTIFY isRedisConnectedChanged)
    Q_PROPERTY(QString redisStatus READ redisStatus NOTIFY redisStatusChanged)

public:
    explicit InventoryController(const QString& configFilePath = "config.ini", QObject* parent = nullptr);
    ~InventoryController() override = default;

    QVariantList catalog() const { return m_catalog; }
    QVariantList lowStockItems() const { return m_lowStockItems; }
    QString statusMessage() const { return m_statusMessage; }
    bool isBusy() const { return m_isBusy; }
    bool isServerOnline() const { return m_isServerOnline; }
    QString serverUrl() const { return m_serverUrl; }
    QString connectionStatus() const { return m_connectionStatus; }
    QString redisHost() const { return m_redisHost; }
    int redisPort() const { return m_redisPort; }
    bool isRedisConnected() const { return m_isRedisConnected; }
    QString redisStatus() const { return m_redisStatus; }

    Q_INVOKABLE void loadConfig();
    Q_INVOKABLE void saveServerUrl(const QString& newUrl);
    Q_INVOKABLE void saveConfig(const QString& url, const QString& redisHost, int redisPort);
    Q_INVOKABLE void testConnection(const QString& targetUrl);
    Q_INVOKABLE void reconnectRedis();
    Q_INVOKABLE void refreshData();
    Q_INVOKABLE bool restockItem(int itemId, int quantity, const QString& reason);
    Q_INVOKABLE bool createItem(const QString& sku, const QString& name, const QString& category,
                                double unitPrice, int stock, int threshold, const QString& unit);

    // Filter helper
    static QVariantList filterLowStock(const QVariantList& items);

signals:
    void catalogChanged();
    void lowStockItemsChanged();
    void statusMessageChanged();
    void isBusyChanged();
    void isServerOnlineChanged();
    void serverUrlChanged();
    void connectionStatusChanged();
    void redisConfigChanged();
    void isRedisConnectedChanged();
    void redisStatusChanged();
    void restockCompleted(int itemId, int newStock);
    void itemCreated(const QString& sku);

private:
    QString m_configPath;
    QString m_serverUrl{"http://localhost:8080/api/v1"};
    QString m_redisHost{"localhost"};
    int m_redisPort{6379};
    bool m_isRedisConnected{false};
    QString m_redisStatus{"Disconnected"};
    QTcpSocket* m_redisSocket{nullptr};
    QTimer* m_redisReconnectTimer{nullptr};

    QVariantList m_catalog;
    QVariantList m_lowStockItems;
    QString m_statusMessage;
    QString m_connectionStatus;
    bool m_isBusy{false};
    bool m_isServerOnline{true};
    QNetworkAccessManager m_netManager;

    void setStatus(const QString& msg);
    void setBusy(bool busy);
    void setServerOnline(bool online);
    void setConnectionStatus(const QString& status);
    void initRedisSubscriber();
    void scheduleRedisReconnect();
};

} // namespace aidans
