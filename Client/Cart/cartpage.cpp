#include "cartpage.h"
#include "ui_cartpage.h"

#include <QListWidgetItem>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QDebug>
#include <QTimer>

#include "cartitem.h"
#include "../Shop/product.h"
#include "../shoppingclient.h"

extern QString FLAG_CHARACTER;

static const int DEBOUNCE_MS = 300;

CartPage::CartPage(QWidget* parent)
	: QWidget(parent),
	ui(new Ui::CartPage)
{
	ui->setupUi(this);
	ui->list_item->setStyleSheet(
		"QListWidget{border:1px solid #CCC;background:#FAFAFA;border-radius:6px;}"
		"QListWidget::item{height:86px;}"
		"QListWidget::item:selected{background:#EDEAFF;}"
	);

	m_total = 0;
	ui->lab_tolprice->setText(QString("总价: $%1").arg(m_total));

	connect(ui->btn_buy, &QPushButton::clicked, this, &CartPage::buySth);
	connect(this, &CartPage::signal_updateCartMoney, this, &CartPage::updateCartMoney);
}

CartPage::~CartPage()
{
	clearAll();
	delete ui;
}

void CartPage::resetCache()
{
	clearAll();
	m_cached = false;
}

void CartPage::addToCart(Product* product)
{
	if (!client || !client->getPerson() || client->getPerson()->getId() <= 0) {
		emit signal_addToCartResult(false, "请先登录");
		return;
	}
	if (!product) {
		emit signal_addToCartResult(false, "商品信息无效");
		return;
	}

	if (!m_cached) {
		loadCart();
		m_cached = true;
	}

	if (m_items.contains(product->id)) {
		// 已存在 => 数量+1
		CartItem* ci = m_items.value(product->id);
		int newQty = ci->currentQty() + 1;
		ci->setQuantity(newQty);

		QString message = QString("商品 \"%1\" 数量已更新为 %2！").arg(product->name).arg(newQty);
		emit signal_addToCartResult(true, message);
		return;
	}

	// 新商品添加
	QJsonObject obj;
	obj.insert("Cartitem_pro_id", QString::number(product->id));
	obj.insert("Cartitem_user_id", QString::number(client->getPerson()->getId()));
	obj.insert("Cartitem_num", "1");

	QByteArray data = client->sendCHTTPMsg("10301", obj);
	QString flag = client->parseHead(data);
	if (flag.isEmpty() || flag.at(0) != QLatin1Char('1')) {
		QString errorMsg = flag.isEmpty() ? QString("服务器无响应") : flag.mid(1);
		emit signal_addToCartResult(false, errorMsg);
		client->error(flag.isEmpty() ? QChar('?') : flag.at(0), errorMsg);
		return;
	}

	auto* lwItem = new QListWidgetItem(ui->list_item);
	auto* ci = new CartItem(product->id, product->pictureaddress, product->name, product->price, 1);
	ci->listItem = lwItem;

	connect(ci, &CartItem::sigQuantityChanged, this, &CartPage::onItemQuantityChanged);
	connect(ci, &CartItem::sigSelectionChanged, this, &CartPage::onItemSelectionChanged);
	connect(ci, &CartItem::sigDeleteRequest, this, &CartPage::onItemDelete);

	ui->list_item->setItemWidget(lwItem, ci);
	m_items.insert(product->id, ci);

	recalcTotal();

	QString successMsg = QString("商品 \"%1\" 加入购物车成功！").arg(product->name);
	emit signal_addToCartResult(true, successMsg);
}

void CartPage::clearAll()
{
	for (auto t : m_debounce) {
		if (t) {
			t->stop();
			t->deleteLater();
		}
	}
	m_debounce.clear();
	m_pending.clear();

	for (auto it = m_items.begin(); it != m_items.end(); ++it) {
		CartItem* c = it.value();
		if (!c) continue;
		if (c->listItem) {
			ui->list_item->removeItemWidget(c->listItem);
			delete c->listItem;
		}
		delete c;
	}
	m_items.clear();
	m_total = 0;
	ui->lab_tolprice->setText("总价: $0");
}

void CartPage::showEvent(QShowEvent* e)
{
	if (!m_cached) {
		loadCart();
		m_cached = true;
	}
	QWidget::showEvent(e);
}

