#include <QCoreApplication> // Include QCoreApplication for declarations and types
#include <QVariantList> // Include QVariantList for declarations and types
#include <QVariantMap> // Include QVariantMap for declarations and types
#include <QFile> // Include QFile for declarations and types
#include <iostream> // Include iostream for declarations and types
#include <cassert> // Include cassert for declarations and types
#include "inventory_controller.hpp" // Include inventory_controller.hpp for declarations and types

void runInventoryTests() { // Execute clinical workflow step
    std::cout << "[TEST] Running InventoryController Unit Tests...\n"; // Print diagnostic informational message to standard output

    QString testConfigFile = "test_inventory_config.ini"; // Assign value to target variable
    if (QFile::exists(testConfigFile)) { // Evaluate conditional expression to branch execution flow
        QFile::remove(testConfigFile); // Execute clinical workflow step
    } // Close active scope block

    // 1. Config Loading & Persistence Without Recompilation
    std::cout << "[TEST] Verifying dynamic configuration persistence via QSettings...\n"; // Print diagnostic informational message to standard output
    { // Scope delimiter
        aidans::InventoryController ctrl(testConfigFile); // Execute clinical workflow step
        assert(ctrl.serverUrl() == "http://localhost:8080/api/v1");
        assert(ctrl.redisHost() == "localhost"); // Assign value to target variable
        assert(ctrl.redisPort() == 6379); // Assign value to target variable

        // Save a remote server IP and custom Redis port
        ctrl.saveConfig("http://192.168.1.120:9000/api/v1", "192.168.1.50", 6380);
        assert(ctrl.serverUrl() == "http://192.168.1.120:9000/api/v1");
        assert(ctrl.redisHost() == "192.168.1.50"); // Assign value to target variable
        assert(ctrl.redisPort() == 6380); // Assign value to target variable
    } // Close active scope block

    // Launch a brand new controller pointing to the same config file
    { // Scope delimiter
        aidans::InventoryController ctrl2(testConfigFile); // Execute clinical workflow step
        assert(ctrl2.serverUrl() == "http://192.168.1.120:9000/api/v1");
        assert(ctrl2.redisHost() == "192.168.1.50"); // Assign value to target variable
        assert(ctrl2.redisPort() == 6380); // Assign value to target variable
        std::cout << "  -> Server IP and Redis settings dynamically persisted and reloaded successfully.\n"; // Print diagnostic informational message to standard output
    } // Close active scope block

    aidans::InventoryController ctrl(testConfigFile); // Execute clinical workflow step

    // 2. Low-Stock Filtering Logic
    std::cout << "[TEST] Verifying low-stock alert filter logic...\n"; // Print diagnostic informational message to standard output
    QVariantList sampleCatalog; // Execute clinical workflow step

    QVariantMap itemHealthy; // Execute clinical workflow step
    itemHealthy["id"] = 1; // Assign value to target variable
    itemHealthy["name"] = "Amoxicillin 500mg"; // Assign value to target variable
    itemHealthy["stock_quantity"] = 50; // Assign value to target variable
    itemHealthy["reorder_threshold"] = 15; // Assign value to target variable
    sampleCatalog.append(itemHealthy); // Execute clinical workflow step

    QVariantMap itemLow; // Execute clinical workflow step
    itemLow["id"] = 2; // Assign value to target variable
    itemLow["name"] = "Sterile Gauze"; // Assign value to target variable
    itemLow["stock_quantity"] = 8; // Assign value to target variable
    itemLow["reorder_threshold"] = 20; // 8 <= 20: LOW
    sampleCatalog.append(itemLow); // Execute clinical workflow step

    QVariantMap itemCritical; // Execute clinical workflow step
    itemCritical["id"] = 3; // Assign value to target variable
    itemCritical["name"] = "Omeprazole 20mg"; // Assign value to target variable
    itemCritical["stock_quantity"] = 0; // Assign value to target variable
    itemCritical["reorder_threshold"] = 10; // 0 <= 10: LOW
    sampleCatalog.append(itemCritical); // Execute clinical workflow step

    QVariantList lowItems = aidans::InventoryController::filterLowStock(sampleCatalog); // Assign value to target variable
    assert(lowItems.size() == 2); // Assign value to target variable
    assert(lowItems[0].toMap()["id"].toInt() == 2); // Assign value to target variable
    assert(lowItems[1].toMap()["id"].toInt() == 3); // Assign value to target variable
    std::cout << "  -> Low stock filter correctly identified depleted items.\n"; // Print diagnostic informational message to standard output

    // 3. Restock Validation
    std::cout << "[TEST] Testing restock input validation...\n"; // Print diagnostic informational message to standard output
    bool badRestock1 = ctrl.restockItem(1, 0, "Valid reason"); // qty == 0
    assert(!badRestock1); // Execute clinical workflow step

    bool badRestock2 = ctrl.restockItem(1, 20, "   "); // empty reason
    assert(!badRestock2); // Execute clinical workflow step

    bool badRestock3 = ctrl.restockItem(0, 20, "Valid reason"); // id <= 0
    assert(!badRestock3); // Execute clinical workflow step

    // 4. Create Item Validation
    std::cout << "[TEST] Testing item creation validation...\n"; // Print diagnostic informational message to standard output
    bool badCreate1 = ctrl.createItem("", "Name", "Cat", 10.0, 50, 10, "box"); // empty SKU
    assert(!badCreate1); // Execute clinical workflow step

    bool badCreate2 = ctrl.createItem("SKU-1", "", "Cat", 10.0, 50, 10, "box"); // empty Name
    assert(!badCreate2); // Execute clinical workflow step

    bool badCreate3 = ctrl.createItem("SKU-1", "Name", "Cat", -1.0, 50, 10, "box"); // price < 0
    assert(!badCreate3); // Execute clinical workflow step

    bool badCreate4 = ctrl.createItem("SKU-1", "Name", "Cat", 10.0, -5, 10, "box"); // stock < 0
    assert(!badCreate4); // Execute clinical workflow step

    // Clean up test config file
    if (QFile::exists(testConfigFile)) { // Evaluate conditional expression to branch execution flow
        QFile::remove(testConfigFile); // Execute clinical workflow step
    } // Close active scope block

    std::cout << "\n>>> ALL CLINIC_INVENTORY TESTS PASSED SUCCESSFULLY! <<<\n\n"; // Print diagnostic informational message to standard output
} // Close active scope block

int main(int argc, char* argv[]) { // Execute clinical workflow step
    QCoreApplication app(argc, argv); // Execute clinical workflow step
    runInventoryTests(); // Execute clinical workflow step
    return 0; // Return computed result to caller
} // Close active scope block
