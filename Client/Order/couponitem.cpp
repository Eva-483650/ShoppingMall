#include "couponitem.h"
#include <QDateTime>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDebug>

CouponItem::CouponItem(const QJsonObject& couponData, int orderAmount, QWidget *parent)
    : QWidget(parent), m_couponData(couponData), m_orderAmount(orderAmount)
{
    setObjectName("couponCard");
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);


    // 解析优惠券数据
    m_couponId = couponData.value("Coupon_id").toString().toInt(); // 注意这里可能需要toString().toInt()
    m_couponCode = couponData.value("Coupon_code").toString();
    m_couponName = couponData.value("Coupon_name").toString();
    m_couponType = couponData.value("Coupon_type").toString();
    m_couponValue = couponData.value("Coupon_value").toString().toDouble(); // 注意这里也可能需要toString().toDouble()
    m_minAmount = couponData.value("Coupon_min_amount").toString().toInt();

    // 处理 max_discount，可能为 "0" 或 null
    QJsonValue maxDiscountValue = couponData.value("Coupon_max_discount");
    if (maxDiscountValue.isString()) {
        m_maxDiscount = maxDiscountValue.toString().toInt();
    } else {
        m_maxDiscount = maxDiscountValue.toInt();
    }

    // 检查优惠券是否可用
    updateUsableStatus(orderAmount);

    setupUI();

    // 强制启用
    this->setEnabled(true);
}

CouponItem::~CouponItem()
{
}

