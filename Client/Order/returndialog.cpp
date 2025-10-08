#include "returndialog.h"

ReturnDialog::ReturnDialog(const QString& orderId, const QString& orderPrice,
                           ShoppingClient* client, QWidget *parent)
    : QDialog(parent)
    , m_orderId(orderId)
    , m_orderPrice(orderPrice)
    , m_client(client)
{
    setWindowTitle("申请退货");
    setFixedSize(480, 450);  // 增加高度从380到450
    setModal(true);

    setupUI();
    setupStyles();
}

void ReturnDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(25, 25, 25, 25);

    // 标题
    QLabel* titleLabel = new QLabel("申请退货", this);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // 订单信息
    QHBoxLayout* orderLayout = new QHBoxLayout();
    QLabel* orderTextLabel = new QLabel("订单号:", this);
    orderTextLabel->setMinimumWidth(80);
    m_orderLabel = new QLabel(m_orderId, this);
    m_orderLabel->setObjectName("orderLabel");
    m_orderLabel->setWordWrap(true);
    orderLayout->addWidget(orderTextLabel);
    orderLayout->addWidget(m_orderLabel);
    orderLayout->addStretch();
    mainLayout->addLayout(orderLayout);

    // 价格信息
    QHBoxLayout* priceLayout = new QHBoxLayout();
    QLabel* priceTextLabel = new QLabel("订单金额:", this);
    priceTextLabel->setMinimumWidth(80);
    m_priceLabel = new QLabel("￥" + m_orderPrice, this);
    m_priceLabel->setObjectName("priceLabel");
    priceLayout->addWidget(priceTextLabel);
    priceLayout->addWidget(m_priceLabel);
    priceLayout->addStretch();
    mainLayout->addLayout(priceLayout);

    // 退货类型 - 使用自定义布局替代QComboBox
    QHBoxLayout* typeLayout = new QHBoxLayout();
    QLabel* typeLabel = new QLabel("退货类型:", this);
    typeLabel->setMinimumWidth(80);

    // 创建一个容器模拟下拉框
    QWidget* comboContainer = new QWidget(this);
    comboContainer->setObjectName("comboContainer");
    comboContainer->setMinimumHeight(35);
    comboContainer->setMinimumWidth(150);

    QHBoxLayout* comboLayout = new QHBoxLayout(comboContainer);
    comboLayout->setContentsMargins(10, 0, 10, 0);

    m_typeCombo = new QComboBox(this);
    m_typeCombo->setObjectName("typeCombo");
    m_typeCombo->addItem("仅退款");
    m_typeCombo->addItem("退货退款");
    m_typeCombo->addItem("换货");
    m_typeCombo->setMinimumHeight(35);
    m_typeCombo->setMinimumWidth(150);

    // 添加一个标签显示箭头
    QLabel* arrowLabel = new QLabel("▼", this);  // 使用Unicode下箭头
    arrowLabel->setObjectName("arrowLabel");
    arrowLabel->setAlignment(Qt::AlignCenter);
    arrowLabel->setFixedSize(20, 20);

    comboLayout->addWidget(m_typeCombo);
    comboLayout->addWidget(arrowLabel);

    typeLayout->addWidget(typeLabel);
    typeLayout->addWidget(comboContainer);
    typeLayout->addStretch();
    mainLayout->addLayout(typeLayout);

    // 退货原因
    QLabel* reasonLabel = new QLabel("退货原因:", this);
    mainLayout->addWidget(reasonLabel);

    m_reasonEdit = new QTextEdit(this);
    m_reasonEdit->setObjectName("reasonEdit");
    m_reasonEdit->setPlaceholderText("请详细描述退货原因...");
    m_reasonEdit->setMinimumHeight(120);
    m_reasonEdit->setMaximumHeight(120);
    mainLayout->addWidget(m_reasonEdit);

    // 按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_cancelBtn = new QPushButton("取消", this);
    m_cancelBtn->setObjectName("cancelBtn");
    m_cancelBtn->setMinimumHeight(40);
    m_confirmBtn = new QPushButton("确认退货", this);
    m_confirmBtn->setObjectName("confirmBtn");
    m_confirmBtn->setMinimumHeight(40);

    buttonLayout->addWidget(m_cancelBtn);
    buttonLayout->addWidget(m_confirmBtn);
    mainLayout->addLayout(buttonLayout);

    // 连接信号
    connect(m_confirmBtn, &QPushButton::clicked, this, &ReturnDialog::onConfirmReturn);
    connect(m_cancelBtn, &QPushButton::clicked, this, &ReturnDialog::onCancel);
}

