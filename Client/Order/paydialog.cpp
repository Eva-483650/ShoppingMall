#include "paydialog.h"
#include "couponselectdialog.h"
#include "shoppingclient.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QFrame>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

PayDialog::PayDialog(const QString& orderId, const QString& orderPrice, const QString& userBalance,
                     ShoppingClient* client, QWidget *parent)
    : QDialog(parent), m_orderId(orderId), m_orderPrice(orderPrice), m_userBalance(userBalance),
    m_client(client), m_selectedCouponId(-1), m_couponDiscount(0), m_merchantDiscount(0)
{
    setWindowTitle("订单付款");
    setFixedSize(520, 420); // 增加高度以容纳优惠券相关内容
    setModal(true);
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);

    m_originalPrice = orderPrice.toInt();
    m_finalPrice = m_originalPrice;

    setupUI();

    // 查询是否有商家折扣
    queryMerchantDiscounts();
}

PayDialog::~PayDialog()
{
}

void PayDialog::setupUI()
{
    setStyleSheet(R"(
        QDialog {
            background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1,
                        stop:0 rgba(252, 250, 255, 255),
                        stop:0.5 rgba(250, 247, 254, 255),
                        stop:1 rgba(248, 245, 253, 255));
            font-family: "Microsoft YaHei";
            border-radius: 15px;
        }

        QLabel {
            font: 14pt "Microsoft YaHei";
            color: rgb(85, 75, 105);
            background: transparent;
            padding: 8px 12px;
            border: none;
        }

        #titleLabel {
            font: bold 18pt "Microsoft YaHei";
            color: rgb(70, 60, 90);
            padding: 15px;
        }

        #orderLabel {
            font: 13pt "Microsoft YaHei";
            color: rgb(100, 90, 120);
            background: rgba(240, 235, 250, 180);
            padding: 12px 15px;
            border-radius: 12px;
            border: 1px solid rgba(189, 170, 233, 30);
            word-wrap: break-word;
            max-width: 450px;
        }

        #priceLabel {
            font: bold 16pt "Microsoft YaHei";
            color: rgb(100, 90, 120);
            background: rgba(240, 235, 250, 120);
            padding: 10px 15px;
            border-radius: 10px;
            border: 1px solid rgba(189, 170, 233, 25);
        }

        #finalPriceLabel {
            font: bold 20pt "Microsoft YaHei";
            color: rgb(189, 170, 233);
            background: rgba(189, 170, 233, 15);
            padding: 15px 20px;
            border-radius: 15px;
            border: 1px solid rgba(189, 170, 233, 40);
        }

        #discountLabel {
            font: 14pt "Microsoft YaHei";
            color: rgb(70, 130, 70);
            background: rgba(200, 255, 200, 120);
            padding: 8px 15px;
            border-radius: 8px;
            border: 1px solid rgba(100, 200, 100, 40);
        }

        #couponLabel {
            font: 12pt "Microsoft YaHei";
            color: rgb(120, 110, 140);
            background: rgba(255, 255, 255, 150);
            padding: 8px 15px;
            border-radius: 8px;
            border: 1px solid rgba(189, 170, 233, 25);
        }

        #balanceLabel {
            font: 14pt "Microsoft YaHei";
            color: rgb(120, 110, 140);
            background: rgba(255, 255, 255, 150);
            padding: 10px 15px;
            border-radius: 10px;
            border: 1px solid rgba(189, 170, 233, 25);
        }

        QPushButton {
            background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1,
                        stop:0 rgba(189, 170, 233, 255),
                        stop:0.5 rgba(195, 176, 238, 255),
                        stop:1 rgba(201, 182, 243, 255));
            border: 1px solid rgba(255, 255, 255, 90);
            border-radius: 22px;
            font: 600 14pt "Microsoft YaHei";
            color: white;
            padding: 12px 25px;
            min-width: 100px;
            min-height: 44px;
        }

        QPushButton:hover {
            background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1,
                        stop:0 rgba(195, 176, 238, 255),
                        stop:0.5 rgba(201, 182, 243, 255),
                        stop:1 rgba(207, 188, 248, 255));
            border: 1px solid rgba(255, 255, 255, 110);
        }

        QPushButton:pressed {
            background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1,
                        stop:0 rgba(179, 160, 223, 255),
                        stop:0.5 rgba(189, 170, 233, 255),
                        stop:1 rgba(195, 176, 238, 255));
        }

        #cancelButton {
            background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1,
                        stop:0 rgba(160, 160, 160, 255),
                        stop:0.5 rgba(170, 170, 170, 255),
                        stop:1 rgba(180, 180, 180, 255));
            border: 1px solid rgba(255, 255, 255, 80);
        }

        #cancelButton:hover {
            background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1,
                        stop:0 rgba(170, 170, 170, 255),
                        stop:0.5 rgba(180, 180, 180, 255),
                        stop:1 rgba(190, 190, 190, 255));
            border: 1px solid rgba(255, 255, 255, 100);
        }

        #selectCouponButton {
            background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1,
                        stop:0 rgba(120, 180, 220, 255),
                        stop:0.5 rgba(130, 190, 230, 255),
                        stop:1 rgba(140, 200, 240, 255));
            font: 600 12pt "Microsoft YaHei";
            min-height: 36px;
            padding: 8px 20px;
        }

        #selectCouponButton:hover {
            background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1,
                        stop:0 rgba(130, 190, 230, 255),
                        stop:0.5 rgba(140, 200, 240, 255),
                        stop:1 rgba(150, 210, 250, 255));
        }
    )");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(25, 20, 25, 20);

    // 标题
    QLabel* titleLabel = new QLabel("确认付款");
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // 分割线
    QFrame* line1 = new QFrame();
    line1->setFrameShape(QFrame::HLine);
    line1->setFrameShadow(QFrame::Sunken);
    line1->setStyleSheet("QFrame { color: rgba(189, 170, 233, 50); margin: 0px 20px; }");
    mainLayout->addWidget(line1);

    // 订单信息
    lab_orderInfo = new QLabel();
    lab_orderInfo->setObjectName("orderLabel");
    lab_orderInfo->setText(QString("订单号：%1").arg(m_orderId));
    lab_orderInfo->setAlignment(Qt::AlignCenter);
    lab_orderInfo->setWordWrap(true);
    lab_orderInfo->setTextInteractionFlags(Qt::TextSelectableByMouse);
    mainLayout->addWidget(lab_orderInfo);

    // 原价信息
    lab_originalPriceInfo = new QLabel();
    lab_originalPriceInfo->setObjectName("priceLabel");
    lab_originalPriceInfo->setText(QString("商品总价：￥%1").arg(m_originalPrice));
    lab_originalPriceInfo->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(lab_originalPriceInfo);

    // 优惠券选择区域
    QHBoxLayout* couponLayout = new QHBoxLayout();
    couponLayout->setSpacing(10);

    lab_couponInfo = new QLabel("未选择优惠券");
    lab_couponInfo->setObjectName("couponLabel");

    btn_selectCoupon = new QPushButton("选择优惠券");
    btn_selectCoupon->setObjectName("selectCouponButton");
    btn_selectCoupon->setFixedWidth(120);

    couponLayout->addWidget(lab_couponInfo, 1);
    couponLayout->addWidget(btn_selectCoupon);
    mainLayout->addLayout(couponLayout);

    // 优惠信息
    lab_discountInfo = new QLabel();
    lab_discountInfo->setObjectName("discountLabel");
    lab_discountInfo->setAlignment(Qt::AlignCenter);
    lab_discountInfo->hide(); // 初始隐藏
    mainLayout->addWidget(lab_discountInfo);

    // 最终价格
    lab_finalPriceInfo = new QLabel();
    lab_finalPriceInfo->setObjectName("finalPriceLabel");
    lab_finalPriceInfo->setText(QString("付款金额：￥%1").arg(m_finalPrice));
    lab_finalPriceInfo->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(lab_finalPriceInfo);

    // 余额信息
    lab_balanceInfo = new QLabel();
    lab_balanceInfo->setObjectName("balanceLabel");
    lab_balanceInfo->setText(QString("当前余额：￥%1").arg(m_userBalance));
    lab_balanceInfo->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(lab_balanceInfo);

    mainLayout->addSpacing(10);

    // 分割线
    QFrame* line2 = new QFrame();
    line2->setFrameShape(QFrame::HLine);
    line2->setFrameShadow(QFrame::Sunken);
    line2->setStyleSheet("QFrame { color: rgba(189, 170, 233, 50); margin: 0px 20px; }");
    mainLayout->addWidget(line2);

    // 按钮布局
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(25);
    buttonLayout->setContentsMargins(0, 10, 0, 0);

    btn_cancel = new QPushButton("取消");
    btn_cancel->setObjectName("cancelButton");
    btn_confirm = new QPushButton("确认付款");

    buttonLayout->addWidget(btn_cancel);
    buttonLayout->addWidget(btn_confirm);

    mainLayout->addLayout(buttonLayout);

    // 连接信号
    connect(btn_confirm, &QPushButton::clicked, this, &PayDialog::confirmPayment);
    connect(btn_cancel, &QPushButton::clicked, this, &PayDialog::cancelPayment);
    connect(btn_selectCoupon, &QPushButton::clicked, this, &PayDialog::selectCoupon);
}

