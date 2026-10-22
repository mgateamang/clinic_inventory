/**
 * @file inventory_controller.cpp
 * @brief Implementation of InventoryController for Qt6 / QML client.
 *
 * Implements non-blocking HTTP network calls to clinic_server, JSON parsing
 * of inventory catalogs, safety stock calculation, and INI configuration management.
 */

#include "inventory_controller.hpp" // Include inventory_controller.hpp for declarations and types
#include <QSettings> // Include QSettings for declarations and types
#include <QJsonDocument> // Include QJsonDocument for declarations and types
#include <QJsonObject> // Include QJsonObject for declarations and types
#include <QJsonArray> // Include QJsonArray for declarations and types
#include <QUrl> // Include QUrl for declarations and types
#include <QFileInfo> // Include QFileInfo for declarations and types

namespace aidans { // Execute clinical workflow step

/**
 * @brief Constructs the controller and reads local configuration.
 */
InventoryController::InventoryController(const QString& configFilePath, QObject* parent) // Execute clinical workflow step
    : QObject(parent), m_configPath(configFilePath) { // Execute clinical workflow step
    loadConfig(); // Execute clinical workflow step
} // Close active scope block

void InventoryController::loadConfig() { // Execute clinical workflow step
    if (QFileInfo::exists(m_configPath)) { // Evaluate conditional expression to branch execution flow
        QSettings settings(m_configPath, QSettings::IniFormat); // Execute clinical workflow step
        settings.beginGroup("Server"); // Execute clinical workflow step
        QString url = settings.value("url", m_serverUrl).toString(); // Assign value to target variable
        m_redisHost = settings.value("redis_host", m_redisHost).toString(); // Assign value to target variable
        m_redisPort = settings.value("redis_port", m_redisPort).toInt(); // Assign value to target variable
        settings.endGroup(); // Execute clinical workflow step
        if (!url.isEmpty()) { // Evaluate conditional expression to branch execution flow
            m_serverUrl = url; // Assign value to target variable
            emit serverUrlChanged(); // Notify QML UI bindings of underlying property state update
        } // Close active scope block
        emit redisConfigChanged(); // Notify QML UI bindings of underlying property state update
    } // Close active scope block
    initRedisSubscriber(); // Execute clinical workflow step
} // Close active scope block

void InventoryController::saveConfig(const QString& url, const QString& redisHost, int redisPort) { // Execute clinical workflow step
    QString trimmedUrl = url.trimmed(); // Assign value to target variable
    QString trimmedRedisHost = redisHost.trimmed().isEmpty() ? "localhost" : redisHost.trimmed(); // Assign value to target variable
    int validPort = redisPort <= 0 ? 6379 : redisPort; // Assign value to target variable

    if (!trimmedUrl.isEmpty()) { // Evaluate conditional expression to branch execution flow
        m_serverUrl = trimmedUrl; // Assign value to target variable
        emit serverUrlChanged(); // Notify QML UI bindings of underlying property state update
    } // Close active scope block

    m_redisHost = trimmedRedisHost; // Assign value to target variable
    m_redisPort = validPort; // Assign value to target variable
    emit redisConfigChanged(); // Notify QML UI bindings of underlying property state update

    QSettings settings(m_configPath, QSettings::IniFormat); // Execute clinical workflow step
    settings.beginGroup("Server"); // Execute clinical workflow step
    settings.setValue("url", m_serverUrl); // Execute clinical workflow step
    settings.setValue("redis_host", m_redisHost); // Execute clinical workflow step
    settings.setValue("redis_port", m_redisPort); // Execute clinical workflow step
    settings.endGroup(); // Execute clinical workflow step
    settings.sync(); // Execute clinical workflow step

    reconnectRedis(); // Execute clinical workflow step
    setStatus("Configuration saved to " + m_configPath); // Execute clinical workflow step
} // Close active scope block

void InventoryController::saveServerUrl(const QString& newUrl) { // Execute clinical workflow step
    saveConfig(newUrl, m_redisHost, m_redisPort); // Execute clinical workflow step
} // Close active scope block

void InventoryController::initRedisSubscriber() { // Execute clinical workflow step
    if (!m_redisSocket) { // Evaluate conditional expression to branch execution flow
        m_redisSocket = new QTcpSocket(this); // Assign value to target variable

        connect(m_redisSocket, &QTcpSocket::connected, this, [this]() { // Connect Qt signal to asynchronous member slot handler
            m_isRedisConnected = true; // Assign value to target variable
            m_redisStatus = QString("Connected to %1:%2 (listening on inventory_updates)").arg(m_redisHost).arg(m_redisPort); // Assign value to target variable
            emit isRedisConnectedChanged(); // Notify QML UI bindings of underlying property state update
            emit redisStatusChanged(); // Notify QML UI bindings of underlying property state update

            m_redisSocket->write("SUBSCRIBE inventory_updates\r\n"); // Execute clinical workflow step
            m_redisSocket->flush(); // Execute clinical workflow step
            setStatus("Connected to Redis real-time sync."); // Execute clinical workflow step
        }); // Execute clinical workflow step

        connect(m_redisSocket, &QTcpSocket::disconnected, this, [this]() { // Connect Qt signal to asynchronous member slot handler
            m_isRedisConnected = false; // Assign value to target variable
            m_redisStatus = "Disconnected"; // Assign value to target variable
            emit isRedisConnectedChanged(); // Notify QML UI bindings of underlying property state update
            emit redisStatusChanged(); // Notify QML UI bindings of underlying property state update
            scheduleRedisReconnect(); // Connect Qt signal to asynchronous member slot handler
        }); // Execute clinical workflow step

        connect(m_redisSocket, &QTcpSocket::errorOccurred, this, [this](QAbstractSocket::SocketError) { // Connect Qt signal to asynchronous member slot handler
            m_isRedisConnected = false; // Assign value to target variable
            m_redisStatus = "Redis offline: " + m_redisSocket->errorString(); // Assign value to target variable
            emit isRedisConnectedChanged(); // Notify QML UI bindings of underlying property state update
            emit redisStatusChanged(); // Notify QML UI bindings of underlying property state update
            scheduleRedisReconnect(); // Connect Qt signal to asynchronous member slot handler
        }); // Execute clinical workflow step

        connect(m_redisSocket, &QTcpSocket::readyRead, this, [this]() { // Connect Qt signal to asynchronous member slot handler
            QByteArray data = m_redisSocket->readAll(); // Assign value to target variable
            if (data.contains("inventory_updates") && (data.contains("message") || data.contains("event") || data.contains("action"))) { // Evaluate conditional expression to branch execution flow
                refreshData(); // Execute clinical workflow step
                setStatus("Real-time inventory ripple update received from Redis."); // Execute clinical workflow step
            } // Close active scope block
        }); // Execute clinical workflow step
    } // Close active scope block

    reconnectRedis(); // Execute clinical workflow step
} // Close active scope block

void InventoryController::reconnectRedis() { // Execute clinical workflow step
    if (m_redisSocket) { // Evaluate conditional expression to branch execution flow
        m_redisSocket->abort(); // Execute clinical workflow step
        m_redisSocket->connectToHost(m_redisHost, m_redisPort); // Execute clinical workflow step
    } // Close active scope block
} // Close active scope block

void InventoryController::scheduleRedisReconnect() { // Connect Qt signal to asynchronous member slot handler
    if (!m_redisReconnectTimer) { // Evaluate conditional expression to branch execution flow
        m_redisReconnectTimer = new QTimer(this); // Assign value to target variable
        m_redisReconnectTimer->setSingleShot(true); // Execute clinical workflow step
        connect(m_redisReconnectTimer, &QTimer::timeout, this, [this]() { // Connect Qt signal to asynchronous member slot handler
            if (!m_isRedisConnected) { // Evaluate conditional expression to branch execution flow
                reconnectRedis(); // Execute clinical workflow step
            } // Close active scope block
        }); // Execute clinical workflow step
    } // Close active scope block
    if (!m_redisReconnectTimer->isActive()) { // Evaluate conditional expression to branch execution flow
        m_redisReconnectTimer->start(3000); // Execute clinical workflow step
    } // Close active scope block
} // Close active scope block

void InventoryController::setStatus(const QString& msg) { // Execute clinical workflow step
    if (m_statusMessage != msg) { // Evaluate conditional expression to branch execution flow
        m_statusMessage = msg; // Assign value to target variable
        emit statusMessageChanged(); // Notify QML UI bindings of underlying property state update
    } // Close active scope block
} // Close active scope block

void InventoryController::setBusy(bool busy) { // Execute clinical workflow step
    if (m_isBusy != busy) { // Evaluate conditional expression to branch execution flow
        m_isBusy = busy; // Assign value to target variable
        emit isBusyChanged(); // Notify QML UI bindings of underlying property state update
    } // Close active scope block
} // Close active scope block

void InventoryController::setServerOnline(bool online) { // Execute clinical workflow step
    if (m_isServerOnline != online) { // Evaluate conditional expression to branch execution flow
        m_isServerOnline = online; // Assign value to target variable
        emit isServerOnlineChanged(); // Notify QML UI bindings of underlying property state update
    } // Close active scope block
} // Close active scope block

void InventoryController::setConnectionStatus(const QString& status) { // Execute clinical workflow step
    if (m_connectionStatus != status) { // Evaluate conditional expression to branch execution flow
        m_connectionStatus = status; // Assign value to target variable
        emit connectionStatusChanged(); // Notify QML UI bindings of underlying property state update
    } // Close active scope block
} // Close active scope block

QVariantList InventoryController::filterLowStock(const QVariantList& items) { // Execute clinical workflow step
    QVariantList low; // Execute clinical workflow step
    for (const auto& itemVar : items) { // Iterate through items in sequence or collection
        QVariantMap item = itemVar.toMap(); // Assign value to target variable
        int stock = item["stock_quantity"].toInt(); // Assign value to target variable
        int threshold = item["reorder_threshold"].toInt(); // Assign value to target variable
        if (stock <= threshold) { // Evaluate conditional expression to branch execution flow
            low.append(item); // Execute clinical workflow step
        } // Close active scope block
    } // Close active scope block
    return low; // Return computed result to caller
} // Close active scope block

void InventoryController::testConnection(const QString& targetUrl) { // Execute clinical workflow step
    QString urlStr = targetUrl.trimmed(); // Assign value to target variable
    if (urlStr.isEmpty()) urlStr = m_serverUrl; // Evaluate conditional expression to branch execution flow

    setBusy(true); // Execute clinical workflow step
    setConnectionStatus("Pinging " + urlStr + "..."); // Execute clinical workflow step

    QString healthUrl = urlStr; // Assign value to target variable
    if (!healthUrl.endsWith("/health") && !healthUrl.endsWith("/api/v1/health")) { // Evaluate conditional expression to branch execution flow
        healthUrl = urlStr + "/health"; // Assign value to target variable
    } // Close active scope block

    QUrl target(healthUrl); // Execute clinical workflow step
    QNetworkRequest req(target); // Construct HTTP request specification with target URL and headers
    req.setAttribute(QNetworkRequest::Http2AllowedAttribute, false); // Construct HTTP request specification with target URL and headers

    QNetworkReply* reply = m_netManager.get(req); // Assign value to target variable
    connect(reply, &QNetworkReply::finished, this, [this, reply, urlStr]() { // Connect Qt signal to asynchronous member slot handler
        reply->deleteLater(); // Schedule network reply object for safe asynchronous deletion
        setBusy(false); // Execute clinical workflow step

        if (reply->error() == QNetworkReply::NoError) { // Evaluate conditional expression to branch execution flow
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll()); // Read incoming response byte stream into buffer
            QString service = doc.object().value("service").toString(); // Assign value to target variable
            QString version = doc.object().value("version").toString(); // Assign value to target variable
            QString redisInfo = doc.object().value("redis").toString(); // Assign value to target variable
            setConnectionStatus(QString("Connected to %1 (%2 v%3, Redis: %4)").arg(urlStr).arg(service).arg(version).arg(redisInfo)); // Execute clinical workflow step
            setServerOnline(true); // Execute clinical workflow step
        } else { // Execute default fallback branch
            setConnectionStatus("Failed to connect: " + reply->errorString()); // Execute clinical workflow step
            setServerOnline(false); // Execute clinical workflow step
        } // Close active scope block
    }); // Execute clinical workflow step
} // Close active scope block

