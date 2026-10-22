/**
 * @file inventory_controller.cpp
 * @brief Implementation of InventoryController for Qt6 / QML client.
 *
 * Implements non-blocking HTTP network calls to clinic_server, JSON parsing
 * of inventory catalogs, safety stock calculation, and INI configuration management.
 */

#include "inventory_controller.hpp"
#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QFileInfo>

namespace aidans {

/**
 * @brief Constructs the controller and reads local configuration.
 */
InventoryController::InventoryController(const QString& configFilePath, QObject* parent)
    : QObject(parent), m_configPath(configFilePath) {
    loadConfig();
}

void InventoryController::loadConfig() {
    if (QFileInfo::exists(m_configPath)) {
        QSettings settings(m_configPath, QSettings::IniFormat);
        settings.beginGroup("Server");
        QString url = settings.value("url", m_serverUrl).toString();
        m_redisHost = settings.value("redis_host", m_redisHost).toString();
        m_redisPort = settings.value("redis_port", m_redisPort).toInt();
        settings.endGroup();
        if (!url.isEmpty()) {
            m_serverUrl = url;
            emit serverUrlChanged();
        }
        emit redisConfigChanged();
    }
    initRedisSubscriber();
}

void InventoryController::saveConfig(const QString& url, const QString& redisHost, int redisPort) {
    QString trimmedUrl = url.trimmed();
    QString trimmedRedisHost = redisHost.trimmed().isEmpty() ? "localhost" : redisHost.trimmed();
    int validPort = redisPort <= 0 ? 6379 : redisPort;

    if (!trimmedUrl.isEmpty()) {
        m_serverUrl = trimmedUrl;
        emit serverUrlChanged();
    }

    m_redisHost = trimmedRedisHost;
    m_redisPort = validPort;
    emit redisConfigChanged();

    QSettings settings(m_configPath, QSettings::IniFormat);
    settings.beginGroup("Server");
    settings.setValue("url", m_serverUrl);
    settings.setValue("redis_host", m_redisHost);
    settings.setValue("redis_port", m_redisPort);
    settings.endGroup();
    settings.sync();

    reconnectRedis();
    setStatus("Configuration saved to " + m_configPath);
}

void InventoryController::saveServerUrl(const QString& newUrl) {
    saveConfig(newUrl, m_redisHost, m_redisPort);
}

void InventoryController::initRedisSubscriber() {
    if (!m_redisSocket) {
        m_redisSocket = new QTcpSocket(this);

        connect(m_redisSocket, &QTcpSocket::connected, this, [this]() {
            m_isRedisConnected = true;
            m_redisStatus = QString("Connected to %1:%2 (listening on inventory_updates)").arg(m_redisHost).arg(m_redisPort);
            emit isRedisConnectedChanged();
            emit redisStatusChanged();

            m_redisSocket->write("SUBSCRIBE inventory_updates\r\n");
            m_redisSocket->flush();
            setStatus("Connected to Redis real-time sync.");
        });

        connect(m_redisSocket, &QTcpSocket::disconnected, this, [this]() {
            m_isRedisConnected = false;
            m_redisStatus = "Disconnected";
            emit isRedisConnectedChanged();
            emit redisStatusChanged();
            scheduleRedisReconnect();
        });

        connect(m_redisSocket, &QTcpSocket::errorOccurred, this, [this](QAbstractSocket::SocketError) {
            m_isRedisConnected = false;
            m_redisStatus = "Redis offline: " + m_redisSocket->errorString();
            emit isRedisConnectedChanged();
            emit redisStatusChanged();
            scheduleRedisReconnect();
        });

        connect(m_redisSocket, &QTcpSocket::readyRead, this, [this]() {
            QByteArray data = m_redisSocket->readAll();
            if (data.contains("inventory_updates") && (data.contains("message") || data.contains("event") || data.contains("action"))) {
                refreshData();
                setStatus("Real-time inventory ripple update received from Redis.");
            }
        });
    }

    reconnectRedis();
}

void InventoryController::reconnectRedis() {
    if (m_redisSocket) {
        m_redisSocket->abort();
        m_redisSocket->connectToHost(m_redisHost, m_redisPort);
    }
}

void InventoryController::scheduleRedisReconnect() {
    if (!m_redisReconnectTimer) {
        m_redisReconnectTimer = new QTimer(this);
        m_redisReconnectTimer->setSingleShot(true);
        connect(m_redisReconnectTimer, &QTimer::timeout, this, [this]() {
            if (!m_isRedisConnected) {
                reconnectRedis();
            }
        });
    }
    if (!m_redisReconnectTimer->isActive()) {
        m_redisReconnectTimer->start(3000);
    }
}

void InventoryController::setStatus(const QString& msg) {
    if (m_statusMessage != msg) {
        m_statusMessage = msg;
        emit statusMessageChanged();
    }
}

void InventoryController::setBusy(bool busy) {
    if (m_isBusy != busy) {
        m_isBusy = busy;
        emit isBusyChanged();
    }
}

void InventoryController::setServerOnline(bool online) {
    if (m_isServerOnline != online) {
        m_isServerOnline = online;
        emit isServerOnlineChanged();
    }
}

void InventoryController::setConnectionStatus(const QString& status) {
    if (m_connectionStatus != status) {
        m_connectionStatus = status;
        emit connectionStatusChanged();
    }
}

QVariantList InventoryController::filterLowStock(const QVariantList& items) {
    QVariantList low;
    for (const auto& itemVar : items) {
        QVariantMap item = itemVar.toMap();
        int stock = item["stock_quantity"].toInt();
        int threshold = item["reorder_threshold"].toInt();
        if (stock <= threshold) {
            low.append(item);
        }
    }
    return low;
}

void InventoryController::testConnection(const QString& targetUrl) {
    QString urlStr = targetUrl.trimmed();
    if (urlStr.isEmpty()) urlStr = m_serverUrl;

    setBusy(true);
    setConnectionStatus("Pinging " + urlStr + "...");

    QString healthUrl = urlStr;
    if (!healthUrl.endsWith("/health") && !healthUrl.endsWith("/api/v1/health")) {
        healthUrl = urlStr + "/health";
    }

    QUrl target(healthUrl);
    QNetworkRequest req(target);
    req.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);

