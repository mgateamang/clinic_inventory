#pragma once // Ensure header is included only once per translation unit

#include <QObject> // Include base Qt object model header
#include <QString> // Include Qt Unicode string class
#include <QVariantList> // Include Qt variant list collection
#include <QVariantMap> // Include Qt variant map dictionary
#include <QNetworkAccessManager> // Include Qt network access coordinator
#include <QNetworkReply> // Include Qt network reply handler
#include <QTcpSocket> // Include Qt TCP socket for real-time channels
#include <QTimer> // Include Qt timer for scheduled callbacks

namespace aidans { // Enclosing clinical suite namespace

/**
 * @class InventoryController
 * @brief ViewModel controller for medical inventory client.
 */
class InventoryController : public QObject { // Inherit QObject for meta-object system
    Q_OBJECT // Macro enabling signals, slots, and properties

    Q_PROPERTY(QVariantList catalog READ catalog NOTIFY catalogChanged) // Expose catalog list to QML
    Q_PROPERTY(QVariantList lowStockItems READ lowStockItems NOTIFY lowStockItemsChanged) // Expose low stock alerts
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged) // Expose status message text
    Q_PROPERTY(bool isBusy READ isBusy NOTIFY isBusyChanged) // Expose network busy state flag
    Q_PROPERTY(bool isServerOnline READ isServerOnline NOTIFY isServerOnlineChanged) // Expose server connectivity state
    Q_PROPERTY(QString serverUrl READ serverUrl NOTIFY serverUrlChanged) // Expose current REST API server URL
    Q_PROPERTY(QString connectionStatus READ connectionStatus NOTIFY connectionStatusChanged) // Expose connection status
    Q_PROPERTY(QString redisHost READ redisHost NOTIFY redisConfigChanged) // Expose cache host string
    Q_PROPERTY(int redisPort READ redisPort NOTIFY redisConfigChanged) // Expose cache port integer
    Q_PROPERTY(bool isRedisConnected READ isRedisConnected NOTIFY isRedisConnectedChanged) // Expose cache connection state
    Q_PROPERTY(QString redisStatus READ redisStatus NOTIFY redisStatusChanged) // Expose human-readable cache status

public: // Public interface methods
    explicit InventoryController(const QString& configFilePath = "config.ini", QObject* parent = nullptr); // Constructor
    ~InventoryController() override = default; // Default destructor

    QVariantList catalog() const { return m_catalog; } // Getter for catalog items
    QVariantList lowStockItems() const { return m_lowStockItems; } // Getter for items below reorder threshold
    QString statusMessage() const { return m_statusMessage; } // Getter for UI status message
    bool isBusy() const { return m_isBusy; } // Getter for busy network flag
    bool isServerOnline() const { return m_isServerOnline; } // Getter for server online flag
    QString serverUrl() const { return m_serverUrl; } // Getter for active backend server URL
    QString connectionStatus() const { return m_connectionStatus; } // Getter for connection status text
    QString redisHost() const { return m_redisHost; } // Getter for cache host
    int redisPort() const { return m_redisPort; } // Getter for cache port
    bool isRedisConnected() const { return m_isRedisConnected; } // Getter for cache connection state
    QString redisStatus() const { return m_redisStatus; } // Getter for cache status description

    Q_INVOKABLE void loadConfig(); // Load INI configuration from disk
    Q_INVOKABLE void saveServerUrl(const QString& newUrl); // Persist updated server URL
    Q_INVOKABLE void saveConfig(const QString& url, const QString& redisHost, int redisPort); // Save full configuration
    Q_INVOKABLE void testConnection(const QString& targetUrl); // Test server connectivity asynchronously
    Q_INVOKABLE void reconnectRedis(); // Attempt cache reconnection
    Q_INVOKABLE void refreshData(); // Refresh inventory data from backend API
    Q_INVOKABLE bool restockItem(int itemId, int quantity, const QString& reason); // Submit restock request
    Q_INVOKABLE bool createItem(const QString& sku, const QString& name, const QString& category, double unitPrice, int stock, int threshold, const QString& unit); // Create new inventory item

    // Filter helper
    static QVariantList filterLowStock(const QVariantList& items); // Static utility to filter low stock items

signals: // Qt notification signals
    void catalogChanged(); // Emitted when catalog updates
    void lowStockItemsChanged(); // Emitted when low stock list changes
    void statusMessageChanged(); // Emitted when status message updates
    void isBusyChanged(); // Emitted when busy state toggles
    void isServerOnlineChanged(); // Emitted when server connectivity changes
    void serverUrlChanged(); // Emitted when server URL changes
    void connectionStatusChanged(); // Emitted when connection status string changes
    void redisConfigChanged(); // Emitted when cache configuration changes
    void isRedisConnectedChanged(); // Emitted when cache connection status changes
    void redisStatusChanged(); // Emitted when cache description changes
    void restockCompleted(int itemId, int newStock); // Emitted upon successful restocking
    void itemCreated(const QString& sku); // Emitted when a new item is created

private: // Private member state and helpers
    QString m_configPath; // Path to config.ini file
    QString m_serverUrl{"http://localhost:8080/api/v1"}; // Active backend server URL
    QString m_redisHost{"localhost"}; // Cache host string
    int m_redisPort{6379}; // Cache port number
    bool m_isRedisConnected{false}; // Cache connection flag
    QString m_redisStatus{"Disconnected"}; // Cache status display string
    QTcpSocket* m_redisSocket{nullptr}; // TCP socket for cache
    QTimer* m_redisReconnectTimer{nullptr}; // Reconnect timer for cache

    QVariantList m_catalog; // Cached list of inventory items
    QVariantList m_lowStockItems; // Cached list of low stock items
    QString m_statusMessage; // Active status message text
    QString m_connectionStatus; // Detailed connection diagnostic text
    bool m_isBusy{false}; // Busy network flag
    bool m_isServerOnline{true}; // Server availability status flag
    QNetworkAccessManager m_netManager; // Network manager instance

    void setStatus(const QString& msg); // Set status message helper
    void setBusy(bool busy); // Set busy state helper
    void setServerOnline(bool online); // Set server availability helper
    void setConnectionStatus(const QString& status); // Set diagnostic connection text helper
    void initRedisSubscriber(); // Initialize cache subscriber channel
    void scheduleRedisReconnect(); // Schedule cache reconnection attempt
}; // End of InventoryController class definition

} // namespace aidans