void InventoryController::refreshData() { // Execute clinical workflow step
    setBusy(true); // Execute clinical workflow step
    setStatus("Fetching inventory supplies from clinic server..."); // Execute clinical workflow step

    QNetworkRequest req(QUrl(m_serverUrl + "/inventory")); // Construct HTTP request specification with target URL and headers
    QNetworkReply* reply = m_netManager.get(req); // Assign value to target variable
    connect(reply, &QNetworkReply::finished, this, [this, reply]() { // Connect Qt signal to asynchronous member slot handler
        reply->deleteLater(); // Schedule network reply object for safe asynchronous deletion
        setBusy(false); // Execute clinical workflow step

        if (reply->error() == QNetworkReply::NoError) { // Evaluate conditional expression to branch execution flow
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll()); // Read incoming response byte stream into buffer
            if (doc.isArray()) { // Evaluate conditional expression to branch execution flow
                m_catalog = doc.toVariant().toList(); // Assign value to target variable
                m_lowStockItems = filterLowStock(m_catalog); // Assign value to target variable
                emit catalogChanged(); // Notify QML UI bindings of underlying property state update
                emit lowStockItemsChanged(); // Notify QML UI bindings of underlying property state update
                setServerOnline(true); // Execute clinical workflow step
                setStatus(QString("Catalog loaded: %1 total items, %2 needing reorder.") // Execute clinical workflow step
                    .arg(m_catalog.size()).arg(m_lowStockItems.size())); // Execute clinical workflow step
            } // Close active scope block
        } else { // Execute default fallback branch
            setServerOnline(false); // Execute clinical workflow step
            setStatus("Server offline or unreachable: " + reply->errorString()); // Execute clinical workflow step
        } // Close active scope block
    }); // Execute clinical workflow step
} // Close active scope block

