#include "orderdetaildialog.h"
#include <QApplication>
#include <QScreen>
#include <QGridLayout>
#include <QDateTime>
#include <QDebug>
#include <QMessageBox>

OrderDetailDialog::OrderDetailDialog(const QString &orderId, int userId, const QString &amount,
                                     const QString &status, const QString &createTime,
                                     const QJsonArray &orderItems, ShoppingManager *manager,
                                     QWidget *parent)
    : QDialog(parent)
    , m_orderId(orderId)
    , m_userId(userId)
    , m_amount(amount)
    , m_status(status)
    , m_createTime(createTime)
    , m_orderItems(orderItems)
    , m_manager(manager)
{
    setupUI();
    setOrderBasicInfo();
    setOrderItems();
    addReturnInfoSection();
    applyStyles();
    addShadowEffect();

    // 设置对话框属性
    setModal(true);
    setWindowTitle("订单详情");
    setFixedSize(600, 750);

    // 居中显示
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
}

void OrderDetailDialog::setupUI()
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 创建滚动区域
    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    contentWidget = new QWidget();
    contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(30, 30, 30, 30);
    contentLayout->setSpacing(20);

    // 标题
    titleLabel = new QLabel("📋 订单详情");
    titleLabel->setAlignment(Qt::AlignCenter);
    contentLayout->addWidget(titleLabel);

    // 基本信息框架
    headerFrame = new QFrame();
    headerFrame->setFrameStyle(QFrame::StyledPanel);
    contentLayout->addWidget(headerFrame);

    // 商品信息框架
    itemsFrame = new QFrame();
    itemsFrame->setFrameStyle(QFrame::StyledPanel);
    contentLayout->addWidget(itemsFrame);

    // 总计框架
    totalFrame = new QFrame();
    totalFrame->setFrameStyle(QFrame::StyledPanel);
    contentLayout->addWidget(totalFrame);

    // 退货信息框架
    returnFrame = new QFrame();
    returnFrame->setFrameStyle(QFrame::StyledPanel);
    contentLayout->addWidget(returnFrame);

    // 只保留关闭按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    closeButton = new QPushButton("关闭");
    closeButton->setFixedSize(100, 40);
    connect(closeButton, &QPushButton::clicked, this, &OrderDetailDialog::onCloseClicked);

    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    buttonLayout->addStretch();

    contentLayout->addLayout(buttonLayout);

    scrollArea->setWidget(contentWidget);
    mainLayout->addWidget(scrollArea);
}

