#include "paydialog.h"

PayDialog::PayDialog(const QString& orderId, const QString& orderPrice, const QString& userBalance, QWidget *parent)
    : QDialog(parent), m_orderId(orderId), m_orderPrice(orderPrice), m_userBalance(userBalance)
{
    setWindowTitle("订单付款");
    // 增加弹窗尺寸，确保有足够空间显示长订单号
    setFixedSize(480, 320);

    // 设置窗口属性
    setModal(true);
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);

    setupUI();
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
            max-width: 400px;
        }

        #priceLabel {
            font: bold 20pt "Microsoft YaHei";
            color: rgb(189, 170, 233);
            background: rgba(189, 170, 233, 15);
            padding: 15px 20px;
            border-radius: 15px;
            border: 1px solid rgba(189, 170, 233, 40);
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
            transform: translateY(-1px);
            box-shadow: 0px 4px 15px rgba(189, 170, 233, 0.2);
        }

        QPushButton:pressed {
            background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1,
                        stop:0 rgba(179, 160, 223, 255),
                        stop:0.5 rgba(189, 170, 233, 255),
                        stop:1 rgba(195, 176, 238, 255));
            transform: translateY(0px);
            box-shadow: 0px 2px 8px rgba(189, 170, 233, 0.25);
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
    )");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 25, 30, 25);

    // 标题
    QLabel* titleLabel = new QLabel("确认付款");
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // 添加分割线
    QFrame* line1 = new QFrame();
    line1->setFrameShape(QFrame::HLine);
    line1->setFrameShadow(QFrame::Sunken);
    line1->setStyleSheet("QFrame { color: rgba(189, 170, 233, 50); margin: 0px 20px; }");
    mainLayout->addWidget(line1);

    // 订单信息 - 使用可换行的标签
    lab_orderInfo = new QLabel();
    lab_orderInfo->setObjectName("orderLabel");
    lab_orderInfo->setText(QString("订单号：%1").arg(m_orderId));
    lab_orderInfo->setAlignment(Qt::AlignCenter);
    lab_orderInfo->setWordWrap(true);  // 启用自动换行
    lab_orderInfo->setTextInteractionFlags(Qt::TextSelectableByMouse);  // 允许选择复制
    mainLayout->addWidget(lab_orderInfo);

    // 价格信息
    lab_priceInfo = new QLabel();
    lab_priceInfo->setObjectName("priceLabel");
    lab_priceInfo->setText(QString("付款金额：￥%1").arg(m_orderPrice));
    lab_priceInfo->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(lab_priceInfo);

    // 余额信息
    lab_balanceInfo = new QLabel();
    lab_balanceInfo->setObjectName("balanceLabel");
    lab_balanceInfo->setText(QString("当前余额：￥%1").arg(m_userBalance));
    lab_balanceInfo->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(lab_balanceInfo);

    // 添加弹性空间
    mainLayout->addSpacing(10);

    // 添加分割线
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
}

void PayDialog::confirmPayment()
{
    // 检查余额是否足够
    int balance = m_userBalance.toInt();
    int price = m_orderPrice.toInt();

    if (balance < price) {
        QMessageBox::warning(this, "余额不足",
                             QString("余额不足！\n\n当前余额：￥%1\n需要金额：￥%2\n缺少金额：￥%3")
                                 .arg(balance).arg(price).arg(price - balance));
        return;
    }

    // 创建确认对话框
    QMessageBox confirmBox(this);
    confirmBox.setWindowTitle("确认付款");
    confirmBox.setIcon(QMessageBox::Question);
    confirmBox.setText("确认付款信息");
    confirmBox.setInformativeText(QString("订单号：%1\n付款金额：￥%2\n付款后余额：￥%3\n\n确认要付款吗？")
                                      .arg(m_orderId)
                                      .arg(m_orderPrice)
                                      .arg(balance - price));

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

void PayDialog::cancelPayment()
{
    reject();
}