void CouponItem::setupUI()
{
    qDebug() << "=== 强制QSS字体版本 setupUI ===";

    // 完全清空样式
    setStyleSheet("");
    setObjectName("");

    setFixedHeight(170);
    setMinimumWidth(380);
    setEnabled(true);

    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(255, 255, 255));
    setPalette(pal);

    // 主布局
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(15, 10, 15, 10);
    mainLayout->setSpacing(15);

    // 左侧容器
    QWidget* leftWidget = new QWidget();
    leftWidget->setFixedWidth(82);
    QVBoxLayout* leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setAlignment(Qt::AlignCenter);
    leftLayout->setSpacing(5);

    // 金额标签 - 改为微软雅黑
    lab_couponValue = new QLabel(QString("￥%1").arg((int)m_couponValue), this);
    lab_couponValue->setStyleSheet(R"(
        QLabel {
            font-family: "Microsoft YaHei", sans-serif;
            font-size: 26px;
            font-weight: bold;
            color: rgb(120, 60, 180);
            background: transparent;
        }
    )");
    lab_couponValue->setAlignment(Qt::AlignCenter);
    lab_couponValue->setMinimumWidth(80);

    // 类型标签 - 改为微软雅黑
    QLabel* typeLabel = new QLabel("优惠券", this);
    typeLabel->setStyleSheet(R"(
        QLabel {
            font-family: "Microsoft YaHei", sans-serif;
            font-size: 14px;
            color: rgb(100, 100, 100);
            background: transparent;
        }
    )");
    typeLabel->setAlignment(Qt::AlignCenter);

    leftLayout->addWidget(lab_couponValue);
    leftLayout->addWidget(typeLabel);

    // 中间信息容器
    QWidget* middleWidget = new QWidget();
    QVBoxLayout* middleLayout = new QVBoxLayout(middleWidget);
    middleLayout->setSpacing(0);
    middleLayout->setContentsMargins(0, 0, 0, 0);

    // 名称标签 - 改为微软雅黑
    lab_couponName = new QLabel(m_couponName, this);
    lab_couponName->setStyleSheet(R"(
        QLabel {
            font-family: "Microsoft YaHei", sans-serif;
            font-size: 20px;
            font-weight: bold;
            color: rgb(50, 50, 50);
            background: transparent;
        }
    )");
    lab_couponName->setMinimumWidth(200);
    lab_couponName->setWordWrap(true);

    // 条件标签 - 改为微软雅黑
    lab_condition = new QLabel(QString("满￥%1可用").arg(m_minAmount), this);
    lab_condition->setStyleSheet(R"(
        QLabel {
            font-family: "Microsoft YaHei", sans-serif;
            font-size: 18px;
            color: rgb(80, 80, 80);
            background: transparent;
        }
    )");
    lab_condition->setMinimumWidth(140);

    // 描述标签 - 改为微软雅黑
    QString description = m_couponData.value("Coupon_description").toString();
    lab_couponDesc = new QLabel(description, this);
    lab_couponDesc->setStyleSheet(R"(
        QLabel {
            font-family: "Microsoft YaHei", sans-serif;
            font-size: 18px;
            color: rgb(100, 100, 100);
            background: transparent;
        }
    )");
    lab_couponDesc->setWordWrap(true);
    lab_couponDesc->setMinimumWidth(200);

    // 有效期标签 - 改为微软雅黑
    QString endTime = m_couponData.value("Coupon_end_time").toString();
    QString displayTime = endTime;
    if (displayTime.endsWith("Z")) {
        displayTime.chop(1);
    }
    if (displayTime.contains("T")) {
        displayTime = displayTime.left(displayTime.indexOf("T"));
    }

    lab_validity = new QLabel(QString("有效期至：%1").arg(displayTime), this);
    lab_validity->setStyleSheet(R"(
        QLabel {
            font-family: "Microsoft YaHei", sans-serif;
            font-size: 18px;
            color: rgb(120, 120, 120);
            background: transparent;
        }
    )");
    lab_validity->setMinimumWidth(150);

    middleLayout->addWidget(lab_couponName);
    middleLayout->addWidget(lab_condition);
    middleLayout->addWidget(lab_couponDesc);
    middleLayout->addWidget(lab_validity);
    middleLayout->addStretch();

    // 右侧按钮 - 改为渐变紫色样式
    btn_select = new QPushButton("选择", this);
    btn_select->setFixedSize(80, 35);
    btn_select->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1,
                        stop:0 rgba(189, 170, 233, 255),
                        stop:0.5 rgba(195, 176, 238, 255),
                        stop:1 rgba(201, 182, 243, 255));
            color: white;
            border: 1px solid rgba(255, 255, 255, 90);
            border-radius: 17px;
            font-family: "Microsoft YaHei", sans-serif;
            font-size: 22px;
            font-weight: bold;
        }
        QPushButton:hover {
            background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1,
                        stop:0 rgba(195, 176, 238, 255),
                        stop:0.5 rgba(201, 182, 243, 255),
                        stop:1 rgba(207, 188, 248, 255));
        }
        QPushButton:pressed {
            background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1,
                        stop:0 rgba(179, 160, 223, 255),
                        stop:0.5 rgba(189, 170, 233, 255),
                        stop:1 rgba(195, 176, 238, 255));
        }
    )");

    // 组装主布局
    mainLayout->addWidget(leftWidget);
    mainLayout->addWidget(middleWidget, 1);
    mainLayout->addWidget(btn_select);

    // 连接信号
    connect(btn_select, &QPushButton::clicked, this, &CouponItem::onSelectClicked);

    qDebug() << "QSS字体设置完成";
}