bool InventoryController::restockItem(int itemId, int quantity, const QString& reason) { // Execute clinical workflow step
    if (itemId <= 0 || quantity == 0 || reason.trimmed().isEmpty()) { // Evaluate conditional expression to branch execution flow
        setStatus("Error: Invalid restock parameters. Quantity cannot be 0 and reason required."); // Execute clinical workflow step
        return false; // Return computed result to caller
    } // Close active scope block

    setBusy(true); // Execute clinical workflow step
    setStatus(QString("Applying restock (%1) for item ID %2...").arg(quantity).arg(itemId)); // Execute clinical workflow step

    QJsonObject payload; // Execute clinical workflow step
    payload["quantity"] = quantity; // Assign value to target variable
    payload["reason"] = reason.trimmed(); // Assign value to target variable

    QNetworkRequest req(QUrl(QString("%1/inventory/%2/restock").arg(m_serverUrl).arg(itemId))); // Construct HTTP request specification with target URL and headers
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json"); // Construct HTTP request specification with target URL and headers

    QNetworkReply* reply = m_netManager.post(req, QJsonDocument(payload).toJson(QJsonDocument::Compact)); // Assign value to target variable
    connect(reply, &QNetworkReply::finished, this, [this, reply, itemId]() { // Connect Qt signal to asynchronous member slot handler
        reply->deleteLater(); // Schedule network reply object for safe asynchronous deletion
        setBusy(false); // Execute clinical workflow step

        if (reply->error() == QNetworkReply::NoError) { // Evaluate conditional expression to branch execution flow
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll()); // Read incoming response byte stream into buffer
            int newStock = doc.object().value("stock_quantity").toInt(); // Assign value to target variable
            QString name = doc.object().value("name").toString(); // Assign value to target variable

            setServerOnline(true); // Execute clinical workflow step
            setStatus(QString("Restocked %1 successfully! New stock: %2").arg(name).arg(newStock)); // Execute clinical workflow step
            emit restockCompleted(itemId, newStock); // Notify QML UI bindings of underlying property state update
            refreshData(); // Refresh full catalog and low-stock list
        } else { // Execute default fallback branch
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll()); // Read incoming response byte stream into buffer
            QString errStr = doc.object().value("error").toString(); // Assign value to target variable
            if (errStr.isEmpty()) errStr = reply->errorString(); // Evaluate conditional expression to branch execution flow
            setStatus("Restock failed: " + errStr); // Execute clinical workflow step
        } // Close active scope block
    }); // Execute clinical workflow step

    return true; // Return computed result to caller
} // Close active scope block

