#include "cartitem.h"
#include "ui_cartitem.h"
#include <QPixmap>
#include <QLabel>
#include <QDebug>

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

	// 初始化数据
	ui->ProName->setText(name);
	ui->ProPrice->setText(QString("单价: $%1").arg(m_unitPrice));
	ui->ProTolPrice->setText(QString("总价: $%1").arg(totalPrice()));

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
	// 设置图片容器样式
	ui->widMusicPic->setStyleSheet("QWidget{border:1px solid #CCC; background:#F7F7F7;}");

	// 设置SpinBox样式
	ui->spinBox->setStyleSheet(
		"QSpinBox{"
		"min-width:52px;"
		"font:11pt 'Microsoft YaHei';"
		"background:white;"
		"border:1px solid #bbb;"
		"border-radius:4px;"
		"color:#333;"
		"padding-right:15px;"
		"} "
		"QSpinBox::up-button{"
		"subcontrol-origin:border;"
		"subcontrol-position:top right;"
		"width:16px;"
		"border-left:1px solid #bbb;"
		"border-bottom:1px solid #bbb;"
		"background:#f0f0f0;"
		"border-top-right-radius:4px;"
		"}"
		"QSpinBox::down-button{"
		"subcontrol-origin:border;"
		"subcontrol-position:bottom right;"
		"width:16px;"
		"border-left:1px solid #bbb;"
		"background:#f0f0f0;"
		"border-bottom-right-radius:4px;"
		"}"
		"QSpinBox::up-arrow{"
		"image:url(:/images/icons/up-arrow.png);"  // 你的上箭头图片
		"width:8px;"
		"height:8px;"
		"}"
		"QSpinBox::down-arrow{"
		"image:url(:/images/icons/down-arrow.png);"  // 你的下箭头图片
		"width:8px;"
		"height:8px;"
		"}"
	);
	ui->spinBox->setFixedHeight(32);
	// 确保控件启用
	ui->spinBox->setEnabled(true);
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

	// 创建一个QLabel来显示图片
	QLabel* picLabel = ui->widMusicPic->findChild<QLabel*>();
	if (!picLabel) {
		picLabel = new QLabel(ui->widMusicPic);
		picLabel->setGeometry(0, 0, 60, 60);
		picLabel->setAlignment(Qt::AlignCenter);
	}

	picLabel->setPixmap(pix.scaled(60, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation));
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

	// 更新总价显示
	ui->ProTolPrice->setText(QString("总价: $%1").arg(value * m_unitPrice));

	// 发送数量变化信号
	emit sigQuantityChanged(m_id, value);

	// 如果值为0，自动取消选中
	if (value == 0) {
		ui->radioButton->setChecked(false);
	}
}

void CartItem::onChecked(bool c)
{
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
		ui->ProTolPrice->setText(QString("更新中… $%1").arg(totalPrice()));
	}
	else {
		ui->ProTolPrice->setText(QString("总价: $%1").arg(totalPrice()));
	}
}