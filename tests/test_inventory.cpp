#include <QCoreApplication>
#include <QVariantList>
#include <QVariantMap>
#include <QFile>
#include <iostream>
#include <cassert>
#include "inventory_controller.hpp"

void runInventoryTests() {
    std::cout << "[TEST] Running InventoryController Unit Tests...\n";

    QString testConfigFile = "test_inventory_config.ini";
    if (QFile::exists(testConfigFile)) {
        QFile::remove(testConfigFile);
    }

    // 1. Config Loading & Persistence Without Recompilation
    std::cout << "[TEST] Verifying dynamic configuration persistence via QSettings...\n";
    {
        aidans::InventoryController ctrl(testConfigFile);
        assert(ctrl.serverUrl() == "http://localhost:8080/api/v1");

        // Save a remote server IP
        ctrl.saveServerUrl("http://192.168.1.120:9000/api/v1");
        assert(ctrl.serverUrl() == "http://192.168.1.120:9000/api/v1");
    }

    // Launch a brand new controller pointing to the same config file
    {
        aidans::InventoryController ctrl2(testConfigFile);
        assert(ctrl2.serverUrl() == "http://192.168.1.120:9000/api/v1");
        std::cout << "  -> Server IP dynamically persisted and reloaded successfully.\n";
    }

    aidans::InventoryController ctrl(testConfigFile);

    // 2. Low-Stock Filtering Logic
    std::cout << "[TEST] Verifying low-stock alert filter logic...\n";
    QVariantList sampleCatalog;

    QVariantMap itemHealthy;
    itemHealthy["id"] = 1;
    itemHealthy["name"] = "Amoxicillin 500mg";
    itemHealthy["stock_quantity"] = 50;
    itemHealthy["reorder_threshold"] = 15;
    sampleCatalog.append(itemHealthy);

    QVariantMap itemLow;
    itemLow["id"] = 2;
    itemLow["name"] = "Sterile Gauze";
    itemLow["stock_quantity"] = 8;
    itemLow["reorder_threshold"] = 20; // 8 <= 20: LOW
    sampleCatalog.append(itemLow);

    QVariantMap itemCritical;
    itemCritical["id"] = 3;
    itemCritical["name"] = "Omeprazole 20mg";
    itemCritical["stock_quantity"] = 0;
    itemCritical["reorder_threshold"] = 10; // 0 <= 10: LOW
    sampleCatalog.append(itemCritical);

    QVariantList lowItems = aidans::InventoryController::filterLowStock(sampleCatalog);
    assert(lowItems.size() == 2);
    assert(lowItems[0].toMap()["id"].toInt() == 2);
    assert(lowItems[1].toMap()["id"].toInt() == 3);
    std::cout << "  -> Low stock filter correctly identified depleted items.\n";

    // 3. Restock Validation
    std::cout << "[TEST] Testing restock input validation...\n";
    bool badRestock1 = ctrl.restockItem(1, 0, "Valid reason"); // qty == 0
    assert(!badRestock1);

    bool badRestock2 = ctrl.restockItem(1, 20, "   "); // empty reason
    assert(!badRestock2);

    bool badRestock3 = ctrl.restockItem(0, 20, "Valid reason"); // id <= 0
    assert(!badRestock3);

    // 4. Create Item Validation
    std::cout << "[TEST] Testing item creation validation...\n";
    bool badCreate1 = ctrl.createItem("", "Name", "Cat", 10.0, 50, 10, "box"); // empty SKU
    assert(!badCreate1);

    bool badCreate2 = ctrl.createItem("SKU-1", "", "Cat", 10.0, 50, 10, "box"); // empty Name
    assert(!badCreate2);

    bool badCreate3 = ctrl.createItem("SKU-1", "Name", "Cat", -1.0, 50, 10, "box"); // price < 0
    assert(!badCreate3);

    bool badCreate4 = ctrl.createItem("SKU-1", "Name", "Cat", 10.0, -5, 10, "box"); // stock < 0
    assert(!badCreate4);

    // Clean up test config file
    if (QFile::exists(testConfigFile)) {
        QFile::remove(testConfigFile);
    }

    std::cout << "\n>>> ALL CLINIC_INVENTORY TESTS PASSED SUCCESSFULLY! <<<\n\n";
}

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    runInventoryTests();
    return 0;
}
