#include "orderpage.h"
#include "ui_orderpage.h"
#include <QAbstractItemView>
#include <QPainter>
#include <QMouseEvent>
#include <QApplication>
#include <QStyle>

// 操作按钮委托实现
OrderButtonDelegate::OrderButtonDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void OrderButtonDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    if (index.column() != 5) { // 不是操作列
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    painter->save();


    // 计算按钮区域 - 调整为更紧凑的布局
    QRect rect = option.rect;
    int buttonWidth = 55;
    int buttonHeight = 24;
    int spacing = 5;
    int totalWidth = buttonWidth * 2 + spacing;
    int startX = rect.left() + (rect.width() - totalWidth) / 2;
    int buttonY = rect.top() + (rect.height() - buttonHeight) / 2;

    // 绘制"查看详情"按钮
    QRect detailButtonRect(startX, buttonY, buttonWidth, buttonHeight);
    QLinearGradient detailGradient(detailButtonRect.topLeft(), detailButtonRect.bottomLeft());
    detailGradient.setColorAt(0, QColor(184, 164, 230));
    detailGradient.setColorAt(1, QColor(161, 140, 209));
    painter->fillRect(detailButtonRect, detailGradient);
    painter->setPen(QPen(QColor(161, 140, 209), 1));
    painter->drawRoundedRect(detailButtonRect, 3, 3);
    painter->setPen(Qt::white);
    painter->setFont(QFont("微软雅黑", 7, QFont::Bold));
    painter->drawText(detailButtonRect, Qt::AlignCenter, "查看详情");

    // 绘制"修改状态"按钮
    QRect statusButtonRect(startX + buttonWidth + spacing, buttonY, buttonWidth, buttonHeight);
    QLinearGradient statusGradient(statusButtonRect.topLeft(), statusButtonRect.bottomLeft());
    statusGradient.setColorAt(0, QColor(139, 120, 196));
    statusGradient.setColorAt(1, QColor(106, 91, 123));
    painter->fillRect(statusButtonRect, statusGradient);
    painter->setPen(QPen(QColor(106, 91, 123), 1));
    painter->drawRoundedRect(statusButtonRect, 3, 3);
    painter->setPen(Qt::white);
    painter->drawText(statusButtonRect, Qt::AlignCenter, "修改状态");

    painter->restore();
}