    QNetworkReply* reply = m_netManager.get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply, urlStr]() {
        reply->deleteLater();
        setBusy(false);

        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QString service = doc.object().value("service").toString();
            QString version = doc.object().value("version").toString();
            QString redisInfo = doc.object().value("redis").toString();
            setConnectionStatus(QString("Connected to %1 (%2 v%3, Redis: %4)").arg(urlStr).arg(service).arg(version).arg(redisInfo));
            setServerOnline(true);
        } else {
            setConnectionStatus("Failed to connect: " + reply->errorString());
            setServerOnline(false);
        }
    });
}

void InventoryController::refreshData() {
    setBusy(true);
    setStatus("Fetching inventory supplies from clinic server...");

    QNetworkRequest req(QUrl(m_serverUrl + "/inventory"));
    QNetworkReply* reply = m_netManager.get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        setBusy(false);

        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            if (doc.isArray()) {
                m_catalog = doc.toVariant().toList();
                m_lowStockItems = filterLowStock(m_catalog);
                emit catalogChanged();
                emit lowStockItemsChanged();
                setServerOnline(true);
                setStatus(QString("Catalog loaded: %1 total items, %2 needing reorder.")
                    .arg(m_catalog.size()).arg(m_lowStockItems.size()));
            }
        } else {
            setServerOnline(false);
            setStatus("Server offline or unreachable: " + reply->errorString());
        }
    });
}

bool InventoryController::restockItem(int itemId, int quantity, const QString& reason) {
    if (itemId <= 0 || quantity == 0 || reason.trimmed().isEmpty()) {
        setStatus("Error: Invalid restock parameters. Quantity cannot be 0 and reason required.");
        return false;
    }

    setBusy(true);
    setStatus(QString("Applying restock (%1) for item ID %2...").arg(quantity).arg(itemId));

    QJsonObject payload;
    payload["quantity"] = quantity;
    payload["reason"] = reason.trimmed();

    QNetworkRequest req(QUrl(QString("%1/inventory/%2/restock").arg(m_serverUrl).arg(itemId)));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply = m_netManager.post(req, QJsonDocument(payload).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, reply, itemId]() {
        reply->deleteLater();
        setBusy(false);

        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            int newStock = doc.object().value("stock_quantity").toInt();
            QString name = doc.object().value("name").toString();

            setServerOnline(true);
            setStatus(QString("Restocked %1 successfully! New stock: %2").arg(name).arg(newStock));
            emit restockCompleted(itemId, newStock);
            refreshData(); // Refresh full catalog and low-stock list
        } else {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QString errStr = doc.object().value("error").toString();
            if (errStr.isEmpty()) errStr = reply->errorString();
            setStatus("Restock failed: " + errStr);
        }
    });

    return true;
}

bool InventoryController::createItem(const QString& sku, const QString& name, const QString& category,
                                     double unitPrice, int stock, int threshold, const QString& unit) {
    if (sku.trimmed().isEmpty() || name.trimmed().isEmpty() || unitPrice < 0.0 || stock < 0 || threshold < 0) {
        setStatus("Error: Invalid item parameters. Check SKU, Name, and positive numbers.");
        return false;
    }

    setBusy(true);
    setStatus("Adding new supply item to catalog...");

    QJsonObject payload;
    payload["sku"] = sku.trimmed();
    payload["name"] = name.trimmed();
    payload["category"] = category.trimmed().isEmpty() ? "General" : category.trimmed();
    payload["unit_price"] = unitPrice;
    payload["stock_quantity"] = stock;
    payload["reorder_threshold"] = threshold;
    payload["unit"] = unit.trimmed().isEmpty() ? "unit" : unit.trimmed();

    QNetworkRequest req(QUrl(m_serverUrl + "/inventory"));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply = m_netManager.post(req, QJsonDocument(payload).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, reply, sku]() {
        reply->deleteLater();
        setBusy(false);

        if (reply->error() == QNetworkReply::NoError) {
            setServerOnline(true);
            setStatus("Item " + sku + " created successfully!");
            emit itemCreated(sku);
            refreshData();
        } else {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QString errStr = doc.object().value("error").toString();
            if (errStr.isEmpty()) errStr = reply->errorString();
            setStatus("Creation failed: " + errStr);
        }
    });

    return true;
}

} // namespace aidans
