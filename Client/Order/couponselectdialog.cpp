#include "couponselectdialog.h"
#include "couponitem.h"
#include "shoppingclient.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QJsonDocument>
#include <QDebug>

CouponSelectDialog::CouponSelectDialog(int orderAmount, ShoppingClient* client, QWidget *parent)
    : QDialog(parent), m_client(client), m_orderAmount(orderAmount),
    m_selectedCouponId(-1), m_discountAmount(0)
{
    setWindowTitle("选择优惠券");
    setFixedSize(650, 500);
    setModal(true);
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);

    setupUI();
    loadUserCoupons();
}

CouponSelectDialog::~CouponSelectDialog()
{
}

void CouponSelectDialog::setupUI()
{
    setStyleSheet(R"(
        QDialog {
            background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1,
                        stop:0 rgba(252, 250, 255, 255),
                        stop:0.5 rgba(250, 247, 254, 255),
                        stop:1 rgba(248, 245, 253, 255));
            border-radius: 15px;
        }

        QScrollArea {
            background: transparent;
            border: 1px solid rgba(189, 170, 233, 30);
            border-radius: 10px;
        }

        QScrollBar:vertical {
            background: rgba(240, 235, 250, 150);
            width: 12px;
            border-radius: 6px;
        }

        QScrollBar::handle:vertical {
            background: rgba(189, 170, 233, 150);
            border-radius: 6px;
            min-height: 20px;
        }

        QScrollBar::handle:vertical:hover {
            background: rgba(189, 170, 233, 200);
        }
    )");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // 标题
    lab_title = new QLabel("选择优惠券");
    lab_title->setStyleSheet(R"(
        QLabel {
            font: bold 18pt "Microsoft YaHei";
            color: rgb(70, 60, 90);
            padding: 10px;
        }
    )");
    lab_title->setAlignment(Qt::AlignCenter);

    // 订单金额信息
    lab_orderAmount = new QLabel(QString("订单金额：￥%1").arg(m_orderAmount));
    lab_orderAmount->setStyleSheet(R"(
        QLabel {
            font: 14pt "Microsoft YaHei";
            color: rgb(100, 90, 120);
            background: rgba(240, 235, 250, 180);
            padding: 10px 15px;
            border-radius: 10px;
            border: 1px solid rgba(189, 170, 233, 30);
        }
    )");
    lab_orderAmount->setAlignment(Qt::AlignCenter);

    // 滚动区域
    scrollArea = new QScrollArea();
    scrollWidget = new QWidget();
    scrollLayout = new QVBoxLayout(scrollWidget);
    scrollLayout->setSpacing(8);
    scrollLayout->setContentsMargins(5, 5, 5, 5);

    scrollArea->setWidget(scrollWidget);
    scrollArea->setWidgetResizable(true);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 选中信息显示
    lab_selectedInfo = new QLabel("请选择一张优惠券");
    lab_selectedInfo->setStyleSheet(R"(
        QLabel {
            font: 12pt "Microsoft YaHei";
            color: rgb(120, 110, 140);
            background: rgba(255, 255, 255, 150);
            padding: 8px 15px;
            border-radius: 8px;
            border: 1px solid rgba(189, 170, 233, 25);
        }
    )");
    lab_selectedInfo->setAlignment(Qt::AlignCenter);

    // 按钮布局
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(15);

    btn_noCoupon = new QPushButton("不使用优惠券");
    btn_cancel = new QPushButton("取消");
    btn_confirm = new QPushButton("确认选择");

    QString buttonStyle = R"(
        QPushButton {
            background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1,
                        stop:0 rgba(189, 170, 233, 255),
                        stop:0.5 rgba(195, 176, 238, 255),
                        stop:1 rgba(201, 182, 243, 255));
            border: 1px solid rgba(255, 255, 255, 90);
            border-radius: 20px;
            font: 600 12pt "Microsoft YaHei";
            color: white;
            padding: 10px 20px;
            min-width: 100px;
            min-height: 40px;
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
    )";

    QString cancelStyle = R"(
        QPushButton {
            background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1,
                        stop:0 rgba(160, 160, 160, 255),
                        stop:0.5 rgba(170, 170, 170, 255),
                        stop:1 rgba(180, 180, 180, 255));
            border: 1px solid rgba(255, 255, 255, 80);
            border-radius: 20px;
            font: 600 12pt "Microsoft YaHei";
            color: white;
            padding: 10px 20px;
            min-width: 100px;
            min-height: 40px;
        }
        QPushButton:hover {
            background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1,
                        stop:0 rgba(170, 170, 170, 255),
                        stop:0.5 rgba(180, 180, 180, 255),
                        stop:1 rgba(190, 190, 190, 255));
        }
    )";

    btn_confirm->setStyleSheet(buttonStyle);
    btn_noCoupon->setStyleSheet(buttonStyle);
    btn_cancel->setStyleSheet(cancelStyle);

    btn_confirm->setEnabled(false); // 初始状态禁用

    buttonLayout->addWidget(btn_noCoupon);
    buttonLayout->addStretch();
    buttonLayout->addWidget(btn_cancel);
    buttonLayout->addWidget(btn_confirm);

    // 添加到主布局
    mainLayout->addWidget(lab_title);
    mainLayout->addWidget(lab_orderAmount);
    mainLayout->addWidget(scrollArea, 1);
    mainLayout->addWidget(lab_selectedInfo);
    mainLayout->addLayout(buttonLayout);

    // 连接信号槽
    connect(btn_confirm, &QPushButton::clicked, this, &CouponSelectDialog::onConfirmClicked);
    connect(btn_cancel, &QPushButton::clicked, this, &CouponSelectDialog::onCancelClicked);
    connect(btn_noCoupon, &QPushButton::clicked, this, &CouponSelectDialog::onNoCouponClicked);
}