void PayDialog::queryMerchantDiscounts()
{
    // 这里可以查询商家折扣，暂时假设没有商家折扣
    m_merchantDiscount = 0;
    calculateFinalPrice();
}

void PayDialog::selectCoupon()
{
    if (!m_client || !m_client->getConnected()) {
        QMessageBox::warning(this, "错误", "网络连接失败");
        return;
    }

    CouponSelectDialog* couponDialog = new CouponSelectDialog(m_originalPrice, m_client, this);

    // 连接优惠券选择信号
    connect(couponDialog, &CouponSelectDialog::couponSelected,
            this, &PayDialog::onCouponSelected);

    couponDialog->exec();
    couponDialog->deleteLater();
}

void PayDialog::onCouponSelected(int couponId, int discountAmount)
{
    m_selectedCouponId = couponId;
    m_couponDiscount = discountAmount;

    updatePriceDisplay();
    calculateFinalPrice();

    qDebug() << "PayDialog - 选择优惠券 ID:" << couponId << "折扣:" << discountAmount;
}

void PayDialog::updatePriceDisplay()
{
    if (m_selectedCouponId > 0) {
        lab_couponInfo->setText(QString("已选择优惠券，优惠￥%1").arg(m_couponDiscount));
        lab_couponInfo->setStyleSheet(R"(
            QLabel {
                font: 12pt "Microsoft YaHei";
                color: rgb(70, 130, 70);
                background: rgba(200, 255, 200, 150);
                padding: 8px 15px;
                border-radius: 8px;
                border: 1px solid rgba(100, 200, 100, 50);
            }
        )");
        btn_selectCoupon->setText("重新选择");

        // 显示优惠详情
        int totalDiscount = m_merchantDiscount + m_couponDiscount;
        if (totalDiscount > 0) {
            QString discountText = QString("共优惠：￥%1").arg(totalDiscount);
            if (m_merchantDiscount > 0 && m_couponDiscount > 0) {
                discountText += QString("（商家优惠￥%1 + 优惠券￥%2）").arg(m_merchantDiscount).arg(m_couponDiscount);
            } else if (m_merchantDiscount > 0) {
                discountText += QString("（商家优惠￥%1）").arg(m_merchantDiscount);
            } else {
                discountText += QString("（优惠券￥%1）").arg(m_couponDiscount);
            }
            lab_discountInfo->setText(discountText);
            lab_discountInfo->show();
        }
    } else {
        lab_couponInfo->setText("未选择优惠券");
        lab_couponInfo->setStyleSheet(R"(
            QLabel {
                font: 12pt "Microsoft YaHei";
                color: rgb(120, 110, 140);
                background: rgba(255, 255, 255, 150);
                padding: 8px 15px;
                border-radius: 8px;
                border: 1px solid rgba(189, 170, 233, 25);
            }
        )");
        btn_selectCoupon->setText("选择优惠券");

        if (m_merchantDiscount > 0) {
            lab_discountInfo->setText(QString("商家优惠：￥%1").arg(m_merchantDiscount));
            lab_discountInfo->show();
        } else {
            lab_discountInfo->hide();
        }
    }
}