void OrderDetailDialog::setOrderBasicInfo()
{
    QVBoxLayout *headerLayout = new QVBoxLayout(headerFrame);
    headerLayout->setContentsMargins(20, 15, 20, 15);
    headerLayout->setSpacing(10);

    // 订单ID
    QHBoxLayout *idLayout = new QHBoxLayout();
    QLabel *idIcon = new QLabel("🆔");
    QLabel *idLabel = new QLabel("订单ID:");
    QLabel *idValue = new QLabel(m_orderId);
    idValue->setStyleSheet("color: #6c757d; font-weight: bold; background-color: transparent;");
    idLayout->addWidget(idIcon);
    idLayout->addWidget(idLabel);
    idLayout->addWidget(idValue);
    idLayout->addStretch();
    headerLayout->addLayout(idLayout);

    // 买家ID
    QHBoxLayout *userLayout = new QHBoxLayout();
    QLabel *userIcon = new QLabel("👤");
    QLabel *userLabel = new QLabel("买家ID:");
    QLabel *userValue = new QLabel(QString::number(m_userId));
    userValue->setStyleSheet("color: #6c757d; font-weight: bold; background-color: transparent;");
    userLayout->addWidget(userIcon);
    userLayout->addWidget(userLabel);
    userLayout->addWidget(userValue);
    userLayout->addStretch();
    headerLayout->addLayout(userLayout);

    // 下单时间
    QHBoxLayout *timeLayout = new QHBoxLayout();
    QLabel *timeIcon = new QLabel("📅");
    QLabel *timeLabel = new QLabel("下单时间:");
    QDateTime dateTime = QDateTime::fromString(m_createTime, "yyyy-MM-ddThh:mm:ss.000Z");
    QString formattedTime = dateTime.toLocalTime().toString("yyyy-MM-dd hh:mm:ss");
    QLabel *timeValue = new QLabel(formattedTime);
    timeValue->setStyleSheet("color: #6c757d; background-color: transparent;");
    timeLayout->addWidget(timeIcon);
    timeLayout->addWidget(timeLabel);
    timeLayout->addWidget(timeValue);
    timeLayout->addStretch();
    headerLayout->addLayout(timeLayout);

    // 订单状态
    QHBoxLayout *statusLayout = new QHBoxLayout();
    QLabel *statusIcon = new QLabel("📊");
    QLabel *statusLabel = new QLabel("订单状态:");
    QLabel *statusValue = new QLabel(m_status);

    // 根据状态设置不同颜色
    QString statusStyle = "font-weight: bold; background-color: transparent;";
    if (m_status == "待付款") {
        statusValue->setStyleSheet(statusStyle + "color: #fd7e14;");
    } else if (m_status == "已付款") {
        statusValue->setStyleSheet(statusStyle + "color: #007bff;");
    } else if (m_status == "已发货") {
        statusValue->setStyleSheet(statusStyle + "color: #6f42c1;");
    } else if (m_status == "已完成") {
        statusValue->setStyleSheet(statusStyle + "color: #28a745;");
    } else if (m_status == "已取消") {
        statusValue->setStyleSheet(statusStyle + "color: #dc3545;");
    } else if (m_status == "申请退货") {
        statusValue->setStyleSheet(statusStyle + "color: #ffc107;");
    } else if (m_status == "退货中") {
        statusValue->setStyleSheet(statusStyle + "color: #6c757d;");
    } else if (m_status == "已退货") {
        statusValue->setStyleSheet(statusStyle + "color: #6f42c1;");
    } else {
        statusValue->setStyleSheet(statusStyle + "color: #6c757d;");
    }

    statusLayout->addWidget(statusIcon);
    statusLayout->addWidget(statusLabel);
    statusLayout->addWidget(statusValue);
    statusLayout->addStretch();
    headerLayout->addLayout(statusLayout);

    // 订单金额
    QHBoxLayout *amountLayout = new QHBoxLayout();
    QLabel *amountIcon = new QLabel("💰");
    QLabel *amountLabel = new QLabel("订单金额:");
    QLabel *amountValue = new QLabel(m_amount);
    amountValue->setStyleSheet("color: #28a745; font-size: 16px; font-weight: bold; background-color: transparent;");
    amountLayout->addWidget(amountIcon);
    amountLayout->addWidget(amountLabel);
    amountLayout->addWidget(amountValue);
    amountLayout->addStretch();
    headerLayout->addLayout(amountLayout);
}