bool OrderButtonDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
                                      const QStyleOptionViewItem &option,
                                      const QModelIndex &index)
{
    if (index.column() != 5) { // 不是操作列
        return QStyledItemDelegate::editorEvent(event, model, option, index);
    }

    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

        // 计算按钮区域
        QRect rect = option.rect;
        int buttonWidth = 55;
        int buttonHeight = 24;
        int spacing = 5;
        int totalWidth = buttonWidth * 2 + spacing;
        int startX = rect.left() + (rect.width() - totalWidth) / 2;
        int buttonY = rect.top() + (rect.height() - buttonHeight) / 2;

        // 查看详情按钮
        QRect detailButtonRect(startX, buttonY, buttonWidth, buttonHeight);
        if (detailButtonRect.contains(mouseEvent->pos())) {
            emit detailClicked(index.row());
            return true;
        }

        // 修改状态按钮
        QRect statusButtonRect(startX + buttonWidth + spacing, buttonY, buttonWidth, buttonHeight);
        if (statusButtonRect.contains(mouseEvent->pos())) {
            emit statusClicked(index.row());
            return true;
        }
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

QSize OrderButtonDelegate::sizeHint(const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const
{
    if (index.column() == 5) { // 操作列
        return QSize(120, 35); // 紧凑的操作列宽度
    }
    return QStyledItemDelegate::sizeHint(option, index);
}

// OrderPage 实现
OrderPage::OrderPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OrderPage),
    isCached(false),
    totalOrders(0)
{
    ui->setupUi(this);

    // 初始化操作委托
    buttonDelegate = new OrderButtonDelegate(this);

    setupUI();
}

OrderPage::~OrderPage()
{
    delete ui;
}

void OrderPage::setupUI()
{
    // 设置初始统计数据
    ui->countLabel->setText("0");
    ui->selectionLabel->setText("已选择 0 项");
}

void OrderPage::setupModel()
{
    // 创建标准模型用于显示订单数据
    orderModel = new QStandardItemModel(this);

    // 设置表头
    QStringList headers;
    headers << "订单ID" << "用户ID" << "订单金额" << "订单状态" << "创建时间" << "操作";
    orderModel->setHorizontalHeaderLabels(headers);

    ui->tableView->setModel(orderModel);

    // 设置操作列的委托
    ui->tableView->setItemDelegateForColumn(5, buttonDelegate);

    // 设置表格属性
    ui->tableView->setAlternatingRowColors(true);
    ui->tableView->setSortingEnabled(true);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);

    // 设置行高
    ui->tableView->verticalHeader()->setDefaultSectionSize(40);

    // 重要：设置表格填充整个空间
    ui->tableView->horizontalHeader()->setStretchLastSection(false);

    // 按比例设置列宽，确保填满整个表格
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // 为特定列设置固定宽度，其余列自动拉伸
    ui->tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents); // 订单ID
    ui->tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed); // 用户ID
    ui->tableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed); // 金额
    ui->tableView->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed); // 状态
    ui->tableView->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch); // 时间列拉伸
    ui->tableView->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Fixed); // 操作列固定

    // 设置固定列的具体宽度
    ui->tableView->setColumnWidth(1, 70);  // 用户ID列
    ui->tableView->setColumnWidth(2, 90);  // 金额列
    ui->tableView->setColumnWidth(3, 100); // 状态列
    ui->tableView->setColumnWidth(5, 120); // 操作列

    // 设置最小列宽
    ui->tableView->horizontalHeader()->setMinimumSectionSize(50);

    // 隐藏行号
    ui->tableView->verticalHeader()->setVisible(false);

    // 设置表格样式 - 确保填满容器
    ui->tableView->setStyleSheet(
        "QTableView {"
        "    gridline-color: rgba(214, 201, 247, 0.5);"
        "    background: transparent;"
        "    border: none;"
        "    selection-background-color: rgba(161, 140, 209, 0.2);"
        "}"
        "QTableView::item {"
        "    padding: 6px 8px;"
        "    border-bottom: 1px solid rgba(214, 201, 247, 0.3);"
        "    background: rgba(255, 255, 255, 0.9);"
        "}"
        "QTableView::item:selected {"
        "    background: rgba(161, 140, 209, 0.25);"
        "    color: #5a4c74;"
        "}"
        "QTableView::item:hover {"
        "    background: rgba(214, 201, 247, 0.4);"
        "}"
        "QTableView::item:alternate {"
        "    background: rgba(248, 245, 255, 0.7);"
        "}"
        "QHeaderView::section {"
        "    background: qlineargradient(spread:pad,x1:0,y1:0,x2:0,y2:1,"
        "                stop:0 rgba(184, 164, 230, 0.9),"
        "                stop:1 rgba(161, 140, 209, 0.9));"
        "    color: white;"
        "    padding: 8px;"
        "    border: none;"
        "    border-right: 1px solid rgba(255, 255, 255, 0.3);"
        "    font: 600 10pt '微软雅黑';"
        "}"
        "QHeaderView::section:first {"
        "    border-top-left-radius: 8px;"
        "}"
        "QHeaderView::section:last {"
        "    border-top-right-radius: 8px;"
        "    border-right: none;"
        "}"
        );

    // 确保表格水平滚动条在需要时显示
    ui->tableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->tableView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}

void OrderPage::setupConnections()
{
    // 连接选择变化
    connect(ui->tableView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &OrderPage::onSelectionChanged);

    // 连接操作按钮点击事件
    connect(buttonDelegate, &OrderButtonDelegate::detailClicked,
            this, &OrderPage::onDetailClicked);
    connect(buttonDelegate, &OrderButtonDelegate::statusClicked,
            this, &OrderPage::onStatusClicked);
}

void OrderPage::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    if (!isCached) {
        setupModel();
        setupConnections();
        loadOrderData();
        isCached = true;
    }
}

