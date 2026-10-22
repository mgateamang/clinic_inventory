import QtQuick // Import module dependency for runtime execution
import QtQuick.Controls // Import module dependency for runtime execution
import QtQuick.Layouts // Import module dependency for runtime execution

ApplicationWindow { // Execute clinical workflow step
    id: root // Execute clinical workflow step
    visible: true // Execute clinical workflow step
    width: 960 // Execute clinical workflow step
    height: 700 // Execute clinical workflow step
    title: "AIDANS - Inventory & Supplies Management" // Execute clinical workflow step

    property int activeTab: 0 // 0: All Stock, 1: Low Stock Alerts, 2: New Item, 3: Settings
    property var selectedRestockItem: null // Execute clinical workflow step

    ColumnLayout { // Execute clinical workflow step
        anchors.fill: parent // Execute clinical workflow step
        anchors.margins: 16 // Execute clinical workflow step
        spacing: 12 // Execute clinical workflow step

        // Header & Navigation Bar
        RowLayout { // Execute clinical workflow step
            Layout.fillWidth: true // Execute clinical workflow step
            spacing: 12 // Execute clinical workflow step

            Label { // Execute clinical workflow step
                text: "Medical Inventory Management" // Execute clinical workflow step
                font.pixelSize: 20 // Execute clinical workflow step
                font.bold: true // Execute clinical workflow step
            } // Close active scope block

            Item { Layout.fillWidth: true } // Execute clinical workflow step

            Rectangle { // Execute clinical workflow step
                width: 10 // Execute clinical workflow step
                height: 10 // Execute clinical workflow step
                radius: 5 // Execute clinical workflow step
                color: controller.isServerOnline ? "#16a34a" : "#dc2626" // Execute clinical workflow step
            } // Close active scope block
            Label { // Execute clinical workflow step
                text: controller.isServerOnline ? "Online" : "Offline" // Execute clinical workflow step
                font.bold: true // Execute clinical workflow step
                font.pixelSize: 12 // Execute clinical workflow step
                color: controller.isServerOnline ? "#16a34a" : "#dc2626" // Execute clinical workflow step
            } // Close active scope block

            Button { // Execute clinical workflow step
                text: "Refresh" // Execute clinical workflow step
                onClicked: controller.refreshData() // Execute clinical workflow step
            } // Close active scope block
        } // Close active scope block

        // Navigation Tabs
        RowLayout { // Execute clinical workflow step
            Layout.fillWidth: true // Execute clinical workflow step
            spacing: 8 // Execute clinical workflow step

            Button { // Execute clinical workflow step
                text: "All Stock (" + controller.catalog.length + ")" // Execute clinical workflow step
                highlighted: root.activeTab === 0 // Assign value to target variable
                onClicked: root.activeTab = 0 // Assign value to target variable
            } // Close active scope block

            Button { // Execute clinical workflow step
                text: "⚠ Low Stock Alerts (" + controller.lowStockItems.length + ")" // Execute clinical workflow step
                highlighted: root.activeTab === 1 // Assign value to target variable
                onClicked: root.activeTab = 1 // Assign value to target variable
            } // Close active scope block

            Button { // Execute clinical workflow step
                text: "+ New Item" // Execute clinical workflow step
                highlighted: root.activeTab === 2 // Assign value to target variable
                onClicked: root.activeTab = 2 // Assign value to target variable
            } // Close active scope block

            Button { // Execute clinical workflow step
                text: "⚙ Settings" // Execute clinical workflow step
                highlighted: root.activeTab === 3 // Assign value to target variable
                onClicked: root.activeTab = 3 // Assign value to target variable
            } // Close active scope block
        } // Close active scope block

        // Content Views
        StackLayout { // Execute clinical workflow step
            Layout.fillWidth: true // Execute clinical workflow step
            Layout.fillHeight: true // Execute clinical workflow step
            currentIndex: root.activeTab // Execute clinical workflow step

            // TAB 0: All Stock View
            ColumnLayout { // Execute clinical workflow step
                spacing: 8 // Execute clinical workflow step

                TextField { // Execute clinical workflow step
                    id: searchInput // Execute clinical workflow step
                    placeholderText: "Search item by name, SKU, or category..." // Execute clinical workflow step
                    Layout.fillWidth: true // Execute clinical workflow step
                } // Close active scope block

                ListView { // Execute clinical workflow step
                    id: catalogList // Execute clinical workflow step
                    Layout.fillWidth: true // Execute clinical workflow step
                    Layout.fillHeight: true // Execute clinical workflow step
                    clip: true // Execute clinical workflow step
                    model: controller.catalog // Execute clinical workflow step
                    delegate: Rectangle { // Execute clinical workflow step
                        width: catalogList.width // Execute clinical workflow step
                        height: 52 // Execute clinical workflow step
                        color: index % 2 === 0 ? "#f8fafc" : "#ffffff" // Assign value to target variable
                        border.color: modelData.stock_quantity <= modelData.reorder_threshold ? "#fed7aa" : "#e2e8f0" // Assign value to target variable
                        radius: 4 // Execute clinical workflow step

                        visible: searchInput.text.trim().length === 0 || // Assign value to target variable
                                 modelData.name.toLowerCase().includes(searchInput.text.toLowerCase()) || // Execute clinical workflow step
                                 modelData.sku.toLowerCase().includes(searchInput.text.toLowerCase()) || // Execute clinical workflow step
                                 modelData.category.toLowerCase().includes(searchInput.text.toLowerCase()) // Execute clinical workflow step

                        RowLayout { // Execute clinical workflow step
                            anchors.fill: parent // Execute clinical workflow step
                            anchors.margins: 10 // Execute clinical workflow step
                            spacing: 12 // Execute clinical workflow step

                            ColumnLayout { // Execute clinical workflow step
                                Layout.preferredWidth: 260 // Execute clinical workflow step
                                spacing: 2 // Execute clinical workflow step
                                Label { // Execute clinical workflow step
                                    text: modelData.name // Execute clinical workflow step
                                    font.bold: true // Execute clinical workflow step
                                    elide: Text.ElideRight // Execute clinical workflow step
                                    Layout.fillWidth: true // Execute clinical workflow step
                                } // Close active scope block
                                Label { // Execute clinical workflow step
                                    text: modelData.sku + " &bull; " + modelData.category // Execute clinical workflow step
                                    font.pixelSize: 11 // Execute clinical workflow step
                                    color: "#64748b" // Execute clinical workflow step
                                } // Close active scope block
                            } // Close active scope block

                            Label { // Execute clinical workflow step
                                text: "Stock: " + modelData.stock_quantity + " " + modelData.unit // Execute clinical workflow step
                                font.bold: true // Execute clinical workflow step
                                color: modelData.stock_quantity <= modelData.reorder_threshold ? "#dc2626" : "#0f172a" // Assign value to target variable
                                Layout.preferredWidth: 120 // Execute clinical workflow step
                            } // Close active scope block

                            Label { // Execute clinical workflow step
                                text: "Min: " + modelData.reorder_threshold // Execute clinical workflow step
                                font.pixelSize: 12 // Execute clinical workflow step
                                color: "#64748b" // Execute clinical workflow step
                                Layout.preferredWidth: 80 // Execute clinical workflow step
                            } // Close active scope block

                            Label { // Execute clinical workflow step
                                text: Number(modelData.unit_price).toFixed(2) + " €" // Execute clinical workflow step
                                Layout.preferredWidth: 80 // Execute clinical workflow step
                            } // Close active scope block

                            Item { Layout.fillWidth: true } // Execute clinical workflow step

                            Button { // Execute clinical workflow step
                                text: "+ Restock" // Execute clinical workflow step
                                highlighted: modelData.stock_quantity <= modelData.reorder_threshold // Assign value to target variable
                                onClicked: { // Execute clinical workflow step
                                    root.selectedRestockItem = modelData; // Assign value to target variable
                                    restockQtySpin.value = 20; // Assign value to target variable
                                    restockReasonInput.text = "Batch replenishment"; // Assign value to target variable
                                    restockPopup.open(); // Execute clinical workflow step
                                } // Close active scope block
                            } // Close active scope block
                        } // Close active scope block
                    } // Close active scope block
                } // Close active scope block
            } // Close active scope block

            // TAB 1: Low Stock Alerts View
            ColumnLayout { // Execute clinical workflow step
                spacing: 8 // Execute clinical workflow step

                ListView { // Execute clinical workflow step
                    id: lowList // Execute clinical workflow step
                    Layout.fillWidth: true // Execute clinical workflow step
                    Layout.fillHeight: true // Execute clinical workflow step
                    clip: true // Execute clinical workflow step
                    model: controller.lowStockItems // Execute clinical workflow step
                    delegate: Rectangle { // Execute clinical workflow step
                        width: lowList.width // Execute clinical workflow step
                        height: 56 // Execute clinical workflow step
                        color: "#fff7ed" // Execute clinical workflow step
                        border.color: "#fdba74" // Execute clinical workflow step
                        radius: 4 // Execute clinical workflow step

                        RowLayout { // Execute clinical workflow step
                            anchors.fill: parent // Execute clinical workflow step
                            anchors.margins: 10 // Execute clinical workflow step
                            spacing: 12 // Execute clinical workflow step

                            Label { // Execute clinical workflow step
                                text: modelData.name + " (" + modelData.sku + ")" // Execute clinical workflow step
                                font.bold: true // Execute clinical workflow step
                                color: "#b91c1c" // Execute clinical workflow step
                                Layout.fillWidth: true // Execute clinical workflow step
                                elide: Text.ElideRight // Execute clinical workflow step
                            } // Close active scope block

                            Label { // Execute clinical workflow step
                                text: "Stock: " + modelData.stock_quantity + " (Min: " + modelData.reorder_threshold + ")" // Execute clinical workflow step
                                font.bold: true // Execute clinical workflow step
                                color: "#dc2626" // Execute clinical workflow step
                                Layout.preferredWidth: 160 // Execute clinical workflow step
                            } // Close active scope block

                            Button { // Execute clinical workflow step
                                text: "+ Restock" // Execute clinical workflow step
                                highlighted: true // Execute clinical workflow step
                                onClicked: { // Execute clinical workflow step
                                    root.selectedRestockItem = modelData; // Assign value to target variable
                                    restockQtySpin.value = modelData.reorder_threshold * 2; // Assign value to target variable
                                    restockReasonInput.text = "Urgent low stock reorder"; // Assign value to target variable
                                    restockPopup.open(); // Execute clinical workflow step
                                } // Close active scope block
                            } // Close active scope block
                        } // Close active scope block
                    } // Close active scope block
                } // Close active scope block
            } // Close active scope block

            // TAB 2: Add New Item
            ScrollView { // Execute clinical workflow step
                Layout.fillWidth: true // Execute clinical workflow step
                Layout.fillHeight: true // Execute clinical workflow step

                ColumnLayout { // Execute clinical workflow step
                    width: parent.width // Execute clinical workflow step
                    spacing: 12 // Execute clinical workflow step

                    Label { // Execute clinical workflow step
                        text: "Add New Supply Item to Catalog" // Execute clinical workflow step
                        font.bold: true // Execute clinical workflow step
                        font.pixelSize: 16 // Execute clinical workflow step
                    } // Close active scope block

                    TextField { // Execute clinical workflow step
                        id: newSku // Execute clinical workflow step
                        placeholderText: "SKU Code (e.g. MED-AMOX-250)" // Execute clinical workflow step
                        Layout.fillWidth: true // Execute clinical workflow step
                    } // Close active scope block

                    TextField { // Execute clinical workflow step
                        id: newName // Execute clinical workflow step
                        placeholderText: "Item / Drug Name (e.g. Amoxicillin 250mg)" // Execute clinical workflow step
                        Layout.fillWidth: true // Execute clinical workflow step
                    } // Close active scope block

                    TextField { // Execute clinical workflow step
                        id: newCategory // Execute clinical workflow step
                        placeholderText: "Category (e.g. Antibiotics, Analgesic, Supplies)" // Execute clinical workflow step
                        Layout.fillWidth: true // Execute clinical workflow step
                    } // Close active scope block

                    RowLayout { // Execute clinical workflow step
                        Layout.fillWidth: true // Execute clinical workflow step
                        spacing: 12 // Execute clinical workflow step

                        ColumnLayout { // Execute clinical workflow step
                            Layout.fillWidth: true // Execute clinical workflow step
                            Label { text: "Unit Price (EUR):" } // Execute clinical workflow step
                            TextField { // Execute clinical workflow step
                                id: newPrice // Execute clinical workflow step
                                Layout.fillWidth: true // Execute clinical workflow step
                                text: "9.99" // Execute clinical workflow step
                                validator: DoubleValidator { bottom: 0.01; decimals: 2 } // Execute clinical workflow step
                            } // Close active scope block
                        } // Close active scope block

                        ColumnLayout { // Execute clinical workflow step
                            Layout.fillWidth: true // Execute clinical workflow step
                            Label { text: "Initial Stock:" } // Execute clinical workflow step
                            SpinBox { // Execute clinical workflow step
                                id: newStock // Execute clinical workflow step
                                Layout.fillWidth: true // Execute clinical workflow step
                                from: 0 // Execute clinical workflow step
                                to: 10000 // Execute clinical workflow step
                                value: 50 // Execute clinical workflow step
                            } // Close active scope block
                        } // Close active scope block

                        ColumnLayout { // Execute clinical workflow step
                            Layout.fillWidth: true // Execute clinical workflow step
                            Label { text: "Reorder Threshold:" } // Execute clinical workflow step
                            SpinBox { // Execute clinical workflow step
                                id: newThreshold // Execute clinical workflow step
                                Layout.fillWidth: true // Execute clinical workflow step
                                from: 1 // Execute clinical workflow step
                                to: 500 // Execute clinical workflow step
                                value: 15 // Execute clinical workflow step
                            } // Close active scope block
                        } // Close active scope block
                    } // Close active scope block

                    TextField { // Execute clinical workflow step
                        id: newUnit // Execute clinical workflow step
                        placeholderText: "Unit of Measure (e.g. box, bottle, roll, ampoule)" // Execute clinical workflow step
                        Layout.fillWidth: true // Execute clinical workflow step
                    } // Close active scope block

                    Button { // Execute clinical workflow step
                        text: "Create Item" // Execute clinical workflow step
                        highlighted: true // Execute clinical workflow step
                        Layout.fillWidth: true // Execute clinical workflow step
                        onClicked: { // Execute clinical workflow step
                            if (newSku.text.trim().length > 0 && newName.text.trim().length > 0) { // Evaluate conditional expression to branch execution flow
                                controller.createItem( // Execute clinical workflow step
                                    newSku.text, // Execute clinical workflow step
                                    newName.text, // Execute clinical workflow step
                                    newCategory.text, // Execute clinical workflow step
                                    parseFloat(newPrice.text) || 0.0, // Execute clinical workflow step
                                    newStock.value, // Execute clinical workflow step
                                    newThreshold.value, // Execute clinical workflow step
                                    newUnit.text // Execute clinical workflow step
                                ); // Execute clinical workflow step
                                newSku.text = ""; // Assign value to target variable
                                newName.text = ""; // Assign value to target variable
                                newCategory.text = ""; // Assign value to target variable
                                root.activeTab = 0; // Assign value to target variable
                            } // Close active scope block
                        } // Close active scope block
                    } // Close active scope block
                } // Close active scope block
            } // Close active scope block

            // TAB 3: Settings Page
            ColumnLayout { // Execute clinical workflow step
                spacing: 16 // Execute clinical workflow step

                Label { // Execute clinical workflow step
                    text: "Application & Server Configuration" // Execute clinical workflow step
                    font.bold: true // Execute clinical workflow step
                    font.pixelSize: 18 // Execute clinical workflow step
                } // Close active scope block

                Label { // Execute clinical workflow step
                    text: "Change the backend server IP/URL without recompiling the application. Persisted to config.ini." // Execute clinical workflow step
                    color: "#64748b" // Execute clinical workflow step
                    font.pixelSize: 13 // Execute clinical workflow step
                } // Close active scope block

                GroupBox { // Execute clinical workflow step
                    title: "Clinic Server Endpoint" // Execute clinical workflow step
                    Layout.fillWidth: true // Execute clinical workflow step

                    ColumnLayout { // Execute clinical workflow step
                        anchors.fill: parent // Execute clinical workflow step
                        spacing: 10 // Execute clinical workflow step

                        RowLayout { // Execute clinical workflow step
                            Layout.fillWidth: true // Execute clinical workflow step
                            spacing: 8 // Execute clinical workflow step

                            TextField { // Execute clinical workflow step
                                id: serverUrlInput // Execute clinical workflow step
                                text: controller.serverUrl // Execute clinical workflow step
                                placeholderText: "http://<server-ip>:8080/api/v1"
                                Layout.fillWidth: true // Execute clinical workflow step
                            } // Close active scope block

                            Button { // Execute clinical workflow step
                                text: "Test Connection" // Execute clinical workflow step
                                onClicked: controller.testConnection(serverUrlInput.text) // Execute clinical workflow step
                            } // Close active scope block
                        } // Close active scope block

                        Label { // Execute clinical workflow step
                            text: controller.connectionStatus.length > 0 ? controller.connectionStatus : "Enter server URL and click Test Connection." // Execute clinical workflow step
                            font.pixelSize: 12 // Execute clinical workflow step
                            color: controller.connectionStatus.includes("Connected") ? "#16a34a" : "#dc2626" // Execute clinical workflow step
                        } // Close active scope block
                    } // Close active scope block
                } // Close active scope block

                RowLayout { // Execute clinical workflow step
                    Layout.fillWidth: true // Execute clinical workflow step
                    spacing: 12 // Execute clinical workflow step

                    Button { // Execute clinical workflow step
                        text: "Save & Apply Settings" // Execute clinical workflow step
                        highlighted: true // Execute clinical workflow step
                        onClicked: { // Execute clinical workflow step
                            controller.saveServerUrl(serverUrlInput.text); // Execute clinical workflow step
                            controller.refreshData(); // Execute clinical workflow step
                            root.activeTab = 0; // Assign value to target variable
                        } // Close active scope block
                    } // Close active scope block

                    Button { // Execute clinical workflow step
                        text: "Reset to Default" // Execute clinical workflow step
                        onClicked: { // Execute clinical workflow step
                            serverUrlInput.text = "http://localhost:8080/api/v1";
                            controller.saveServerUrl(serverUrlInput.text); // Execute clinical workflow step
                            controller.refreshData(); // Execute clinical workflow step
                        } // Close active scope block
                    } // Close active scope block
                } // Close active scope block

                Item { Layout.fillHeight: true } // Execute clinical workflow step
            } // Close active scope block
        } // Close active scope block

        // Status Bar
        Rectangle { // Execute clinical workflow step
            Layout.fillWidth: true // Execute clinical workflow step
            height: 32 // Execute clinical workflow step
            color: "#f1f5f9" // Execute clinical workflow step
            border.color: "#cbd5e1" // Execute clinical workflow step
            radius: 4 // Execute clinical workflow step

            Label { // Execute clinical workflow step
                anchors.centerIn: parent // Execute clinical workflow step
                text: controller.statusMessage.length > 0 ? controller.statusMessage : "Ready" // Execute clinical workflow step
                font.pixelSize: 12 // Execute clinical workflow step
            } // Close active scope block
        } // Close active scope block
    } // Close active scope block

    // Restock Popup Dialog
    Popup { // Execute clinical workflow step
        id: restockPopup // Execute clinical workflow step
        anchors.centerIn: parent // Execute clinical workflow step
        width: 420 // Execute clinical workflow step
        height: 300 // Execute clinical workflow step
        modal: true // Execute clinical workflow step
        focus: true // Execute clinical workflow step
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside // Execute clinical workflow step

        contentItem: ColumnLayout { // Execute clinical workflow step
            anchors.fill: parent // Execute clinical workflow step
            anchors.margins: 16 // Execute clinical workflow step
            spacing: 12 // Execute clinical workflow step

            Label { // Execute clinical workflow step
                text: "Restock Supply Item" // Execute clinical workflow step
                font.bold: true // Execute clinical workflow step
                font.pixelSize: 16 // Execute clinical workflow step
            } // Close active scope block

            Label { // Execute clinical workflow step
                text: root.selectedRestockItem ? (root.selectedRestockItem.name + " (" + root.selectedRestockItem.sku + ")") : "" // Execute clinical workflow step
                font.bold: true // Execute clinical workflow step
                color: "#1e293b" // Execute clinical workflow step
            } // Close active scope block

            Label { // Execute clinical workflow step
                text: root.selectedRestockItem ? ("Current Stock: " + root.selectedRestockItem.stock_quantity + " " + root.selectedRestockItem.unit) : "" // Execute clinical workflow step
                color: "#64748b" // Execute clinical workflow step
                font.pixelSize: 12 // Execute clinical workflow step
            } // Close active scope block

            RowLayout { // Execute clinical workflow step
                spacing: 8 // Execute clinical workflow step
                Label { text: "Quantity to add:" } // Execute clinical workflow step
                SpinBox { // Execute clinical workflow step
                    id: restockQtySpin // Execute clinical workflow step
                    from: 1 // Execute clinical workflow step
                    to: 10000 // Execute clinical workflow step
                    value: 20 // Execute clinical workflow step
                } // Close active scope block
            } // Close active scope block

            TextField { // Execute clinical workflow step
                id: restockReasonInput // Execute clinical workflow step
                placeholderText: "Reason / Delivery batch reference..." // Execute clinical workflow step
                Layout.fillWidth: true // Execute clinical workflow step
            } // Close active scope block

            RowLayout { // Execute clinical workflow step
                Layout.fillWidth: true // Execute clinical workflow step
                spacing: 12 // Execute clinical workflow step

                Button { // Execute clinical workflow step
                    text: "Cancel" // Execute clinical workflow step
                    onClicked: restockPopup.close() // Execute clinical workflow step
                } // Close active scope block

                Item { Layout.fillWidth: true } // Execute clinical workflow step

                Button { // Execute clinical workflow step
                    text: "Confirm Restock" // Execute clinical workflow step
                    highlighted: true // Execute clinical workflow step
                    onClicked: { // Execute clinical workflow step
                        if (root.selectedRestockItem && restockReasonInput.text.trim().length > 0) { // Evaluate conditional expression to branch execution flow
                            controller.restockItem(root.selectedRestockItem.id, restockQtySpin.value, restockReasonInput.text); // Execute clinical workflow step
                            restockPopup.close(); // Execute clinical workflow step
                        } // Close active scope block
                    } // Close active scope block
                } // Close active scope block
            } // Close active scope block
        } // Close active scope block
    } // Close active scope block

    Component.onCompleted: { // Execute clinical workflow step
        controller.refreshData(); // Execute clinical workflow step
    } // Close active scope block
} // Close active scope block