void CartPage::loadCart()
{
	if (!client || !client->getPerson() || client->getPerson()->getId() <= 0) {
		qWarning() << "[CartPage] loadCart invalid state";
		return;
	}

	QJsonObject obj;
	obj.insert("want",
		"Cartitem_pro_id AS cart_pro_id,"
		"Product_name AS pro_name,"
		"Product_price AS pro_price,"
		"Product_pictureaddress AS pro_pictureaddress,"
		"Cartitem_num AS cart_num");
	obj.insert("restriction",
		QString("Products.Product_id = Cartitem_pro_id AND Cartitem_user_id = %1")
		.arg(client->getPerson()->getId()));

	QByteArray data = client->sendCHTTPMsg("10304", obj);
	QString flag = client->parseHead(data);
	if (flag.isEmpty()) {
		client->error(QChar('?'), "响应空");
		return;
	}
	if (flag.at(0) != QLatin1Char('1')) {
		client->error(flag.at(0), flag.mid(1));
		return;
	}

	QJsonArray arr = client->parseResponse(data);
	for (auto v : arr) {
		QJsonObject o = v.toObject();
		int id = o.value("cart_pro_id").toString().toInt();
		int qty = o.value("cart_num").toString().toInt();
		int price = o.value("pro_price").toString().toInt();
		QString name = o.value("pro_name").toString();
		QString pic = o.value("pro_pictureaddress").toString();

		if (m_items.contains(id)) continue;

		auto* lwItem = new QListWidgetItem(ui->list_item);
		auto* ci = new CartItem(id, pic, name, price, qty);
		ci->listItem = lwItem;

		connect(ci, &CartItem::sigQuantityChanged,
			this, &CartPage::onItemQuantityChanged);
		connect(ci, &CartItem::sigSelectionChanged,
			this, &CartPage::onItemSelectionChanged);
		connect(ci, &CartItem::sigDeleteRequest,
			this, &CartPage::onItemDelete);

		ui->list_item->setItemWidget(lwItem, ci);
		m_items.insert(id, ci);
	}

	recalcTotal();
}

void CartPage::recalcTotal()
{
	int total = 0;
	for (auto it = m_items.begin(); it != m_items.end(); ++it) {
		CartItem* ci = it.value();
		if (!ci) continue;
		if (ci->isChecked()) {
			total += ci->unitPrice() * ci->currentQty();
		}
	}
	m_total = total;
	ui->lab_tolprice->setText(QString("总价: $%1").arg(m_total));
}



void CartPage::onItemQuantityChanged(int proId, int newQty)
{
	if (!m_items.contains(proId)) return;

	if (newQty == 0) {
		// 直接删除，但要确保在当前信号处理完成后再删除
		QTimer::singleShot(0, this, [this, proId]() {
			onItemDelete(proId);
			});
		return;
	}

	recalcTotal();     // 乐观立即刷新
	scheduleDebounce(proId);
}

void CartPage::onItemSelectionChanged(int /*proId*/, bool /*checked*/)
{
	recalcTotal();
}

void CartPage::onItemDelete(int proId)
{
	if (!m_items.contains(proId)) return;

	// 服务器删除
	deleteCartServer(proId);
}

void CartPage::scheduleDebounce(int proId)
{
	QTimer* t = nullptr;
	if (!m_debounce.contains(proId)) {
		t = new QTimer(this);
		t->setSingleShot(true);
		m_debounce.insert(proId, t);
		connect(t, &QTimer::timeout, this, [this, proId]() {
			onCommitQuantity(proId);
			});
	}
	else {
		t = m_debounce.value(proId);
	}
	t->start(DEBOUNCE_MS);
}

void CartPage::onCommitQuantity(int proId)
{
	if (!m_items.contains(proId)) return;
	CartItem* ci = m_items.value(proId);
	int latest = ci->currentQty();
	if (latest == ci->confirmedQty()) {
		return; // 没变化
	}
	commitQuantityToServer(proId, latest);
}