bool InventoryController::createItem(const QString& sku, const QString& name, const QString& category, // Execute clinical workflow step
                                     double unitPrice, int stock, int threshold, const QString& unit) { // Execute clinical workflow step
    if (sku.trimmed().isEmpty() || name.trimmed().isEmpty() || unitPrice < 0.0 || stock < 0 || threshold < 0) { // Evaluate conditional expression to branch execution flow
        setStatus("Error: Invalid item parameters. Check SKU, Name, and positive numbers."); // Execute clinical workflow step
        return false; // Return computed result to caller
    } // Close active scope block

    setBusy(true); // Execute clinical workflow step
    setStatus("Adding new supply item to catalog..."); // Execute clinical workflow step

    QJsonObject payload; // Execute clinical workflow step
    payload["sku"] = sku.trimmed(); // Assign value to target variable
    payload["name"] = name.trimmed(); // Assign value to target variable
    payload["category"] = category.trimmed().isEmpty() ? "General" : category.trimmed(); // Assign value to target variable
    payload["unit_price"] = unitPrice; // Assign value to target variable
    payload["stock_quantity"] = stock; // Assign value to target variable
    payload["reorder_threshold"] = threshold; // Assign value to target variable
    payload["unit"] = unit.trimmed().isEmpty() ? "unit" : unit.trimmed(); // Assign value to target variable

    QNetworkRequest req(QUrl(m_serverUrl + "/inventory")); // Construct HTTP request specification with target URL and headers
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json"); // Construct HTTP request specification with target URL and headers

    QNetworkReply* reply = m_netManager.post(req, QJsonDocument(payload).toJson(QJsonDocument::Compact)); // Assign value to target variable
    connect(reply, &QNetworkReply::finished, this, [this, reply, sku]() { // Connect Qt signal to asynchronous member slot handler
        reply->deleteLater(); // Schedule network reply object for safe asynchronous deletion
        setBusy(false); // Execute clinical workflow step

        if (reply->error() == QNetworkReply::NoError) { // Evaluate conditional expression to branch execution flow
            setServerOnline(true); // Execute clinical workflow step
            setStatus("Item " + sku + " created successfully!"); // Execute clinical workflow step
            emit itemCreated(sku); // Notify QML UI bindings of underlying property state update
            refreshData(); // Execute clinical workflow step
        } else { // Execute default fallback branch
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll()); // Read incoming response byte stream into buffer
            QString errStr = doc.object().value("error").toString(); // Assign value to target variable
            if (errStr.isEmpty()) errStr = reply->errorString(); // Evaluate conditional expression to branch execution flow
            setStatus("Creation failed: " + errStr); // Execute clinical workflow step
        } // Close active scope block
    }); // Execute clinical workflow step

    return true; // Return computed result to caller
} // Close active scope block

} // namespace aidans
