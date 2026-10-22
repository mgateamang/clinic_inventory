#pragma once

/**
 * @file inventory_controller.hpp
 * @brief ViewModel Controller for AIDANS Medical Inventory Management Client.
 *
 * Exposes a reactive Qt C++ interface to declarative QML UI views.
 * Responsibilities:
 *  - Communicates asynchronously with the clinic_server REST API via QNetworkAccessManager.
 *  - Maintains live inventory catalog and calculated low-stock alerts.
 *  - Reads and persists local connection settings from/to config.ini.
 *  - Performs non-blocking health checks and connection latency diagnostics.
 *  - Validates and dispatches stock replenishment (restock) requests.
 */

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

    // Reactive properties exposed directly to QML binding expressions
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
    /**
     * @brief Constructs controller, loads INI configuration, and checks server connectivity.
     * @param configFilePath Path to local INI configuration file.
     * @param parent Qt parent object hierarchy pointer.
     */
    explicit InventoryController(const QString& configFilePath = "config.ini", QObject* parent = nullptr);
    ~InventoryController() override = default;

    // Property Getters
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

    /**
     * @brief Loads server base URL and parameters from disk config.ini.
     */
    Q_INVOKABLE void loadConfig();

    /**
     * @brief Persists new server base URL to disk and triggers immediate reconnect.
     * @param newUrl Target backend endpoint (e.g. "http://127.0.0.1:8080").
     */
    Q_INVOKABLE void saveServerUrl(const QString& newUrl);

    /**
     * @brief Saves extended network configuration to config.ini.
     */
    Q_INVOKABLE void saveConfig(const QString& url, const QString& redisHost, int redisPort);

    /**
     * @brief Performs an asynchronous probe to verify server reachability.
     * @param targetUrl Server URL to test.
     */
    Q_INVOKABLE void testConnection(const QString& targetUrl);

    /**
     * @brief Diagnostic redis reconnect stub (retained for backward compatibility).
     */
    Q_INVOKABLE void reconnectRedis();

    /**
     * @brief Asynchronously fetches the latest medical supply catalog from backend.
     */
    Q_INVOKABLE void refreshData();

    /**
     * @brief Restocks an inventory item with an audited increment and reason.
     * @param itemId Unique ID of item.
     * @param quantity Number of units added.
     * @param reason Audit justification (e.g., "PO #1042 Delivery").
     * @return true if network request successfully queued.
     */
    Q_INVOKABLE bool restockItem(int itemId, int quantity, const QString& reason);

    /**
     * @brief Adds a brand new medicine or medical supply item into the catalog.
     * @param sku Unique barcode / stock keeping unit.
     * @param name Descriptive item name.
     * @param category Classification category.
     * @param unitPrice Selling price per unit.
     * @param stock Initial inventory quantity on hand.
     * @param threshold Minimum safety stock limit.
     * @param unit Packaging unit ("bottles", "tablets", "boxes").
     * @return true if request dispatched.
     */
    Q_INVOKABLE bool createItem(const QString& sku, const QString& name, const QString& category,
                                double unitPrice, int stock, int threshold, const QString& unit);

    /**
     * @brief Pure algorithmic helper to filter out items running low on stock.
     * @param items Complete list of catalog items.
     * @return Subset of items where stock_quantity <= reorder_threshold.
     */
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
    void restockCompleted(bool success, const QString& message);
    void itemCreated(bool success, const QString& message);

private:
    QString m_configFilePath{"config.ini"};
    QString m_serverUrl{"http://localhost:8080"};
    QString m_redisHost{"127.0.0.1"};
    int m_redisPort{6379};
    bool m_isRedisConnected{false};
    QString m_redisStatus{"Disabled"};

    QVariantList m_catalog;
    QVariantList m_lowStockItems;
    QString m_statusMessage{"Ready"};
    bool m_isBusy{false};
    bool m_isServerOnline{false};
    QString m_connectionStatus{"Checking..."};

    QNetworkAccessManager* m_netManager{nullptr};
    QTcpSocket* m_redisSocket{nullptr};
    QTimer* m_redisReconnectTimer{nullptr};

    void checkServerHealth();
    void updateLowStockItems();
};

} // namespace aidans