void ReturnDialog::setupStyles()
{
    setStyleSheet(R"(
        QDialog {
            background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1,
                        stop:0 rgba(252, 250, 255, 255),
                        stop:0.5 rgba(250, 247, 254, 255),
                        stop:1 rgba(248, 245, 253, 255));
            font-family: "Microsoft YaHei";
        }

        #titleLabel {
            font: bold 18pt "Microsoft YaHei";
            color: rgb(110, 90, 140);
            padding: 10px;
        }

        QLabel {
            font: 12pt "Microsoft YaHei";
            color: rgb(110, 100, 130);
        }

        #orderLabel, #priceLabel {
            font: bold 12pt "Microsoft YaHei";
            color: rgb(70, 60, 90);
            background: rgba(255, 255, 255, 220);
            padding: 8px 12px;
            border-radius: 8px;
            border: 1px solid rgba(189, 170, 233, 60);
        }

        #comboContainer {
            background: rgba(255, 255, 255, 240);
            border: 2px solid rgba(189, 170, 233, 80);
            border-radius: 8px;
        }

        #typeCombo {
            font: 12pt "Microsoft YaHei";
            color: rgb(70, 60, 90);
            background: transparent;
            border: none;
            padding: 8px 10px;
        }

        #typeCombo::drop-down {
            border: none;
            background: transparent;
        }

        #typeCombo::down-arrow {
            image: none;
            border: none;
        }

        #arrowLabel {
            font: 14pt "Microsoft YaHei";
            color: rgb(70, 60, 90);
            background: transparent;
            border: none;
        }

        #typeCombo QAbstractItemView {
            background: rgba(255, 255, 255, 250);
            border: 2px solid rgba(189, 170, 233, 100);
            border-radius: 8px;
            selection-background-color: rgba(189, 170, 233, 120);
            color: rgb(70, 60, 90);
            font: 12pt "Microsoft YaHei";
            padding: 5px;
        }

        #typeCombo QAbstractItemView::item {
            height: 34px;
            padding: 5px 6px;
            color: rgb(70, 60, 90);
            background: transparent;
        }

        #typeCombo QAbstractItemView::item:hover {
            background: rgba(189, 170, 233, 80);
            color: rgb(50, 40, 70);
        }

        #typeCombo QAbstractItemView::item:selected {
            background: rgba(189, 170, 233, 120);
            color: rgb(50, 40, 70);
        }

        #reasonEdit {
            font: 11pt "Microsoft YaHei";
            color: rgb(70, 60, 90);
            background: rgba(255, 255, 255, 240);
            border: 2px solid rgba(189, 170, 233, 80);
            border-radius: 8px;
            padding: 10px;
            line-height: 1.4;
        }

        #reasonEdit:focus {
            border: 2px solid rgba(189, 170, 233, 150);
        }

        #confirmBtn {
            background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1,
                        stop:0 rgba(220, 53, 69, 255),
                        stop:0.5 rgba(240, 73, 89, 255),
                        stop:1 rgba(255, 93, 109, 255));
            border: 1px solid rgba(255, 255, 255, 90);
            border-radius: 20px;
            font: bold 12pt "Microsoft YaHei";
            color: white;
            padding: 10px 25px;
            min-width: 100px;
            min-height: 30px;
        }

        #confirmBtn:hover {
            background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1,
                        stop:0 rgba(240, 73, 89, 255),
                        stop:0.5 rgba(255, 93, 109, 255),
                        stop:1 rgba(255, 113, 129, 255));
        }

        #cancelBtn {
            background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1,
                        stop:0 rgba(189, 170, 233, 255),
                        stop:0.5 rgba(195, 176, 238, 255),
                        stop:1 rgba(201, 182, 243, 255));
            border: 1px solid rgba(255, 255, 255, 90);
            border-radius: 20px;
            font: 600 12pt "Microsoft YaHei";
            color: white;
            padding: 10px 25px;
            min-width: 60px;
            min-height: 30px;
        }

        #cancelBtn:hover {
            background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1,
                        stop:0 rgba(195, 176, 238, 255),
                        stop:0.5 rgba(201, 182, 243, 255),
                        stop:1 rgba(207, 188, 248, 255));
        }
    )");
}

void ReturnDialog::onConfirmReturn()
{
    m_returnReason = m_reasonEdit->toPlainText().trimmed();
    m_returnType = m_typeCombo->currentText();

    if (m_returnReason.isEmpty()) {
        QMessageBox::warning(this, "提示", "请填写退货原因！");
        return;
    }

    if (m_returnReason.length() < 10) {
        QMessageBox::warning(this, "提示", "退货原因请至少填写10个字符！");
        return;
    }

    // 确认退货
    QMessageBox::StandardButton reply = QMessageBox::question(this, "确认退货",
                                                              QString("确认要申请退货吗？\n\n订单号：%1\n退货类型：%2\n退货原因：%3")
                                                                  .arg(m_orderId).arg(m_returnType).arg(m_returnReason),
                                                              QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        accept();
    }
}

void ReturnDialog::onCancel()
{
    reject();
}
