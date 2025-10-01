#include "orderpage.h"
#include "ui_orderpage.h"

// 在构造函数中添加布局优化
OrderPage::OrderPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OrderPage),
    btn_pay(nullptr)
{
    ui->setupUi(this);
    this->current = 0;

    // 优化布局设置
    optimizeLayout();

    connect(ui->btn_back, SIGNAL(clicked()), this, SLOT(backOrder()));
    connect(ui->btn_next, SIGNAL(clicked()), this, SLOT(nextOrder()));
}

// 添加布局优化方法
void OrderPage::optimizeLayout() {
    // 设置列表控件的最小高度，确保有足够空间显示内容
    ui->listWidget->setMinimumHeight(300);

    // 调整底部布局的间距和边距
    if (ui->horizontalLayout_2) {
        ui->horizontalLayout_2->setSpacing(15);
        ui->horizontalLayout_2->setContentsMargins(20, 10, 20, 10);
    }

    // 确保标签有足够的空间显示文本
    if (ui->lab_order) {
        ui->lab_order->setMinimumWidth(200);
        ui->lab_order->setWordWrap(true);  // 允许文字换行
    }

    if (ui->lab_time) {
        ui->lab_time->setMinimumWidth(150);
    }

    if (ui->lab_price) {
        ui->lab_price->setMinimumWidth(100);
    }

    // 为整个OrderPage设置更好的样式
    this->setStyleSheet(R"(
        QListWidget {
            background: rgba(255, 255, 255, 180);
            border: none;
            border-radius: 0px;
            padding: 10px;
            font: 12pt "Microsoft YaHei";
            color: rgb(80, 70, 95);
            outline: none;
        }

        QListWidget::item {
            background: transparent;
            border: none;
            padding: 5px;
            margin: 3px 0px;
            min-height: 90px;
        }

        QLabel {
            font: 13pt "Microsoft YaHei";
            color: rgb(110, 100, 130);
            background: transparent;
            padding: 5px;
        }

        /* 特别优化订单号显示 */
        #lab_order {
            font: 12pt "Microsoft YaHei";
            color: rgb(189, 170, 233);
            background: rgba(255, 255, 255, 160);
            padding: 6px 12px;
            border-radius: 14px;
            border: 1px solid rgba(189, 170, 233, 40);
            word-wrap: break-word;
            max-width: 300px;
        }
    )");
}

OrderPage::~OrderPage()
{
    delete ui;
}


void OrderPage::showEvent(QShowEvent* event) 
{
	qDebug() << "OrderPage showEvent triggered";
	getAllOrder();
	qDebug() << "Order num after getAllOrder:" << orderlist.size();

	if (orderlist.isEmpty()) {
		qDebug() << "No orders found for current user";
		// 可以在UI上显示"暂无订单"的提示
		ui->lab_status->setText("暂无订单");
		ui->lab_price->setText("0");
		ui->lab_time->setText("");
		ui->lab_order->setText("");
	}
}

void OrderPage::getAllOrder() {
    orderlist.clear();
    QString FLAG_INSKIND = "04";
    QString FLAG_INS = "05";  // 使用联合查询
    QJsonObject obj;

    int userId = client->getPerson()->getId();
    qDebug() << "Current user ID:" << userId;

    // 查询有订单项的订单
    obj.insert("want", "DISTINCT orders.Order_id,orders.Order_user_id,orders.Order_tolprice,orders.Order_status,orders.Order_time");
    obj.insert("restriction", QString("orders.Order_id = orderitems.Orderitem_order_id AND orders.Order_user_id = %1").arg(userId));

    QByteArray data = client->sendCHTTPMsg(FLAG_CHARACTER + FLAG_INSKIND + FLAG_INS, obj);
    QString flag = client->parseHead(data);

    if (flag[0] != "1") {
        client->error(flag[0], flag.mid(1));
        return;
    }

    QJsonArray result = client->parseResponse(data);
    qDebug() << "Query result size:" << result.size();

    if (!result.isEmpty()) {
        // 创建一个有序的订单列表，按时间排序
        QList<QJsonObject> sortedOrders;

        for (int i = 0; i < result.size(); i++) {
            QJsonObject orderObj = result[i].toObject();
            sortedOrders.append(orderObj);
        }

        // 按时间排序（最新的在前）
        std::sort(sortedOrders.begin(), sortedOrders.end(), [](const QJsonObject &a, const QJsonObject &b) {
            QString timeA = a.value("Order_time").toString();
            QString timeB = b.value("Order_time").toString();
            return timeA > timeB;  // 降序排列，最新的在前
        });

        // 重新填充orderlist，确保顺序正确
        orderlist.clear();
        for (const QJsonObject &orderObj : sortedOrders) {
            QString orderId = orderObj.value("Order_id").toString();
            orderlist.insert(orderId, orderObj);
            qDebug() << "Added order to list:" << orderId << "Time:" << orderObj.value("Order_time").toString();
        }

        if (!orderlist.isEmpty()) {
            current = 0;
            QStringList keys = orderlist.keys();
            QString latestOrderId = keys.first();  // 第一个就是最新的
            qDebug() << "Showing latest order with items:" << latestOrderId;
            updateOrderItems(latestOrderId);
        }
    }
    else {
        qDebug() << "No orders with items found";
        ui->listWidget->clear();
        ui->lab_status->setText("暂无订单");
        ui->lab_price->setText("0");
        ui->lab_time->setText("");
        ui->lab_order->setText("");
    }
}