void CartPage::commitQuantityToServer(int proId, int newQty)
{
	if (!client || !client->getPerson()) return;
	if (!m_items.contains(proId)) return;
	if (m_pending.contains(proId)) return;

	CartItem* ci = m_items.value(proId);
	m_pending.insert(proId);
	ci->setLoading(true);

	QJsonObject obj;
	obj.insert("Cartitem_pro_id", QString::number(proId));
	obj.insert("Cartitem_user_id", QString::number(client->getPerson()->getId()));
	obj.insert("Cartitem_num", QString::number(newQty));
	obj.insert("restriction",
		QString("Cartitem_user_id = '%1' AND Cartitem_pro_id = '%2'")
		.arg(client->getPerson()->getId()).arg(proId));

	QByteArray data = client->sendCHTTPMsg("10303", obj);
	QString flag = client->parseHead(data);
	bool ok = (!flag.isEmpty() && flag.at(0) == QLatin1Char('1'));

	if (ok) {
		ci->setConfirmedQty(newQty);
	}
	else {
		// 回滚
		int rollback = ci->confirmedQty();
		ci->blockSignals(true);
		ci->onSpinChanged(rollback); // 会再次发信号但被阻断

		ci->updateTotalLabel(QString("总价: $%1").arg(rollback * ci->unitPrice()));

		ci->blockSignals(false);
		recalcTotal();
		client->error(flag.isEmpty() ? QChar('?') : flag.at(0),
			flag.isEmpty() ? QString("返回空") : flag.mid(1));
	}

	ci->setLoading(false);
	m_pending.remove(proId);
	recalcTotal();
}

void CartPage::deleteCartServer(int proId)
{
	if (!client || !client->getPerson()) return;

	QJsonObject obj;
	obj.insert("restriction",
		QString("Cartitem_user_id = '%1' AND Cartitem_pro_id = '%2'")
		.arg(client->getPerson()->getId()).arg(proId));

	QByteArray data = client->sendCHTTPMsg("10302", obj);
	QString flag = client->parseHead(data);
	if (flag.isEmpty() || flag.at(0) != QLatin1Char('1')) {
		client->error(flag.isEmpty() ? QChar('?') : flag.at(0),
			flag.isEmpty() ? QString("返回空") : flag.mid(1));
		return;
	}
	removeItemLocal(proId);
	recalcTotal();
}

void CartPage::removeItemLocal(int proId)
{
	// 停止防抖
	if (m_debounce.contains(proId)) {
		QTimer* timer = m_debounce.value(proId);
		if (timer) {
			timer->stop();
			timer->deleteLater();
		}
		m_debounce.remove(proId);
	}
	m_pending.remove(proId);

	if (!m_items.contains(proId)) return;

	CartItem* ci = m_items.value(proId);
	if (!ci) return;

	// 先断开信号连接，防止删除过程中触发信号
	disconnect(ci, nullptr, this, nullptr);

	// 从列表中移除
	if (ci->listItem) {
		ui->list_item->removeItemWidget(ci->listItem);
		delete ci->listItem;
		ci->listItem = nullptr;
	}

	// 从哈希表中移除
	m_items.remove(proId);

	// 最后删除对象
	ci->deleteLater();  // 使用 deleteLater 而不是 delete
}

void CartPage::buySth()
{
	if (!client || !client->getPerson()) return;

	QJsonArray toBuy;
	for (auto id : m_items.keys()) {
		CartItem* ci = m_items.value(id);
		if (ci && ci->isChecked()) toBuy.append(QString::number(id));
	}
	if (toBuy.isEmpty()) {
		QMessageBox::information(this, "提示", "未选择商品！");
		return;
	}

	if (QMessageBox::question(this, "确认", "确定结算选中商品？",
		QMessageBox::Yes | QMessageBox::Cancel,
		QMessageBox::Cancel) != QMessageBox::Yes) {
		return;
	}

	QJsonObject obj;
	obj.insert("type", "cart");
	obj.insert("user_id", QString::number(client->getPerson()->getId()));
	obj.insert("wannabuy", toBuy);

	QByteArray data = client->sendCHTTPMsg(FLAG_CHARACTER + "0205", obj);
	QString flag = client->parseHead(data);
	if (flag.isEmpty() || flag.at(0) != QLatin1Char('1')) {
		client->error(flag.isEmpty() ? QChar('?') : flag.at(0),
			flag.isEmpty() ? QString("返回空") : flag.mid(1));
		return;
	}

	QJsonArray resp = client->parseResponse(data);
	if (!resp.isEmpty()) {
		emit signal_updateUserMoney(resp[0].toInt());
		if (resp.size() >= 2)
			emit signal_updateCartMoney(resp[1].toInt());
	}

	// 移除已购买
	for (auto v : toBuy) {
		int id = v.toString().toInt();
		removeItemLocal(id);
	}

	recalcTotal();
	QMessageBox::information(this, "提示", "购买成功！");
}

void CartPage::updateCartMoney(int /*change*/)
{
	// 兼容旧接口：忽略 delta，直接重算（此处 change = 本次消费金额）
	recalcTotal();
}