void CouponSelectDialog::loadUserCoupons()
{
    if (!m_client || !m_client->getConnected()) {
        QMessageBox::warning(this, "错误", "网络连接失败");
        return;
    }

    // 检查用户是否已登录
    if (!m_client->getPerson()) {
        QMessageBox::warning(this, "错误", "用户未登录");
        return;
    }

    int userId = m_client->getPerson()->getId();
    if (userId <= 0) {
        QMessageBox::warning(this, "错误", "用户ID无效");
        return;
    }

    // 构造请求
    QJsonObject requestBody;
    requestBody.insert("user_id", userId);  // 直接传递整数
    requestBody.insert("order_amount", m_orderAmount);

    qDebug() << "=== 客户端优惠券请求调试 ===";
    qDebug() << "用户对象:" << (m_client->getPerson() ? "存在" : "不存在");
    qDebug() << "用户ID:" << userId;
    qDebug() << "订单金额:" << m_orderAmount;
    qDebug() << "请求体:" << requestBody;

    // 发送请求
    const QString COUPON_PROTOCOL = "10501";
    QByteArray response = m_client->sendCHTTPMsg(COUPON_PROTOCOL, requestBody);

    // 解析响应
    QString head = m_client->parseHead(response);
    qDebug() << "优惠券请求响应头:" << head;

    if (head.startsWith("1")) { // 成功
        QJsonArray coupons = m_client->parseResponse(response);

        qDebug() << "获取到优惠券数量:" << coupons.size();

        if (coupons.isEmpty()) {
            QLabel* noCouponLabel = new QLabel("暂无可用优惠券");
            noCouponLabel->setStyleSheet(R"(
                QLabel {
                    font: 14pt "Microsoft YaHei";
                    color: rgb(150, 150, 150);
                    padding: 30px;
                }
            )");
            noCouponLabel->setAlignment(Qt::AlignCenter);
            scrollLayout->addWidget(noCouponLabel);
        } else {
            for (const QJsonValue& value : coupons) {
                if (value.isObject()) {
                    qDebug() << "优惠券数据:" << value.toObject();
                    addCouponItem(value.toObject());
                }
            }
        }

        scrollLayout->addStretch();
    } else {
        QString errorMsg = head.mid(1); // 去掉错误代码
        qDebug() << "加载优惠券失败:" << errorMsg;
        QMessageBox::warning(this, "加载失败", "加载优惠券失败：" + errorMsg);
    }
}

void CouponSelectDialog::addCouponItem(const QJsonObject& couponData)
{
    CouponItem* item = new CouponItem(couponData, m_orderAmount, this);
    m_couponItems.append(item);
    scrollLayout->addWidget(item);

    // 连接信号
    connect(item, &CouponItem::couponSelected,
            this, &CouponSelectDialog::onCouponItemSelected);
}

void CouponSelectDialog::onCouponItemSelected(int couponId, int discountAmount)
{
    m_selectedCouponId = couponId;
    m_discountAmount = discountAmount;

    // 更新选中信息显示
    lab_selectedInfo->setText(QString("已选择优惠券，优惠金额：￥%1").arg(discountAmount));
    lab_selectedInfo->setStyleSheet(R"(
        QLabel {
            font: 12pt "Microsoft YaHei";
            color: rgb(70, 130, 70);
            background: rgba(200, 255, 200, 150);
            padding: 8px 15px;
            border-radius: 8px;
            border: 1px solid rgba(100, 200, 100, 50);
        }
    )");

    btn_confirm->setEnabled(true);

    qDebug() << "选中优惠券 ID:" << couponId << "折扣金额:" << discountAmount;
}

void CouponSelectDialog::onConfirmClicked()
{
    if (m_selectedCouponId > 0) {
        emit couponSelected(m_selectedCouponId, m_discountAmount);
        accept();
    }
}

void CouponSelectDialog::onCancelClicked()
{
    reject();
}

void CouponSelectDialog::onNoCouponClicked()
{
    m_selectedCouponId = -1;
    m_discountAmount = 0;
    emit couponSelected(-1, 0);
    accept();
}
