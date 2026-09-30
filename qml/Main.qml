import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    visible: true
    width: 960
    height: 700
    title: "AIDANS - Inventory & Supplies Management"

    property int activeTab: 0 // 0: All Stock, 1: Low Stock Alerts, 2: New Item, 3: Settings
    property var selectedRestockItem: null

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        // Header & Navigation Bar
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            ColumnLayout {
                spacing: 2
                Label {
                    text: "Medical Inventory Management"
                    font.pixelSize: 20
                    font.bold: true
                }
                Label {
                    text: "Elizabeth's Clinic Frankfurt &bull; AIDANS"
                    font.pixelSize: 12
                    color: "#64748b"
                }
            }

            Item { Layout.fillWidth: true }

            // Online / Offline Indicator
            Rectangle {
                width: 10
                height: 10
                radius: 5
                color: controller.isServerOnline ? "#16a34a" : "#dc2626"
            }
            Label {
                text: controller.isServerOnline ? "Server Online" : "Server Offline"
                font.bold: true
                font.pixelSize: 12
                color: controller.isServerOnline ? "#16a34a" : "#dc2626"
            }

            Button {
                text: "Refresh"
                onClicked: controller.refreshData()
            }
        }

        // Navigation Tabs
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Button {
                text: "All Stock (" + controller.catalog.length + ")"
                highlighted: root.activeTab === 0
                onClicked: root.activeTab = 0
            }

            Button {
                text: "⚠ Low Stock Alerts (" + controller.lowStockItems.length + ")"
                highlighted: root.activeTab === 1
                onClicked: root.activeTab = 1
            }

            Button {
                text: "+ New Item"
                highlighted: root.activeTab === 2
                onClicked: root.activeTab = 2
            }

            Button {
                text: "⚙ Settings"
                highlighted: root.activeTab === 3
                onClicked: root.activeTab = 3
            }
        }

        // Content Views
        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: root.activeTab

            // TAB 0: All Stock View
            ColumnLayout {
                spacing: 8

                TextField {
                    id: searchInput
                    placeholderText: "Search item by name, SKU, or category..."
                    Layout.fillWidth: true
                }

                ListView {
                    id: catalogList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: controller.catalog
                    delegate: Rectangle {
                        width: catalogList.width
                        height: 52
                        color: index % 2 === 0 ? "#f8fafc" : "#ffffff"
                        border.color: modelData.stock_quantity <= modelData.reorder_threshold ? "#fed7aa" : "#e2e8f0"
                        radius: 4

                        visible: searchInput.text.trim().length === 0 ||
                                 modelData.name.toLowerCase().includes(searchInput.text.toLowerCase()) ||
                                 modelData.sku.toLowerCase().includes(searchInput.text.toLowerCase()) ||
                                 modelData.category.toLowerCase().includes(searchInput.text.toLowerCase())

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 12

                            ColumnLayout {
                                Layout.preferredWidth: 260
                                spacing: 2
                                Label {
                                    text: modelData.name
                                    font.bold: true
                                    elide: Text.ElideRight
                                    Layout.fillWidth: true
                                }
                                Label {
                                    text: modelData.sku + " &bull; " + modelData.category
                                    font.pixelSize: 11
                                    color: "#64748b"
                                }
                            }

                            Label {
                                text: "Stock: " + modelData.stock_quantity + " " + modelData.unit
                                font.bold: true
                                color: modelData.stock_quantity <= modelData.reorder_threshold ? "#dc2626" : "#0f172a"
                                Layout.preferredWidth: 120
                            }

                            Label {
                                text: "Min: " + modelData.reorder_threshold
                                font.pixelSize: 12
                                color: "#64748b"
                                Layout.preferredWidth: 80
                            }

                            Label {
                                text: Number(modelData.unit_price).toFixed(2) + " €"
                                Layout.preferredWidth: 80
                            }

                            // Low Stock Badge
                            Rectangle {
                                width: 80
                                height: 24
                                radius: 4
                                color: modelData.stock_quantity <= modelData.reorder_threshold ? "#fee2e2" : "#dcfce7"
                                visible: true

                                Label {
                                    anchors.centerIn: parent
                                    text: modelData.stock_quantity <= modelData.reorder_threshold ? "LOW STOCK" : "IN STOCK"
                                    font.pixelSize: 10
                                    font.bold: true
                                    color: modelData.stock_quantity <= modelData.reorder_threshold ? "#dc2626" : "#16a34a"
                                }
                            }

                            Item { Layout.fillWidth: true }

                            Button {
                                text: "+ Restock"
                                highlighted: modelData.stock_quantity <= modelData.reorder_threshold
                                onClicked: {
                                    root.selectedRestockItem = modelData;
                                    restockQtySpin.value = 20;
                                    restockReasonInput.text = "Batch replenishment";
                                    restockPopup.open();
                                }
                            }
                        }
                    }
                }
            }

            // TAB 1: Low Stock Alerts View
            ColumnLayout {
                spacing: 8

                Rectangle {
                    Layout.fillWidth: true
                    height: 40
                    color: "#fffbeb"
                    border.color: "#fde68a"
                    radius: 4

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        Label {
                            text: "⚠ Attention: The following supplies are at or below their reorder threshold!"
                            font.bold: true
                            color: "#b45309"
                        }
                    }
                }

                ListView {
                    id: lowList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: controller.lowStockItems
                    delegate: Rectangle {
                        width: lowList.width
                        height: 56
                        color: "#fff7ed"
                        border.color: "#fdba74"
                        radius: 4

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 12

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 2
                                Label {
                                    text: modelData.name
                                    font.bold: true
                                    color: "#9a3412"
                                }
                                Label {
                                    text: "SKU: " + modelData.sku + " &bull; Category: " + modelData.category
                                    font.pixelSize: 11
                                    color: "#78350f"
                                }
                            }

                            Label {
                                text: "Current: " + modelData.stock_quantity + " / Threshold: " + modelData.reorder_threshold
                                font.bold: true
                                color: "#dc2626"
                                Layout.preferredWidth: 200
                            }

                            Button {
                                text: "+ Restock Item"
                                highlighted: true
                                onClicked: {
                                    root.selectedRestockItem = modelData;
                                    restockQtySpin.value = modelData.reorder_threshold * 2;
                                    restockReasonInput.text = "Urgent low stock reorder";
                                    restockPopup.open();
                                }
                            }
                        }
                    }
                }
            }

            // TAB 2: Add New Item
            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true

                ColumnLayout {
                    width: parent.width
                    spacing: 12

                    Label {
                        text: "Add New Supply Item to Catalog"
                        font.bold: true
                        font.pixelSize: 16
                    }

                    TextField {
                        id: newSku
                        placeholderText: "SKU Code (e.g. MED-AMOX-250)"
                        Layout.fillWidth: true
                    }

                    TextField {
                        id: newName
                        placeholderText: "Item / Drug Name (e.g. Amoxicillin 250mg)"
                        Layout.fillWidth: true
                    }

                    TextField {
                        id: newCategory
                        placeholderText: "Category (e.g. Antibiotics, Analgesic, Supplies)"
                        Layout.fillWidth: true
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        ColumnLayout {
                            Layout.fillWidth: true
                            Label { text: "Unit Price (EUR):" }
                            TextField {
                                id: newPrice
                                Layout.fillWidth: true
                                text: "9.99"
                                validator: DoubleValidator { bottom: 0.01; decimals: 2 }
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            Label { text: "Initial Stock:" }
                            SpinBox {
                                id: newStock
                                Layout.fillWidth: true
                                from: 0
                                to: 10000
                                value: 50
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            Label { text: "Reorder Threshold:" }
                            SpinBox {
                                id: newThreshold
                                Layout.fillWidth: true
                                from: 1
                                to: 500
                                value: 15
                            }
                        }
                    }

                    TextField {
                        id: newUnit
                        placeholderText: "Unit of Measure (e.g. box, bottle, roll, ampoule)"
                        Layout.fillWidth: true
                    }

                    Button {
                        text: "Create Item"
                        highlighted: true
                        Layout.fillWidth: true
                        onClicked: {
                            if (newSku.text.trim().length > 0 && newName.text.trim().length > 0) {
                                controller.createItem(
                                    newSku.text,
                                    newName.text,
                                    newCategory.text,
                                    parseFloat(newPrice.text) || 0.0,
                                    newStock.value,
                                    newThreshold.value,
                                    newUnit.text
                                );
                                newSku.text = "";
                                newName.text = "";
                                newCategory.text = "";
                                root.activeTab = 0;
                            }
                        }
                    }
                }
            }

            // TAB 3: Settings Page
            ColumnLayout {
                spacing: 16

                Label {
                    text: "Application & Server Configuration"
                    font.bold: true
                    font.pixelSize: 18
                }

                Label {
                    text: "Change the backend server IP/URL without recompiling the application. Persisted to config.ini."
                    color: "#64748b"
                    font.pixelSize: 13
                }

                GroupBox {
                    title: "Clinic Server Endpoint"
                    Layout.fillWidth: true

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 10

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            TextField {
                                id: serverUrlInput
                                text: controller.serverUrl
                                placeholderText: "http://<server-ip>:8080/api/v1"
                                Layout.fillWidth: true
                            }

                            Button {
                                text: "Test Connection"
                                onClicked: controller.testConnection(serverUrlInput.text)
                            }
                        }

                        Label {
                            text: controller.connectionStatus.length > 0 ? controller.connectionStatus : "Enter server URL and click Test Connection."
                            font.pixelSize: 12
                            color: controller.connectionStatus.includes("Connected") ? "#16a34a" : "#dc2626"
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12

                    Button {
                        text: "Save & Apply Settings"
                        highlighted: true
                        onClicked: {
                            controller.saveServerUrl(serverUrlInput.text);
                            controller.refreshData();
                            root.activeTab = 0;
                        }
                    }

                    Button {
                        text: "Reset to Default (localhost)"
                        onClicked: {
                            serverUrlInput.text = "http://localhost:8080/api/v1";
                            controller.saveServerUrl(serverUrlInput.text);
                            controller.refreshData();
                        }
                    }
                }

                Item { Layout.fillHeight: true }
            }
        }

        // Status Bar
        Rectangle {
            Layout.fillWidth: true
            height: 32
            color: "#f1f5f9"
            border.color: "#cbd5e1"
            radius: 4

            Label {
                anchors.centerIn: parent
                text: controller.statusMessage.length > 0 ? controller.statusMessage : "Ready"
                font.pixelSize: 12
            }
        }
    }

    // Restock Popup Dialog
    Popup {
        id: restockPopup
        anchors.centerIn: parent
        width: 420
        height: 300
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        contentItem: ColumnLayout {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 12

            Label {
                text: "Restock Supply Item"
                font.bold: true
                font.pixelSize: 16
            }

            Label {
                text: root.selectedRestockItem ? (root.selectedRestockItem.name + " (" + root.selectedRestockItem.sku + ")") : ""
                font.bold: true
                color: "#1e293b"
            }

            Label {
                text: root.selectedRestockItem ? ("Current Stock: " + root.selectedRestockItem.stock_quantity + " " + root.selectedRestockItem.unit) : ""
                color: "#64748b"
                font.pixelSize: 12
            }

            RowLayout {
                spacing: 8
                Label { text: "Quantity to add:" }
                SpinBox {
                    id: restockQtySpin
                    from: 1
                    to: 10000
                    value: 20
                }
            }

            TextField {
                id: restockReasonInput
                placeholderText: "Reason / Delivery batch reference..."
                Layout.fillWidth: true
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Button {
                    text: "Cancel"
                    onClicked: restockPopup.close()
                }

                Item { Layout.fillWidth: true }

                Button {
                    text: "Confirm Restock"
                    highlighted: true
                    onClicked: {
                        if (root.selectedRestockItem && restockReasonInput.text.trim().length > 0) {
                            controller.restockItem(root.selectedRestockItem.id, restockQtySpin.value, restockReasonInput.text);
                            restockPopup.close();
                        }
                    }
                }
            }
        }
    }

    // SERVER OFFLINE ERROR OVERLAY
    Rectangle {
        id: offlineOverlay
        anchors.fill: parent
        color: Qt.rgba(0, 0, 0, 0.6)
        visible: !controller.isServerOnline && root.activeTab !== 3
        z: 999

        Rectangle {
            anchors.centerIn: parent
            width: 480
            height: 280
            radius: 8
            color: "#ffffff"
            border.color: "#f87171"
            border.width: 2

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 24
                spacing: 14

                Label {
                    text: "⚠ Server Offline or Unreachable"
                    font.bold: true
                    font.pixelSize: 18
                    color: "#dc2626"
                    Layout.alignment: Qt.AlignHCenter
                }

                Label {
                    text: "The clinic server appears to be offline or unreachable at:\n" + controller.serverUrl
                    horizontalAlignment: Text.AlignHCenter
                    color: "#475569"
                    font.pixelSize: 13
                    Layout.alignment: Qt.AlignHCenter
                }

                Label {
                    text: "Please verify that clinic_server is running, or reconfigure the server IP in Settings."
                    font.pixelSize: 12
                    color: "#64748b"
                    horizontalAlignment: Text.AlignHCenter
                    Layout.alignment: Qt.AlignHCenter
                }

                Item { Layout.fillHeight: true }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12

                    Button {
                        text: "⚙ Configure Server IP"
                        Layout.fillWidth: true
                        onClicked: {
                            root.activeTab = 3; // Switch to settings
                        }
                    }

                    Button {
                        text: "🔄 Retry Connection"
                        highlighted: true
                        Layout.fillWidth: true
                        onClicked: controller.refreshData()
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        controller.refreshData();
    }
}