void OrderDetailDialog::setOrderItems()
{
    QVBoxLayout *itemsLayout = new QVBoxLayout(itemsFrame);
    itemsLayout->setContentsMargins(20, 15, 20, 15);
    itemsLayout->setSpacing(15);

    // 标题
    QLabel *itemsTitle = new QLabel(QString("🛒 购买商品 (共%1种商品)").arg(m_orderItems.size()));
    itemsTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #495057; background-color: transparent;");
    itemsLayout->addWidget(itemsTitle);

    // 分隔线
    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("color: #dee2e6; background-color: #dee2e6;");
    itemsLayout->addWidget(line);

    double calculatedTotal = 0;

    // 商品列表
    for (int i = 0; i < m_orderItems.size(); ++i) {
        QJsonObject item = m_orderItems[i].toObject();
        QString productName = item["Product_name"].toString();
        int quantity = item["Orderitem_num"].toString().toInt();
        int price = item["Orderitem_pro_price"].toString().toInt();
        double itemTotal = price * quantity;
        calculatedTotal += itemTotal;

        // 商品框架
        QFrame *itemFrame = new QFrame();
        itemFrame->setFrameStyle(QFrame::StyledPanel);
        itemFrame->setStyleSheet(
            "QFrame {"
            "    background-color: #f8f9fa;"
            "    border: 1px solid #e9ecef;"
            "    border-radius: 8px;"
            "    padding: 10px;"
            "}"
            );

        QVBoxLayout *itemLayout = new QVBoxLayout(itemFrame);
        itemLayout->setContentsMargins(15, 10, 15, 10);
        itemLayout->setSpacing(5);

        // 商品名称
        QLabel *nameLabel = new QLabel(QString("%1. %2").arg(i+1).arg(productName));
        nameLabel->setStyleSheet("font-size: 13px; font-weight: bold; color: #495057; background-color: transparent;");
        itemLayout->addWidget(nameLabel);

        // 商品详情
        QHBoxLayout *detailLayout = new QHBoxLayout();

        QLabel *qtyLabel = new QLabel(QString("数量: %1 件").arg(quantity));
        qtyLabel->setStyleSheet("color: #6c757d; font-size: 11px; background-color: transparent;");

        QLabel *priceLabel = new QLabel(QString("单价: ¥%1").arg(price));
        priceLabel->setStyleSheet("color: #6c757d; font-size: 11px; background-color: transparent;");

        QLabel *totalLabel = new QLabel(QString("小计: ¥%1").arg(itemTotal));
        totalLabel->setStyleSheet("color: #007bff; font-size: 11px; font-weight: bold; background-color: transparent;");

        detailLayout->addWidget(qtyLabel);
        detailLayout->addWidget(priceLabel);
        detailLayout->addStretch();
        detailLayout->addWidget(totalLabel);

        itemLayout->addLayout(detailLayout);
        itemsLayout->addWidget(itemFrame);
    }

    // 总计信息
    QVBoxLayout *totalLayout = new QVBoxLayout(totalFrame);
    totalLayout->setContentsMargins(20, 15, 20, 15);
    totalLayout->setSpacing(10);

    QFrame *totalLine = new QFrame();
    totalLine->setFrameShape(QFrame::HLine);
    totalLine->setStyleSheet("color: #dee2e6; background-color: #dee2e6;");
    totalLayout->addWidget(totalLine);

    QHBoxLayout *calcLayout = new QHBoxLayout();
    QLabel *calcLabel = new QLabel("🧮 计算总金额:");
    calcLabel->setStyleSheet("color: #495057; background-color: transparent;");
    QLabel *calcValue = new QLabel(QString("¥%1").arg(calculatedTotal));
    calcValue->setStyleSheet("color: #6c757d; font-weight: bold; background-color: transparent;");
    calcLayout->addWidget(calcLabel);
    calcLayout->addStretch();
    calcLayout->addWidget(calcValue);
    totalLayout->addLayout(calcLayout);

    QHBoxLayout *orderTotalLayout = new QHBoxLayout();
    QLabel *orderTotalLabel = new QLabel("💰 订单总金额:");
    orderTotalLabel->setStyleSheet("color: #495057; background-color: transparent;");
    QLabel *orderTotalValue = new QLabel(m_amount);
    orderTotalValue->setStyleSheet("color: #28a745; font-size: 16px; font-weight: bold; background-color: transparent;");
    orderTotalLayout->addWidget(orderTotalLabel);
    orderTotalLayout->addStretch();
    orderTotalLayout->addWidget(orderTotalValue);
    totalLayout->addLayout(orderTotalLayout);

    // 检查金额是否一致
    QString amountCopy = m_amount;
    double orderAmount = amountCopy.remove("¥").toDouble();
    if (qAbs(calculatedTotal - orderAmount) > 0.01) {
        QLabel *warningLabel = new QLabel("⚠️ 注意：计算金额与订单金额不符！");
        warningLabel->setStyleSheet("color: #dc3545; font-weight: bold; background-color: transparent;");
        totalLayout->addWidget(warningLabel);
    }
}

void OrderDetailDialog::addReturnInfoSection()
{
    QVBoxLayout *returnLayout = new QVBoxLayout(returnFrame);
    returnLayout->setContentsMargins(20, 15, 20, 15);
    returnLayout->setSpacing(10);

    // 标题
    QLabel *returnTitle = new QLabel("🔄 退货信息");
    returnTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #495057; background-color: transparent;");
    returnLayout->addWidget(returnTitle);

    // 分隔线
    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("color: #dee2e6; background-color: #dee2e6;");
    returnLayout->addWidget(line);

    // 检查是否有退货相关状态
    if (m_status == "申请退货" || m_status == "退货中" || m_status == "已退货") {
        // 自动加载退货信息
        loadReturnInfo();
    } else {
        // 显示无退货信息
        QLabel *noReturnLabel = new QLabel("📌 该订单暂无退货信息");
        noReturnLabel->setStyleSheet("color: #6c757d; font-style: italic; background-color: transparent;");
        returnLayout->addWidget(noReturnLabel);
    }
}