QString OrderPage::formatDateTime(const QString &dateTimeStr)
{
    // 解析数据库中的时间格式，转换为更友好的显示格式
    QDateTime dateTime = QDateTime::fromString(dateTimeStr, "yyyy-MM-ddThh:mm:ss.000Z");
    if (!dateTime.isValid()) {
        // 尝试其他格式
        dateTime = QDateTime::fromString(dateTimeStr, "yyyy-MM-dd hh:mm:ss");
    }
    if (!dateTime.isValid()) {
        // 如果还是无效，返回原字符串，但只保留日期部分
        if (dateTimeStr.length() >= 10) {
            return dateTimeStr.left(10); // 只显示日期
        }
        return dateTimeStr;
    }

    // 转换为本地时间并格式化为紧凑格式
    dateTime = dateTime.toLocalTime();
    return dateTime.toString("MM-dd hh:mm"); // 更紧凑的时间格式
}

void OrderPage::loadOrderData()
{
    if (!manager->getConnected()) {
        QMessageBox::warning(this, "连接错误", "请先连接到服务器");
        return;
    }

    // 构建请求
    QJsonObject body;
    body.insert("want", "*");
    body.insert("restriction", "1=1");

    qDebug() << "发送订单查询请求:" << body;

    // 发送请求到服务器
    QByteArray response = manager->sendCHTTPMsg("20404", body);

    if (response.isEmpty()) {
        QMessageBox::warning(this, "网络错误", "请求订单数据失败");
        return;
    }

    // 解析响应
    QString head = manager->parseHead(response);
    qDebug() << "收到响应头:" << head;

    if (head.startsWith("1")) {
        // 成功响应
        QJsonArray orders = manager->parseResponse(response);
        qDebug() << "解析到的订单数据:" << orders;
        updateOrderDisplay(orders);
    } else if (head.startsWith("2")) {
        manager->error('2', head.mid(1));
    } else if (head.startsWith("3")) {
        manager->error('3', head.mid(1));
    }
}

void OrderPage::updateOrderDisplay(const QJsonArray &orders)
{
    // 清空现有数据
    orderModel->clear();

    // 重新设置表头
    QStringList headers;
    headers << "订单ID" << "用户ID" << "订单金额" << "订单状态" << "创建时间" << "操作";
    orderModel->setHorizontalHeaderLabels(headers);

    // 存储原始订单数据
    originalOrdersData = orders;

    qDebug() << "更新订单显示，收到" << orders.size() << "条订单数据";

    // 填充数据
    for (int i = 0; i < orders.size(); ++i) {
        QJsonObject order = orders[i].toObject();

        qDebug() << "处理订单" << i << "，数据:" << order;

        QList<QStandardItem*> row;

        // 订单ID
        QString fullOrderId = order["Order_id"].toString();
        if (fullOrderId.isEmpty()) {
            qDebug() << "警告：订单ID为空！";
            continue;
        }

        QString displayOrderId = fullOrderId;
        if (fullOrderId.length() > 12) {
            displayOrderId = fullOrderId.left(8) + "..." + fullOrderId.right(4);
        }
        QStandardItem *orderIdItem = new QStandardItem(displayOrderId);
        orderIdItem->setFont(QFont("微软雅黑", 8, QFont::Bold));
        orderIdItem->setForeground(QColor(106, 91, 123));
        orderIdItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        orderIdItem->setToolTip(fullOrderId);
        orderIdItem->setData(i, Qt::UserRole);
        row << orderIdItem;

        // 用户ID - 修复字符串转换问题
        QString userIdStr = order["Order_user_id"].toString();
        int userId = userIdStr.toInt();
        qDebug() << "用户ID字符串:" << userIdStr << "转换后:" << userId;

        QStandardItem *userIdItem = new QStandardItem(QString::number(userId));
        userIdItem->setTextAlignment(Qt::AlignCenter);
        userIdItem->setFont(QFont("微软雅黑", 8));
        row << userIdItem;

        // 订单金额 - 修复字符串转换问题
        QString priceStr = order["Order_tolprice"].toString();
        int price = priceStr.toInt();
        qDebug() << "订单金额字符串:" << priceStr << "转换后:" << price;

        QStandardItem *priceItem = new QStandardItem(QString("¥%1").arg(price));
        priceItem->setTextAlignment(Qt::AlignCenter);
        priceItem->setFont(QFont("微软雅黑", 8, QFont::Bold));
        priceItem->setForeground(QColor(161, 140, 209));
        row << priceItem;

        // 订单状态
        QString status = order["Order_status"].toString();
        qDebug() << "订单状态:" << status;
        QStandardItem *statusItem = new QStandardItem();
        statusItem->setTextAlignment(Qt::AlignCenter);
        statusItem->setFont(QFont("微软雅黑", 8, QFont::Bold));

        if (status == "待付款") {
            statusItem->setText("🕐 待付款");
            statusItem->setForeground(QColor(255, 165, 0));
        } else if (status == "已付款") {
            statusItem->setText("💰 已付款");
            statusItem->setForeground(QColor(0, 123, 255));
        } else if (status == "已发货") {
            statusItem->setText("🚚 已发货");
            statusItem->setForeground(QColor(128, 0, 128));
        } else if (status == "已完成") {
            statusItem->setText("✅ 已完成");
            statusItem->setForeground(QColor(40, 167, 69));
        } else if (status == "已取消") {
            statusItem->setText("❌ 已取消");
            statusItem->setForeground(QColor(220, 53, 69));
        } else if (status == "申请退货") {
            statusItem->setText("🔄 申请退货");
            statusItem->setForeground(QColor(255, 140, 0));
        } else {
            statusItem->setText(status.isEmpty() ? "未知" : status);
            statusItem->setForeground(QColor(106, 91, 123));
        }
        row << statusItem;

        // 创建时间
        QString timeStr = order["Order_time"].toString();
        qDebug() << "创建时间:" << timeStr;
        QString formattedTime = formatDateTime(timeStr);
        QStandardItem *timeItem = new QStandardItem(formattedTime);
        timeItem->setTextAlignment(Qt::AlignCenter);
        timeItem->setFont(QFont("微软雅黑", 8));
        timeItem->setForeground(QColor(106, 91, 123));
        timeItem->setToolTip(timeStr);
        row << timeItem;

        // 操作列
        QStandardItem *actionItem = new QStandardItem("");
        actionItem->setEditable(false);
        row << actionItem;

        orderModel->appendRow(row);
    }

    totalOrders = orders.size();
    updateOrderCount();
    updateSelectionCount();

    qDebug() << "订单显示更新完成，总计" << totalOrders << "条订单";
}