void CouponItem::updateUsableStatus(int orderAmount)
{
    m_orderAmount = orderAmount;

    qDebug() << "=== 优惠券可用性检查 ===";
    qDebug() << "优惠券ID:" << m_couponId;
    qDebug() << "优惠券名称:" << m_couponName;
    qDebug() << "最低使用金额:" << m_minAmount;
    qDebug() << "当前订单金额:" << orderAmount;

    // 检查是否满足最低使用金额
    bool amountOk = (orderAmount >= m_minAmount);
    qDebug() << "金额检查:" << (amountOk ? "通过" : "未通过");

    // 检查有效期 - 尝试多种日期格式
    QString endTime = m_couponData.value("Coupon_end_time").toString();
    qDebug() << "原始结束时间:" << endTime;

    QDateTime endDateTime;
    bool dateParseOk = false;

    // 尝试不同的日期格式
    QStringList dateFormats = {
        "yyyy-MM-ddThh:mm:ss.zzzt",      // 2025-12-31T23:59:59.000Z
        "yyyy-MM-dd hh:mm:ss",           // 2025-12-31 23:59:59
        "yyyy-MM-ddThh:mm:ss"            // 2025-12-31T23:59:59
    };

    // 先尝试字符串格式
    for (const QString& format : dateFormats) {
        endDateTime = QDateTime::fromString(endTime, format);
        if (endDateTime.isValid()) {
            dateParseOk = true;
            qDebug() << "日期解析成功，格式:" << format;
            qDebug() << "解析后的结束时间:" << endDateTime.toString("yyyy-MM-dd hh:mm:ss");
            break;
        }
    }

    // 如果字符串格式都不行，尝试 Qt::ISODate
    if (!dateParseOk) {
        endDateTime = QDateTime::fromString(endTime, Qt::ISODate);
        if (endDateTime.isValid()) {
            dateParseOk = true;
            qDebug() << "日期解析成功，使用 Qt::ISODate";
            qDebug() << "解析后的结束时间:" << endDateTime.toString("yyyy-MM-dd hh:mm:ss");
        }
    }

    // 如果还是不行，尝试去掉时区信息
    if (!dateParseOk && endTime.contains("Z")) {
        QString cleanTime = endTime;
        cleanTime.remove("Z");
        cleanTime.replace("T", " ");
        if (cleanTime.contains(".")) {
            cleanTime = cleanTime.left(cleanTime.indexOf("."));
        }
        qDebug() << "清理后的时间:" << cleanTime;

        endDateTime = QDateTime::fromString(cleanTime, "yyyy-MM-dd hh:mm:ss");
        if (endDateTime.isValid()) {
            dateParseOk = true;
            qDebug() << "日期解析成功，使用清理后的格式";
            qDebug() << "解析后的结束时间:" << endDateTime.toString("yyyy-MM-dd hh:mm:ss");
        }
    }

    if (!dateParseOk) {
        qDebug() << "日期解析失败，所有格式都不匹配";
        m_isUsable = false;
        return;
    }

    QDateTime currentTime = QDateTime::currentDateTime();
    qDebug() << "当前时间:" << currentTime.toString("yyyy-MM-dd hh:mm:ss");

    bool timeOk = (currentTime <= endDateTime);
    qDebug() << "时间检查:" << (timeOk ? "通过" : "未通过");

    // 检查优惠券状态
    QString status = m_couponData.value("Coupon_status").toString();
    bool statusOk = (status == "有效" || status.isEmpty()); // 如果没有状态字段，默认认为有效
    qDebug() << "优惠券状态:" << status << "状态检查:" << (statusOk ? "通过" : "未通过");

    // 最终结果
    m_isUsable = amountOk && timeOk && statusOk;
    qDebug() << "最终可用性:" << (m_isUsable ? "可用" : "不可用");
}


int CouponItem::calculateDiscount(int orderAmount) const
{
    if (!m_isUsable || orderAmount < m_minAmount) {
        return 0;
    }

    int discount = 0;
    if (m_couponType == "固定金额") {
        discount = (int)m_couponValue;
    } else { // 百分比折扣
        discount = (int)(orderAmount * m_couponValue / 100.0);
        if (m_maxDiscount > 0 && discount > m_maxDiscount) {
            discount = m_maxDiscount;
        }
    }

    // 折扣不能超过订单金额
    if (discount > orderAmount) {
        discount = orderAmount;
    }

    return discount;
}

void CouponItem::onSelectClicked()
{
    if (m_isUsable) {
        int discountAmount = calculateDiscount(m_orderAmount);
        emit couponSelected(m_couponId, discountAmount);
    }
}