void OrderDetailDialog::loadReturnInfo()
{
    if (!m_manager || !m_manager->getConnected()) {
        QLabel *errorLabel = new QLabel("❌ 无法连接到服务器获取退货信息");
        errorLabel->setStyleSheet("color: #dc3545; background-color: transparent;");
        returnFrame->layout()->addWidget(errorLabel);
        return;
    }

    // 构建查询退货信息的请求
    QJsonObject body;
    body.insert("want", "*");
    body.insert("restriction", QString("Return_order_id = '%1'").arg(m_orderId));

    qDebug() << "查询退货信息，订单ID:" << m_orderId;

    // 发送请求到服务器 (协议码 20408 - 获取退货列表)
    QByteArray response = m_manager->sendCHTTPMsg("20408", body);

    if (response.isEmpty()) {
        QLabel *errorLabel = new QLabel("❌ 获取退货信息失败");
        errorLabel->setStyleSheet("color: #dc3545; background-color: transparent;");
        returnFrame->layout()->addWidget(errorLabel);
        return;
    }

    QString head = m_manager->parseHead(response);
    if (head.startsWith("1")) {
        QJsonArray returnInfo = m_manager->parseResponse(response);
        displayReturnInfo(returnInfo);
    } else {
        QLabel *noReturnLabel = new QLabel("📌 该订单暂无退货记录");
        noReturnLabel->setStyleSheet("color: #6c757d; font-style: italic; background-color: transparent;");
        returnFrame->layout()->addWidget(noReturnLabel);
    }
}

void OrderDetailDialog::displayReturnInfo(const QJsonArray &returnInfo)
{
    QVBoxLayout *returnLayout = qobject_cast<QVBoxLayout*>(returnFrame->layout());
    if (!returnLayout) return;

    if (returnInfo.isEmpty()) {
        QLabel *noReturnLabel = new QLabel("📌 该订单暂无退货记录");
        noReturnLabel->setStyleSheet("color: #6c757d; font-style: italic; background-color: transparent;");
        returnLayout->addWidget(noReturnLabel);
        return;
    }

    // 显示退货信息
    for (int i = 0; i < returnInfo.size(); ++i) {
        QJsonObject returnItem = returnInfo[i].toObject();

        // 创建退货项目框架
        QFrame *returnItemFrame = new QFrame();
        returnItemFrame->setFrameStyle(QFrame::StyledPanel);
        returnItemFrame->setStyleSheet(
            "QFrame {"
            "    background-color: #fff3cd;"
            "    border: 1px solid #ffeaa7;"
            "    border-radius: 8px;"
            "    padding: 10px;"
            "}"
            );

        QVBoxLayout *itemLayout = new QVBoxLayout(returnItemFrame);
        itemLayout->setContentsMargins(15, 10, 15, 10);
        itemLayout->setSpacing(8);

        // 退货单号
        QHBoxLayout *idLayout = new QHBoxLayout();
        QLabel *idLabel = new QLabel("退货单号:");
        idLabel->setStyleSheet("background-color: transparent;");
        QLabel *idValue = new QLabel(returnItem["Return_id"].toString());
        idValue->setStyleSheet("color: #e67e22; font-weight: bold; background-color: transparent;");
        idLayout->addWidget(idLabel);
        idLayout->addWidget(idValue);
        idLayout->addStretch();
        itemLayout->addLayout(idLayout);

        // 退货类型
        QHBoxLayout *typeLayout = new QHBoxLayout();
        QLabel *typeLabel = new QLabel("退货类型:");
        typeLabel->setStyleSheet("background-color: transparent;");
        QLabel *typeValue = new QLabel(returnItem["Return_type"].toString());
        typeValue->setStyleSheet("color: #d35400; font-weight: bold; background-color: transparent;");
        typeLayout->addWidget(typeLabel);
        typeLayout->addWidget(typeValue);
        typeLayout->addStretch();
        itemLayout->addLayout(typeLayout);

        // 退货状态
        QHBoxLayout *statusLayout = new QHBoxLayout();
        QLabel *statusLabel = new QLabel("退货状态:");
        statusLabel->setStyleSheet("background-color: transparent;");
        QLabel *statusValue = new QLabel(returnItem["Return_status"].toString());
        QString returnStatus = returnItem["Return_status"].toString();
        QString statusStyle = "font-weight: bold; background-color: transparent;";
        if (returnStatus == "待审核") {
            statusValue->setStyleSheet(statusStyle + "color: #f39c12;");
        } else if (returnStatus == "审核通过") {
            statusValue->setStyleSheet(statusStyle + "color: #27ae60;");
        } else if (returnStatus == "审核拒绝") {
            statusValue->setStyleSheet(statusStyle + "color: #e74c3c;");
        } else if (returnStatus == "已完成") {
            statusValue->setStyleSheet(statusStyle + "color: #2ecc71;");
        } else {
            statusValue->setStyleSheet(statusStyle + "color: #6c757d;");
        }
        statusLayout->addWidget(statusLabel);
        statusLayout->addWidget(statusValue);
        statusLayout->addStretch();
        itemLayout->addLayout(statusLayout);

        // 退货金额
        QHBoxLayout *amountLayout = new QHBoxLayout();
        QLabel *amountLabel = new QLabel("退货金额:");
        amountLabel->setStyleSheet("background-color: transparent;");
        int returnAmount = returnItem["Return_amount"].toString().toInt();
        QLabel *amountValue = new QLabel(QString("¥%1").arg(returnAmount));
        amountValue->setStyleSheet("color: #dc3545; font-weight: bold; background-color: transparent;");
        amountLayout->addWidget(amountLabel);
        amountLayout->addWidget(amountValue);
        amountLayout->addStretch();
        itemLayout->addLayout(amountLayout);

        // 退货原因
        QString reason = returnItem["Return_reason"].toString();
        if (!reason.isEmpty()) {
            QLabel *reasonLabel = new QLabel(QString("退货原因: %1").arg(reason));
            reasonLabel->setStyleSheet("color: #6c757d; font-size: 11px; background-color: transparent;");
            reasonLabel->setWordWrap(true);
            itemLayout->addWidget(reasonLabel);
        }

        // 申请时间
        QString applyTime = returnItem["Return_apply_time"].toString();
        if (!applyTime.isEmpty()) {
            QDateTime dateTime = QDateTime::fromString(applyTime, "yyyy-MM-ddThh:mm:ss.000Z");
            QString formattedTime = dateTime.toLocalTime().toString("yyyy-MM-dd hh:mm:ss");
            QLabel *timeLabel = new QLabel(QString("申请时间: %1").arg(formattedTime));
            timeLabel->setStyleSheet("color: #6c757d; font-size: 10px; background-color: transparent;");
            itemLayout->addWidget(timeLabel);
        }

        returnLayout->addWidget(returnItemFrame);
    }
}