void OrderPage::updateOrderCount()
{
    ui->countLabel->setText(QString::number(totalOrders));
}

void OrderPage::updateSelectionCount()
{
    int selectedCount = ui->tableView->selectionModel()->selectedRows().count();
    ui->selectionLabel->setText(QString("已选择 %1 项").arg(selectedCount));
}

void OrderPage::onSelectionChanged()
{
    updateSelectionCount();
}

void OrderPage::onDetailClicked(int row)
{
    showOrderDetails(row);
}

void OrderPage::onStatusClicked(int row)
{
    changeOrderStatus(row);
}

void OrderPage::showOrderDetails(int row)
{
    if (row < 0 || row >= orderModel->rowCount()) return;

    // 从原始数据中获取完整的订单信息
    int dataIndex = orderModel->item(row, 0)->data(Qt::UserRole).toInt();
    if (dataIndex < 0 || dataIndex >= originalOrdersData.size()) {
        QMessageBox::warning(this, "错误", "无法获取订单数据");
        return;
    }

    QJsonObject orderData = originalOrdersData[dataIndex].toObject();
    QString orderId = orderData["Order_id"].toString();

    int userId = orderData["Order_user_id"].toString().toInt();
    int priceInt = orderData["Order_tolprice"].toString().toInt();
    QString amount = QString("¥%1").arg(priceInt);

    QString status = orderData["Order_status"].toString();
    QString createTime = orderData["Order_time"].toString();

    qDebug() << "查询订单详情，订单ID:" << orderId;

    if (!manager->getConnected()) {
        QMessageBox::warning(this, "连接错误", "请先连接到服务器");
        return;
    }

    // 构建查询订单详情的请求
    QJsonObject body;
    body.insert("want", "orderitems.Orderitem_id,orderitems.Orderitem_pro_id,products.Product_name,orderitems.Orderitem_num,orderitems.Orderitem_pro_price,orderitems.Orderitem_time");
    body.insert("restriction", QString("orderitems.Orderitem_order_id = '%1' AND orderitems.Orderitem_pro_id = products.Product_id").arg(orderId));

    // 使用新的协议码 20410
    QByteArray response = manager->sendCHTTPMsg("20410", body);

    if (response.isEmpty()) {
        QMessageBox::warning(this, "网络错误", "获取订单详情失败");
        return;
    }

    QString head = manager->parseHead(response);
    if (head.startsWith("1")) {
        QJsonArray orderItems = manager->parseResponse(response);

        // 使用新的自定义对话框，传入manager参数
        OrderDetailDialog *dialog = new OrderDetailDialog(orderId, userId, amount, status, createTime, orderItems, manager, this);
        dialog->exec();
        dialog->deleteLater();

    } else if (head.startsWith("2")) {
        manager->error('2', head.mid(1));
    } else if (head.startsWith("3")) {
        manager->error('3', head.mid(1));
    }
}