void OrderPage::updateOrderItems(QString ordernum) {
    if (ordernum.isEmpty()) {
        qDebug() << "Error: ordernum is empty!";
        return;
    }

    qDebug() << "Updating order items for order:" << ordernum;

    QString FLAG_INSKIND = "04";
    QString FLAG_INS = "05";
    QJsonObject obj;

    // 修正字段名和表名，确保与数据库一致
    obj.insert("want", "Product_pictureaddress AS pro_pictureaddress,Product_name AS pro_name,Orderitem_num AS orderitem_num,Orderitem_pro_price AS orderitem_pro_price,Orderitem_num*Orderitem_pro_price AS orderitem_tolprice");

    // 修正JOIN条件，使用正确的字段名
    obj.insert("restriction", QString("products.Product_id = orderitems.Orderitem_pro_id AND orderitems.Orderitem_order_id = orders.Order_id AND orders.Order_id = '%1'").arg(ordernum));

    QByteArray data = client->sendCHTTPMsg(FLAG_CHARACTER + FLAG_INSKIND + FLAG_INS, obj);
    QString flag = client->parseHead(data);
    if (flag[0] != "1") {
        client->error(flag[0], flag.mid(1));
        return;
    }

    ui->listWidget->clear();
    QJsonArray result = client->parseResponse(data);
    qDebug() << "orderitems:" << result.size();

    for (int i = 0; i < result.size(); i++) {
        QJsonObject order = result[i].toObject();
        OrderItem* newitem = new OrderItem(order);
        addItem(newitem);
    }

    QJsonObject order = orderlist.value(ordernum);
    QString status = order.value("Order_status").toString();
    ui->lab_status->setText(status);
    ui->lab_price->setText(order.value("Order_tolprice").toString());

    // 修复时间显示格式
    QString rawTime = order.value("Order_time").toString();
    QString formattedTime = formatOrderTime(rawTime);
    ui->lab_time->setText(formattedTime);

    // 确保显示正确的订单号
    ui->lab_order->setText(ordernum);

    qDebug() << "Updated UI with order:" << ordernum << "Status:" << status;


    // 根据订单状态显示/隐藏付款按钮
    if (status == "待付款") {
        if (!btn_pay) {  // 注意这里改为 btn_pay，不是 ui->btn_pay
            // 创建付款按钮（如果不存在）
            btn_pay = new QPushButton("立即付款", this);
            btn_pay->setStyleSheet(R"(
                QPushButton {
                    background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1,
                                stop:0 rgba(255, 140, 0, 255),
                                stop:0.5 rgba(255, 165, 0, 255),
                                stop:1 rgba(255, 215, 0, 255));
                    border: 1px solid rgba(255, 255, 255, 90);
                    border-radius: 18px;
                    font: bold 12pt "Microsoft YaHei";
                    color: white;
                    padding: 8px 15px;
                    min-width: 100px;
                    min-height: 35px;
                }
                QPushButton:hover {
                    background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1,
                                stop:0 rgba(255, 165, 0, 255),
                                stop:0.5 rgba(255, 215, 0, 255),
                                stop:1 rgba(255, 255, 0, 255));
                }
            )");

            // 添加到布局中
            ui->horizontalLayout_2->insertWidget(2, btn_pay);

            // 连接付款信号
            connect(btn_pay, &QPushButton::clicked, this, &OrderPage::payCurrentOrder);
        }
        btn_pay->show();
    }
    else {
        if (btn_pay) {  // 注意这里也改为 btn_pay
            btn_pay->hide();
        }
    }
}