void PayDialog::calculateFinalPrice()
{
    m_finalPrice = m_originalPrice - m_merchantDiscount - m_couponDiscount;
    if (m_finalPrice < 0) {
        m_finalPrice = 0;
    }

    lab_finalPriceInfo->setText(QString("付款金额：￥%1").arg(m_finalPrice));

    // 更新余额显示颜色
    int balance = m_userBalance.toInt();
    if (balance < m_finalPrice) {
        lab_balanceInfo->setStyleSheet(R"(
            QLabel {
                font: 14pt "Microsoft YaHei";
                color: rgb(180, 60, 60);
                background: rgba(255, 200, 200, 150);
                padding: 10px 15px;
                border-radius: 10px;
                border: 1px solid rgba(255, 100, 100, 50);
            }
        )");
    } else {
        lab_balanceInfo->setStyleSheet(R"(
            QLabel {
                font: 14pt "Microsoft YaHei";
                color: rgb(120, 110, 140);
                background: rgba(255, 255, 255, 150);
                padding: 10px 15px;
                border-radius: 10px;
                border: 1px solid rgba(189, 170, 233, 25);
            }
        )");
    }
}

void PayDialog::confirmPayment()
{
    // 检查余额是否足够
    int balance = m_userBalance.toInt();

    if (balance < m_finalPrice) {
        QMessageBox::warning(this, "余额不足",
                             QString("余额不足！\n\n当前余额：￥%1\n需要金额：￥%2\n缺少金额：￥%3")
                                 .arg(balance).arg(m_finalPrice).arg(m_finalPrice - balance));
        return;
    }

    // 如果使用了优惠券，先验证并使用优惠券
    if (m_selectedCouponId > 0) {
        if (!useCoupon()) {
            return; // 使用优惠券失败
        }
    }

    // 创建确认对话框
    QMessageBox confirmBox(this);
    confirmBox.setWindowTitle("确认付款");
    confirmBox.setIcon(QMessageBox::Question);
    confirmBox.setText("确认付款信息");

    QString infoText = QString("订单号：%1\n原价：￥%2").arg(m_orderId).arg(m_originalPrice);

    if (m_couponDiscount > 0 || m_merchantDiscount > 0) {
        int totalDiscount = m_couponDiscount + m_merchantDiscount;
        infoText += QString("\n优惠金额：￥%1").arg(totalDiscount);
        if (m_couponDiscount > 0 && m_merchantDiscount > 0) {
            infoText += QString("\n  - 优惠券：￥%1\n  - 商家优惠：￥%2").arg(m_couponDiscount).arg(m_merchantDiscount);
        }
    }

    infoText += QString("\n实付金额：￥%1\n付款后余额：￥%2\n\n确认要付款吗？")
                    .arg(m_finalPrice).arg(balance - m_finalPrice);

    confirmBox.setInformativeText(infoText);

    QPushButton* yesButton = confirmBox.addButton("确认付款", QMessageBox::YesRole);
    QPushButton* noButton = confirmBox.addButton("取消", QMessageBox::NoRole);

    // 设置确认框样式
    confirmBox.setStyleSheet(R"(
        QMessageBox {
            background: rgba(252, 250, 255, 255);
            font-family: "Microsoft YaHei";
        }
        QMessageBox QLabel {
            color: rgb(70, 60, 90);
            font: 12pt "Microsoft YaHei";
            padding: 10px;
        }
        QPushButton {
            background: rgba(189, 170, 233, 255);
            border: 1px solid rgba(255, 255, 255, 90);
            border-radius: 15px;
            font: 600 11pt "Microsoft YaHei";
            color: white;
            padding: 8px 20px;
            min-width: 80px;
        }
        QPushButton:hover {
            background: rgba(195, 176, 238, 255);
        }
    )");

    confirmBox.exec();

    if (confirmBox.clickedButton() == yesButton) {
        accept();
    }
}