QString OrderPage::buildOrderDetailsText(const QString &orderId, int userId, const QString &amount,
                                         const QString &status, const QString &createTime,
                                         const QJsonArray &orderItems)
{
    QString details = QString("📋 订单详情\n\n");
    details += QString("🆔 订单ID: %1\n").arg(orderId);
    details += QString("👤 买家ID: %2\n").arg(userId);
    details += QString("📅 下单时间: %3\n").arg(createTime);
    details += QString("💰 订单金额: %4\n").arg(amount);
    details += QString("📊 订单状态: %5\n\n").arg(status);

    if (!orderItems.isEmpty()) {
        details += QString("🛒 购买商品 (共%1件商品):\n").arg(orderItems.size());
        details += "─────────────────────────\n";

        double calculatedTotal = 0;
        for (int i = 0; i < orderItems.size(); ++i) {
            QJsonObject item = orderItems[i].toObject();
            QString productName = item["Product_name"].toString();

            // 修复字符串转换问题
            int quantity = item["Orderitem_num"].toString().toInt();
            int price = item["Orderitem_pro_price"].toString().toInt();

            double itemTotal = price * quantity;
            calculatedTotal += itemTotal;

            details += QString("%1. %2\n").arg(i+1).arg(productName);
            details += QString("   数量: %1 件 | 单价: ¥%2 | 小计: ¥%3\n\n")
                           .arg(quantity)
                           .arg(price)
                           .arg(itemTotal);
        }

        details += "─────────────────────────\n";
        details += QString("🧮 计算总金额: ¥%1\n").arg(calculatedTotal);
        details += QString("💰 订单总金额: %2").arg(amount);

        // 检查金额是否一致
        QString amountCopy = amount;
        double orderAmount = amountCopy.remove("¥").toDouble();
        if (qAbs(calculatedTotal - orderAmount) > 0.01) {
            details += "\n⚠️ 注意：计算金额与订单金额不符！";
        }
    } else {
        details += "📦 该订单没有商品信息\n";
    }

    return details;
}



void OrderPage::refreshOrder()
{
    loadOrderData();
    QMessageBox::information(this, "提示", "订单数据已刷新");
}


