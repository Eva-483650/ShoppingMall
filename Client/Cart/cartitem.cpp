#include "cartitem.h"
#include "ui_cartitem.h"
#include <QPixmap>
#include <QLabel>
#include <QDebug>
#include <QPainter>
 #include <QPainterPath>

CartItem::CartItem(int id,
                   const QString& pictureAddress,
                   const QString& name,
                   int unitPrice,
                   int initQty,
                   QWidget* parent)
    : QWidget(parent),
    ui(new Ui::CartItem),
    m_id(id),
    m_unitPrice(unitPrice),
    m_confirmedQty(initQty)
{
    ui->setupUi(this);
    setupUI();

    // 安装事件过滤器
    ui->spinBox->installEventFilter(this);

    // 初始化数据
    ui->ProName->setText(name);
    ui->ProPrice->setText(QString("￥%1").arg(m_unitPrice));
    ui->ProTolPrice->setText(QString("￥%1").arg(totalPrice()));

    // 设置SpinBox范围和初始值
    ui->spinBox->setMinimum(0);  // 允许为0，表示删除
    ui->spinBox->setMaximum(9999);
    ui->spinBox->setValue(initQty < 0 ? 0 : initQty);

    setPicture(pictureAddress);

    // 连接信号槽
    connect(ui->spinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &CartItem::onSpinChanged);
    connect(ui->radioButton, &QRadioButton::toggled,
            this, &CartItem::onChecked);
}

CartItem::~CartItem()
{
    // 断开所有信号连接，防止析构后还收到信号
    if (ui && ui->spinBox) {
        disconnect(ui->spinBox, nullptr, this, nullptr);
    }
    if (ui && ui->radioButton) {
        disconnect(ui->radioButton, nullptr, this, nullptr);
    }

    // 安全删除ui
    if (ui) {
        delete ui;
        ui = nullptr;
    }
}

void CartItem::setupUI()
{
    // 移除会覆盖UI文件样式的代码，只保留必要的设置

    // 不再设置图片容器样式，使用UI文件中的样式
    // ui->widMusicPic->setStyleSheet("QWidget{border:1px solid #CCC; background:#F7F7F7;}");

    // 简化SpinBox样式，与UI文件协调
    // 移除 setFixedHeight，让UI文件的布局决定高度
    // ui->spinBox->setFixedHeight(32);

    // 确保控件启用
    ui->spinBox->setEnabled(true);

    // 设置最小尺寸以确保正确显示
    this->setMinimumHeight(110);

    // 设置CartItem的基础样式，与列表配合
    this->setStyleSheet(
        "CartItem {"
        "    background: transparent;"
        "    border: none;"
        "    border-radius: 12px;"
        "    font-family: 'Microsoft YaHei';"
        "    margin: 2px;"
        "}"
        );

    QString spinBoxStyle = R"(
    QSpinBox {
        background: rgba(120, 180, 220, 15);
        border: 1px solid rgba(120, 180, 220, 40);
        border-radius: 12px;
        padding: 4px 8px;
        color: rgb(120, 180, 220);
        font: bold 11pt "Microsoft YaHei";
        min-width: 70px;
    }

    QSpinBox:hover {
        background: rgba(120, 180, 220, 25);
        border: 1px solid rgba(120, 180, 220, 60);
    }

    QSpinBox:focus {
        background: rgba(120, 180, 220, 20);
        border: 2px solid rgba(120, 180, 220, 80);
    }

    QSpinBox::up-button {
        subcontrol-origin: border;
        subcontrol-position: top right;
        width: 20px;
        height: 20px;
        border-left: 1px solid rgba(120, 180, 220, 60);
        border-bottom: 1px solid rgba(120, 180, 220, 40);
        background: rgba(120, 180, 220, 20);
        border-top-right-radius: 8px;
    }

    QSpinBox::down-button {
        subcontrol-origin: border;
        subcontrol-position: bottom right;
        width: 20px;
        height: 20px;
        border-left: 1px solid rgba(120, 180, 220, 60);
        background: rgba(120, 180, 220, 20);
        border-bottom-right-radius: 8px;
    }

    QSpinBox::up-button:hover, QSpinBox::down-button:hover {
        background: rgba(120, 180, 220, 40);
    }

    QSpinBox::up-button:pressed, QSpinBox::down-button:pressed {
        background: rgba(120, 180, 220, 60);
    }

    /* 使用Unicode字符作为箭头 */
    QSpinBox::up-arrow {
        image: none;
        border: none;
        width: 0px;
        height: 0px;
    }

    QSpinBox::down-arrow {
        image: none;
        border: none;
        width: 0px;
        height: 0px;
    }

    QSpinBox::up-button::after {
        content: "+";
        color: rgb(120, 180, 220);
        font: bold 10pt "Microsoft YaHei";
        text-align: center;
        padding-top: 2px;
    }

    QSpinBox::down-button::after {
        content: "-";
        color: rgb(120, 180, 220);
        font: bold 10pt "Microsoft YaHei";
        text-align: center;
        padding-top: 2px;
    }
)";

    ui->spinBox->setStyleSheet(spinBoxStyle);

    // 确保控件启用
    ui->spinBox->setEnabled(true);

    // 设置最小尺寸以确保正确显示
    this->setMinimumHeight(110);
    this->setMaximumHeight(110); // 添加最大高度限制
}