QString OrderPage::formatOrderTime(const QString& rawTime) {
    // 处理不同的时间格式
    QDateTime dateTime;

    // 尝试解析ISO格式 (如: 2025-09-30T23:12:47.000Z)
    if (rawTime.contains('T')) {
        dateTime = QDateTime::fromString(rawTime, Qt::ISODate);
        if (dateTime.isValid()) {
            // 转换为本地时间
            dateTime = dateTime.toLocalTime();
        }
    }

    // 如果ISO格式解析失败，尝试MySQL标准格式
    if (!dateTime.isValid()) {
        dateTime = QDateTime::fromString(rawTime, "yyyy-MM-dd hh:mm:ss");
    }

    // 如果还是无效，尝试其他常见格式
    if (!dateTime.isValid()) {
        dateTime = QDateTime::fromString(rawTime, "yyyy-MM-dd");
    }

    // 返回格式化的时间字符串
    if (dateTime.isValid()) {
        return dateTime.toString("yyyy-MM-dd hh:mm:ss");
    } else {
        qDebug() << "Failed to parse time:" << rawTime;
        return rawTime; // 解析失败时返回原始字符串
    }
}


void OrderPage::addItem(OrderItem *item){
    QListWidgetItem *newWidget = new QListWidgetItem(ui->listWidget);
    ui->listWidget->setItemWidget(newWidget,item);
}

void OrderPage::backOrder() {
    if (orderlist.isEmpty()) {
        qDebug() << "No orders available for navigation";
        return;
    }

    current = (current + orderlist.size() - 1) % orderlist.size();
    QStringList keys = orderlist.keys();
    QString orderKey = keys.at(current);
    qDebug() << "Navigate to previous order:" << orderKey << "at index:" << current;
    updateOrderItems(orderKey);
}

void OrderPage::nextOrder()
{
    if (orderlist.isEmpty()) {
        qDebug() << "No orders available for navigation";
        return;
    }

    current = (current + 1) % orderlist.size();
    QStringList keys = orderlist.keys();
    QString orderKey = keys.at(current);
    qDebug() << "Navigate to next order:" << orderKey << "at index:" << current;
    updateOrderItems(orderKey);
}

void OrderPage::payCurrentOrder() {
    if (orderlist.isEmpty()) return;

    QStringList keys = orderlist.keys();
    if (current >= keys.size()) return;

    QString orderKey = keys.at(current);
    QJsonObject order = orderlist.value(orderKey);

    QString orderId = order.value("Order_id").toString();
    QString orderPrice = order.value("Order_tolprice").toString();
    QString userBalance = QString::number(client->getPerson()->getMoney());

    // 添加调试信息，确认订单号的获取
    qDebug() << "Current order index:" << current;
    qDebug() << "Order key:" << orderKey;
    qDebug() << "Expected order ID from UI:" << ui->lab_order->text();
    qDebug() << "Actual order ID from data:" << orderId;

    // 确保使用正确的订单号 - 使用UI中显示的订单号
    QString currentDisplayedOrderId = ui->lab_order->text();
    if (!currentDisplayedOrderId.isEmpty()) {
        orderId = currentDisplayedOrderId;
    }

    PayDialog* payDialog = new PayDialog(orderId, orderPrice, userBalance, this);
    if (payDialog->exec() == QDialog::Accepted) {
        // 发送付款请求
        QString FLAG_INSKIND = "04";
        QString FLAG_INS = "06";
        QJsonObject obj;
        obj.insert("order_id", payDialog->getOrderId());
        obj.insert("user_id", QString::number(client->getPerson()->getId()));

        QByteArray data = client->sendCHTTPMsg(FLAG_CHARACTER + FLAG_INSKIND + FLAG_INS, obj);
        QString flag = client->parseHead(data);

        if (flag[0] == "1") {
            QJsonArray result = client->parseResponse(data);
            if (!result.isEmpty()) {
                QJsonObject payResult = result[0].toObject();
                QString remainingBalance = payResult.value("remaining_balance").toString();

                // 更新客户端用户余额
                client->getPerson()->setMoney(remainingBalance);

                QMessageBox::information(this, "付款成功",
                                         QString("付款成功！\n订单号：%1\n付款金额：￥%2\n剩余余额：￥%3")
                                             .arg(orderId)
                                             .arg(orderPrice)
                                             .arg(remainingBalance));

                // 刷新订单信息
                getAllOrder();
            }
        }
        else {
            QString errorMsg = client->parseError(data);
            QMessageBox::warning(this, "付款失败", errorMsg);
        }
    }
    delete payDialog;
}