void OrderPage::changeOrderStatus(int row)
{
    if (row < 0 || row >= orderModel->rowCount()) return;

    // 从原始数据中获取完整的订单信息
    int dataIndex = orderModel->item(row, 0)->data(Qt::UserRole).toInt();
    if (dataIndex < 0 || dataIndex >= originalOrdersData.size()) {
        QMessageBox::warning(this, "错误", "无法获取订单数据");
        return;
    }

    QJsonObject orderData = originalOrdersData[dataIndex].toObject();
    QString orderId = orderData["Order_id"].toString();
    QString currentStatusText = orderModel->item(row, 3)->text();

    // 提取状态文本（去掉图标）
    QString currentStatus;
    if (currentStatusText.contains("待付款")) currentStatus = "待付款";
    else if (currentStatusText.contains("已付款")) currentStatus = "已付款";
    else if (currentStatusText.contains("已发货")) currentStatus = "已发货";
    else if (currentStatusText.contains("已完成")) currentStatus = "已完成";
    else if (currentStatusText.contains("已取消")) currentStatus = "已取消";
    else if (currentStatusText.contains("申请退货")) currentStatus = "申请退货";
    else if (currentStatusText.contains("退货中")) currentStatus = "退货中";
    else if (currentStatusText.contains("已退货")) currentStatus = "已退货";
    else currentStatus = currentStatusText;

    qDebug() << "准备修改订单状态，订单ID:" << orderId << "当前状态:" << currentStatus;

    // 状态选项
    QStringList statusOptions;
    statusOptions << "待付款" << "已付款" << "已发货" << "已完成" << "已取消" << "申请退货" << "退货中" << "已退货";

    bool ok;
    QString newStatus = QInputDialog::getItem(this,
                                              QString("修改订单状态"),
                                              QString("订单ID: %1\n当前状态: %2\n\n请选择新状态:")
                                                  .arg(orderId, currentStatus),
                                              statusOptions,
                                              statusOptions.indexOf(currentStatus),
                                              false,
                                              &ok);

    if (ok && newStatus != currentStatus) {
        // 确认修改
        int ret = QMessageBox::question(this, "确认修改",
                                        QString("确定要将订单状态\n从 \"%1\" 修改为 \"%2\" 吗？")
                                            .arg(currentStatus, newStatus),
                                        QMessageBox::Yes | QMessageBox::No);

        if (ret == QMessageBox::Yes) {
            // 发送状态更新请求到服务器
            updateOrderStatusOnServer(orderId, newStatus, row);
        }
    }
}

void OrderPage::updateOrderStatusOnServer(const QString &orderId, const QString &newStatus, int row)
{
    if (!manager->getConnected()) {
        QMessageBox::warning(this, "连接错误", "请先连接到服务器");
        return;
    }

    // 构建请求
    QJsonObject body;
    body.insert("orderId", orderId);
    body.insert("newStatus", newStatus);

    qDebug() << "发送状态更新请求:" << body;

    // 发送请求到服务器 (协议码 20411)
    QByteArray response = manager->sendCHTTPMsg("20411", body);

    if (response.isEmpty()) {
        QMessageBox::warning(this, "网络错误", "修改订单状态失败");
        return;
    }

    QString head = manager->parseHead(response);
    if (head.startsWith("1")) {
        // 更新成功，更新本地显示
        updateOrderStatusDisplay(row, newStatus);

        // 同时更新原始数据
        int dataIndex = orderModel->item(row, 0)->data(Qt::UserRole).toInt();
        if (dataIndex >= 0 && dataIndex < originalOrdersData.size()) {
            QJsonObject updatedOrder = originalOrdersData[dataIndex].toObject();
            updatedOrder["Order_status"] = newStatus;
            originalOrdersData[dataIndex] = updatedOrder;
        }

        QMessageBox::information(this, "修改成功",
                                 QString("订单 %1 状态已成功修改为: %2")
                                     .arg(orderId, newStatus));
    } else if (head.startsWith("2")) {
        manager->error('2', head.mid(1));
    } else if (head.startsWith("3")) {
        manager->error('3', head.mid(1));
    }
}

void OrderPage::updateOrderStatusDisplay(int row, const QString &newStatus)
{
    QStandardItem *statusItem = orderModel->item(row, 3);

    if (newStatus == "待付款") {
        statusItem->setText("🕐 待付款");
        statusItem->setForeground(QColor(255, 165, 0));
    } else if (newStatus == "已付款") {
        statusItem->setText("💰 已付款");
        statusItem->setForeground(QColor(0, 123, 255));
    } else if (newStatus == "已发货") {
        statusItem->setText("🚚 已发货");
        statusItem->setForeground(QColor(128, 0, 128));
    } else if (newStatus == "已完成") {
        statusItem->setText("✅ 已完成");
        statusItem->setForeground(QColor(40, 167, 69));
    } else if (newStatus == "已取消") {
        statusItem->setText("❌ 已取消");
        statusItem->setForeground(QColor(220, 53, 69));
    } else if (newStatus == "申请退货") {
        statusItem->setText("🔄 申请退货");
        statusItem->setForeground(QColor(255, 140, 0));
    } else if (newStatus == "退货中") {
        statusItem->setText("📦 退货中");
        statusItem->setForeground(QColor(108, 117, 125));
    } else if (newStatus == "已退货") {
        statusItem->setText("↩️ 已退货");
        statusItem->setForeground(QColor(75, 0, 130));
    }
}