bool PayDialog::useCoupon()
{
    if (!m_client || !m_client->getConnected()) {
        QMessageBox::warning(this, "错误", "网络连接失败");
        return false;
    }

    // 构造使用优惠券的请求
    QJsonObject requestBody;
    if (m_client->getPerson()) {
        requestBody.insert("user_id", m_client->getPerson()->getId());
    }
    requestBody.insert("coupon_id", m_selectedCouponId);
    requestBody.insert("order_id", m_orderId);
    requestBody.insert("order_amount", m_originalPrice);

    // 发送使用优惠券请求
    QByteArray response = m_client->sendCHTTPMsg("10502", requestBody);

    // 解析响应
    QString head = m_client->parseHead(response);
    if (head.startsWith("1")) { // 成功
        qDebug() << "优惠券使用成功";
        return true;
    } else {
        QString errorMsg = head.mid(1); // 去掉错误代码
        QMessageBox::warning(this, "优惠券使用失败",
                             "使用优惠券失败：" + errorMsg + "\n\n是否继续原价付款？");

        // 重置优惠券信息
        m_selectedCouponId = -1;
        m_couponDiscount = 0;
        updatePriceDisplay();
        calculateFinalPrice();

        return false;
    }
}

void PayDialog::cancelPayment()
{
    reject();
}