void OrderDetailDialog::applyStyles()
{
    // 主窗口样式 - 修复黑色背景问题
    this->setStyleSheet(
        "OrderDetailDialog {"
        "    background-color: #f8f9fa;"
        "    border-radius: 15px;"
        "}"
        "QDialog {"
        "    background-color: #f8f9fa;"
        "    border-radius: 15px;"
        "}"
        );

    // 滚动区域样式
    scrollArea->setStyleSheet(
        "QScrollArea {"
        "    border: none;"
        "    background-color: transparent;"
        "}"
        "QScrollBar:vertical {"
        "    background: rgba(230, 230, 230, 0.8);"
        "    width: 8px;"
        "    border-radius: 4px;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background: rgba(180, 180, 180, 0.8);"
        "    border-radius: 4px;"
        "    min-height: 20px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "    background: rgba(160, 160, 160, 1);"
        "}"
        );

    // 内容区域样式
    contentWidget->setStyleSheet(
        "QWidget {"
        "    background-color: transparent;"
        "}"
        );

    // 标题样式
    titleLabel->setStyleSheet(
        "QLabel {"
        "    font-size: 20px;"
        "    font-weight: bold;"
        "    color: #2c3e50;"
        "    padding: 10px;"
        "    background-color: transparent;"
        "}"
        );

    // 框架样式 - 确保背景是白色
    QString frameStyle =
        "QFrame {"
        "    background-color: #ffffff;"
        "    border: 1px solid #dee2e6;"
        "    border-radius: 12px;"
        "}";

    headerFrame->setStyleSheet(frameStyle);
    itemsFrame->setStyleSheet(frameStyle);
    totalFrame->setStyleSheet(frameStyle);
    returnFrame->setStyleSheet(frameStyle);

    // 标签通用样式
    QString labelStyle =
        "QLabel {"
        "    font-family: '微软雅黑';"
        "    font-size: 12px;"
        "    color: #495057;"
        "    background-color: transparent;"
        "}";

    QList<QLabel*> labels = findChildren<QLabel*>();
    for (QLabel* label : labels) {
        if (label != titleLabel) {
            label->setStyleSheet(labelStyle);
        }
    }

    // 关闭按钮样式
    closeButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #007bff;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 20px;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "    font-family: '微软雅黑';"
        "}"
        "QPushButton:hover {"
        "    background-color: #0056b3;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #004085;"
        "}"
        );
}

void OrderDetailDialog::addShadowEffect()
{
    QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setBlurRadius(20);
    shadowEffect->setColor(QColor(0, 0, 0, 50));  // 使用黑色阴影
    shadowEffect->setOffset(0, 5);
    setGraphicsEffect(shadowEffect);
}

void OrderDetailDialog::onCloseClicked()
{
    accept();
}