// 添加事件过滤器函数
bool CartItem::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->spinBox && event->type() == QEvent::Paint) {
        // 先让SpinBox正常绘制
        QSpinBox *spinBox = qobject_cast<QSpinBox*>(obj);
        if (spinBox) {
            // 调用默认绘制
            bool result = QWidget::eventFilter(obj, event);

            // 然后在按钮上绘制我们的图标
            QPainter painter(spinBox);
            painter.setRenderHint(QPainter::Antialiasing);

            // 计算按钮位置
            int buttonWidth = 20;
            int buttonHeight = spinBox->height() / 2;

            // 上按钮区域
            QRect upRect(spinBox->width() - buttonWidth, 0, buttonWidth, buttonHeight);
            // 下按钮区域
            QRect downRect(spinBox->width() - buttonWidth, buttonHeight, buttonWidth, buttonHeight);

            // 设置字体和颜色
            painter.setPen(QColor(120, 180, 220));
            painter.setFont(QFont("Arial", 12, QFont::Bold));

            // 绘制符号
            painter.drawText(upRect, Qt::AlignCenter, "+");
            painter.drawText(downRect, Qt::AlignCenter, "−");

            return result;
        }
    }
    return QWidget::eventFilter(obj, event);
}

int CartItem::currentQty() const
{
    return ui->spinBox->value();
}

bool CartItem::isChecked() const
{
    return ui->radioButton->isChecked();
}

void CartItem::setQuantity(int qty)
{
    ui->spinBox->setValue(qty);
}

void CartItem::updateTotalLabel(const QString& text)
{
    ui->ProTolPrice->setText(text);
}

void CartItem::setPicture(const QString& relPath)
{
    QString full = ":/images/products/" + relPath;
    QPixmap pix(full);
    if (pix.isNull()) {
        pix.load(":/images/products/picerror.jpg");
    }

    // 创建圆角矩形图片
    QPixmap roundedPix(60, 60);
    roundedPix.fill(Qt::transparent);

    QPainter painter(&roundedPix);
    painter.setRenderHint(QPainter::Antialiasing);

    // 创建圆角矩形路径
    QPainterPath path;
    path.addRoundedRect(0, 0, 60, 60, 8, 8);  // 圆角半径为8像素
    painter.setClipPath(path);

    // 绘制缩放后的图片
    QPixmap scaledPix = pix.scaled(60, 60, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

    // 如果图片比例不是正方形，需要居中裁剪
    if (scaledPix.width() > 60 || scaledPix.height() > 60) {
        int x = (scaledPix.width() - 60) / 2;
        int y = (scaledPix.height() - 60) / 2;
        scaledPix = scaledPix.copy(x, y, 60, 60);
    }

    painter.drawPixmap(0, 0, scaledPix);
    painter.end();

    // 创建一个QLabel来显示图片
    QLabel* picLabel = ui->widMusicPic->findChild<QLabel*>();
    if (!picLabel) {
        picLabel = new QLabel(ui->widMusicPic);
        picLabel->setGeometry(5, 5, 60, 60);
        picLabel->setAlignment(Qt::AlignCenter);
        picLabel->setStyleSheet("QLabel { background: transparent; border: none; }");
    }

    // 设置圆角矩形图片
    picLabel->setPixmap(roundedPix);
}

void CartItem::onSpinChanged(int value)
{
    // 添加安全检查
    if (!ui) {
        qWarning() << "CartItem::onSpinChanged - ui is null!";
        return;
    }

    // 检查ui指针是否有效
    if (reinterpret_cast<quintptr>(ui) == 0xFFFFFFFFFFFFFFF7) {
        qWarning() << "CartItem::onSpinChanged - ui pointer corrupted!";
        return;
    }

    // 更新总价显示 - 使用人民币符号
    ui->ProTolPrice->setText(QString("￥%1").arg(value * m_unitPrice));

    // 发送数量变化信号
    emit sigQuantityChanged(m_id, value);

    // 如果值为0，自动取消选中
    if (value == 0) {
        ui->radioButton->setChecked(false);
    }
}

void CartItem::onChecked(bool c)
{
    // 根据RadioButton状态更新背景色
    if (c) {
        this->setStyleSheet(
            "CartItem {"
            "    background: rgba(240, 230, 248, 180);"
            "    border: none;"
            "    border-radius: 12px;"
            "    font-family: 'Microsoft YaHei';"
            "    margin: 2px;"
            "}"
            );
    } else {
        this->setStyleSheet(
            "CartItem {"
            "    background: transparent;"
            "    border: none;"
            "    border-radius: 12px;"
            "    font-family: 'Microsoft YaHei';"
            "    margin: 2px;"
            "}"
            );
    }
    emit sigSelectionChanged(m_id, c);
}

void CartItem::onDeleteClicked()
{
    emit sigDeleteRequest(m_id);
}

void CartItem::setLoading(bool on)
{
    if (m_loading == on) return;
    m_loading = on;

    ui->spinBox->setEnabled(!on);

    if (on) {
        ui->ProTolPrice->setText(QString("更新中… ￥%1").arg(totalPrice()));
    }
    else {
        ui->ProTolPrice->setText(QString("￥%1").arg(totalPrice()));
    }
}
