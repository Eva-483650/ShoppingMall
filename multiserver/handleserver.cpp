#include "handleserver.h"
#include <QtDebug>

HandleServer::HandleServer(SQLServer* sqlserver)
{
	this->sql = sqlserver;
}

void HandleServer::handleRequest(const QString& ip, const qintptr port, const QByteArray data) {
	QJsonParseError jsonError;
	QJsonDocument document = QJsonDocument::fromJson(data, &jsonError);
	if (!document.isNull() && (jsonError.error == QJsonParseError::NoError)) {
		if (document.isObject()) {
			QJsonObject object = document.object();
			QString strHead;
			if (object.contains("head")) {
				strHead = object.value("head").toString();
				if (strHead.length() != 5) { jsonResReady("3", QJsonArray(), port, "协议码长度错误！"); return; }
				qDebug() << "head : " << strHead;
			}
			else { jsonResReady("3", QJsonArray(), port, "没有报文头部！"); return; }

			if (object.contains("body")) {
				int flag_character = strHead.mid(0, 1).toInt();
				int flag_inskind = strHead.mid(1, 2).toInt();
				int flag_ins = strHead.mid(3, 2).toInt();
				QJsonObject body = object.value("body").toObject();

				switch (flag_inskind) {
				case 1:
					switch (flag_ins) {
					case 1:
						if (flag_character == 1) handleUserLogin(body, port);
						else if (flag_character == 2) handleManagerLogin(body, port);
						else jsonResReady("2", QJsonArray(), port, "不合法的端标识！");
						break;
					case 2:
						handleRegister(body, port);
						break;
					default:
						jsonResReady("2", QJsonArray(), port, "不合法的报文！");
						break;
					}
					break;
				case 2:
					switch (flag_ins) {
					case 4: handleSearchProduct(body, port); break;
					case 5: handleBuySth(body, port); break;
					default: jsonResReady("2", QJsonArray(), port, "未知商品操作"); break;
					}
					break;
				case 3:
					switch (flag_ins) {
					case 1: handleAddCart(body, port); break;
					case 2: handleDelCart(body, port); break;
					case 3: handleUpdateCart(body, port); break;
					case 4: handleSearchCart(body, port); break;
					default: jsonResReady("2", QJsonArray(), port, "未知购物车操作"); break;
					}
					break;
				case 4:
					switch (flag_ins) {
					case 4: handleSearchOrder(body, port); break;
					case 5: handleSearchOrderItems(body, port); break;
					default: jsonResReady("2", QJsonArray(), port, "未知订单操作"); break;
					}
					break;
				default:
					jsonResReady("2", QJsonArray(), port, "未知行为类型");
					break;
				}
			}
			else { jsonResReady("3", QJsonArray(), port, "没有报文主体！"); return; }
		}
	}
	else { jsonResReady("3", QJsonArray(), port, "解析错误！"); return; }
}

void HandleServer::jsonResReady(QString head, QJsonArray res, qintptr port, QString errmsg) {
	QJsonObject response;
	response.insert("head", head);
	if (head.size() > 0 && head.at(0) == QLatin1Char('3')) {
		response.insert("error", errmsg);
	}
	else if (head.size() > 0 && head.at(0) == QLatin1Char('2')) {
		response.insert("error", errmsg);
	}
	else if (head.size() > 0 && head.at(0) == QLatin1Char('1')) {
		response.insert("result", res);
	}
	QJsonDocument document(response);
	QByteArray byteArray = document.toJson(QJsonDocument::Compact);
	emit signal_responeReady(byteArray, port);
}

void HandleServer::handleUserLogin(QJsonObject body, qintptr port) {
	QJsonArray res;
	bool flag = sql->selectSth("users", body, res);
	if (flag) jsonResReady("1", res, port);
	else jsonResReady("3", QJsonArray(), port, "查询用户名失败！");
}

void HandleServer::handleManagerLogin(QJsonObject body, qintptr port) {
	QJsonArray res;
	bool flag = sql->selectSth("managers", body, res);
	if (flag) jsonResReady("1", res, port);
	else jsonResReady("3", QJsonArray(), port, "查询用户名失败！");
}

void HandleServer::handleRegister(QJsonObject body, qintptr port) {
	QString table = "users";
	// ==== MODIFIED BUG FIX ====
	// 原代码: QJsonObject obj; QString _name = obj.value("user_name")... 错误使用了空 obj
	if (body.contains("user_name")) {
		QString _name = body.value("user_name").toString();
		QJsonObject obj;
		obj.insert("want", "user_name");
		obj.insert("isDistinct", "true");
		obj.insert("restriction", QString("user_name = '%1'").arg(_name));
		QJsonArray search;
		bool fl = sql->selectSth(table, obj, search);
		if (!fl) { jsonResReady("3", QJsonArray(), port, "查询用户名失败！"); return; }
		if (!search.isEmpty()) {
			jsonResReady("2", QJsonArray(), port, "该用户名已被注册！");
			return;
		}
	}
	bool flag = sql->insertSth(table, body);
	if (flag) jsonResReady("1", QJsonArray(), port);
	else jsonResReady("3", QJsonArray(), port, "注册失败！");
}

void HandleServer::handleSearchProduct(QJsonObject body, qintptr port) {
	QJsonArray result;
	bool flag = sql->selectSth("products", body, result);
	if (flag) {
		qDebug() << "商品查询结果：" << result;
		jsonResReady("1", result, port);
	}
	else jsonResReady("3", QJsonArray(), port, "查询商品失败！");
}

static void normalizeCartKeysForInsert(QJsonObject& body) {
	// 兼容客户端同时可能传老键名 cart_pro_id / cart_user_id / cart_num
	if (body.contains("cart_pro_id") && !body.contains("Cartitem_pro_id"))
		body.insert("Cartitem_pro_id", body.value("cart_pro_id"));
	if (body.contains("cart_user_id") && !body.contains("Cartitem_user_id"))
		body.insert("Cartitem_user_id", body.value("cart_user_id"));
	if (body.contains("cart_num") && !body.contains("Cartitem_num"))
		body.insert("Cartitem_num", body.value("cart_num"));
}

void HandleServer::handleAddCart(QJsonObject body, qintptr port) {
	normalizeCartKeysForInsert(body);

	if (!body.contains("Cartitem_user_id") || !body.contains("Cartitem_pro_id")) {
		jsonResReady("2", QJsonArray(), port, "缺少必要字段");
		return;
	}
	int uid = body.value("Cartitem_user_id").toString().toInt();
	int pid = body.value("Cartitem_pro_id").toString().toInt();
	if (uid <= 0) {
		jsonResReady("2", QJsonArray(), port, "请先登录");
		return;
	}

	// 可选: 预检查外键存在
	QJsonObject chkUser;
	chkUser.insert("want", "User_id");
	chkUser.insert("restriction", QString("User_id = %1").arg(uid));
	QJsonArray ru;
	if (!sql->selectSth("users", chkUser, ru) || ru.isEmpty()) {
		jsonResReady("2", QJsonArray(), port, "用户不存在");
		return;
	}

	QJsonObject chkProd;
	chkProd.insert("want", "Product_id");
	chkProd.insert("restriction", QString("Product_id = %1").arg(pid));
	QJsonArray rp;
	if (!sql->selectSth("products", chkProd, rp) || rp.isEmpty()) {
		jsonResReady("2", QJsonArray(), port, "商品不存在");
		return;
	}

	// 唯一键预检查
	QJsonObject chkDup;
	chkDup.insert("want", "Cartitem_id");
	chkDup.insert("restriction",
		QString("Cartitem_pro_id = %1 AND Cartitem_user_id = %2")
		.arg(pid).arg(uid));
	QJsonArray rd;
	if (sql->selectSth("cartitems", chkDup, rd) && !rd.isEmpty()) {
		jsonResReady("2", QJsonArray(), port, "该商品已在购物车");
		return;
	}

	if (body.contains("cart_num") && !body.contains("Cartitem_num"))
		body.insert("Cartitem_num", body.value("cart_num"));
	if (!body.contains("Cartitem_num"))
		body.insert("Cartitem_num", "1");

	bool ok = sql->insertSth("cartitems", body);
	if (ok) jsonResReady("1", QJsonArray(), port);
	else    jsonResReady("3", QJsonArray(), port, "插入购物车失败");
}

void HandleServer::handleDelCart(QJsonObject body, qintptr port) {
	normalizeCartKeysForInsert(body);
	bool flag = sql->deleteSth("cartitems", body);
	if (flag) jsonResReady("1", QJsonArray(), port);
	else jsonResReady("3", QJsonArray(), port, "删除购物车失败！");
}

void HandleServer::handleUpdateCart(QJsonObject body, qintptr port) {
	normalizeCartKeysForInsert(body);
	bool flag = sql->updateSth("cartitems", body);
	if (flag) jsonResReady("1", QJsonArray(), port);
	else jsonResReady("3", QJsonArray(), port, "更新购物车失败！");
}

void HandleServer::handleSearchCart(QJsonObject body, qintptr port) {
	// 这里 body 的 "want" / "restriction" 已由客户端改成真实列 + AS，无硬编码可不变
	QJsonArray result;
	bool flag = sql->selectSth("products,cartitems", body, result);
	if (flag) jsonResReady("1", result, port);
	else jsonResReady("3", QJsonArray(), port, "查询购物车失败！");
}

void HandleServer::handleBuySth(QJsonObject body, qintptr port) {
	if (!body.contains("type")) {
		jsonResReady("2", QJsonArray(), port, "需指定购买类型！");
		return;
	}
	QString type = body.value("type").toString().toLower();

	if (type == "cart") {
		// 用户余额
		QJsonArray arrUser;
		QJsonObject qUser;
		int userId = body.value("user_id").toString().toInt();
		qUser.insert("want", "user_money");
		qUser.insert("restriction", QString("user_id = '%1'").arg(userId));
		if (!sql->selectSth("users", qUser, arrUser) || arrUser.isEmpty()) {
			jsonResReady("3", QJsonArray(), port, "查询用户失败！");
			return;
		}
		int money = arrUser[0].toObject().value("user_money").toString().toInt();

		// ==== MODIFIED: 使用真实列并起别名 ====
		QJsonArray allCart;
		QJsonObject qCart;
		qCart.insert("want",
			"Cartitem_pro_id AS cart_pro_id,"
			"Product_name AS pro_name,"
			"Product_amount AS pro_amount,"
			"Product_sales AS pro_sales,"
			"Cartitem_num AS cart_num,"
			"Product_price AS pro_price,"
			"Product_price * Cartitem_num AS pro_tolprice");
		qCart.insert("restriction",
			QString("Products.Product_id = Cartitem_pro_id "
				"AND Cartitem_user_id = user_id "
				"AND user_id = %1").arg(userId));

		if (!sql->selectSth("users,products,cartitems", qCart, allCart) || allCart.isEmpty()) {
			jsonResReady("3", QJsonArray(), port, "查询购物车失败！");
			return;
		}

		// 建立映射
		QJsonObject map;
		for (const auto& v : allCart) {
			QJsonObject row = v.toObject();
			map.insert(row.value("cart_pro_id").toString(), row);
		}

		QJsonArray wannabuy = body.value("wannabuy").toArray();
		if (wannabuy.isEmpty()) {
			jsonResReady("2", QJsonArray(), port, "未选择商品！");
			return;
		}

		int tolprice = 0;
		for (const auto& v : wannabuy) {
			QString idStr = v.toString();
			if (!map.contains(idStr)) {
				jsonResReady("2", QJsonArray(), port, QString("商品ID %1 无效").arg(idStr));
				return;
			}
			QJsonObject rec = map.value(idStr).toObject();
			int need = rec.value("cart_num").toString().toInt();
			int stock = rec.value("pro_amount").toString().toInt();
			QString pname = rec.value("pro_name").toString();
			if (need > stock || stock == 0) {
				jsonResReady("2", QJsonArray(), port, QString("%1商品余量不足！").arg(pname));
				return;
			}
			tolprice += rec.value("pro_tolprice").toString().toInt();
		}

		if (money < tolprice) {
			jsonResReady("2", QJsonArray(), port, "您的余额不足！");
			return;
		}

		// 扣钱
		{
			QJsonObject upd;
			upd.insert("user_money", QString::number(money - tolprice));
			upd.insert("restriction", QString("user_id = %1").arg(userId));
			if (!sql->updateSth("users", upd)) {
				jsonResReady("3", QJsonArray(), port, "更新用户金额失败！");
				return;
			}
		}

		// 返回余额 + 总价
		{
			QJsonArray response;
			response.push_back(money - tolprice);
			response.push_back(tolprice);
			jsonResReady("1", response, port);
		}

		// 更新商品库存销量（真实列）
		for (const auto& v : wannabuy) {
			QJsonObject rec = map.value(v.toString()).toObject();
			int need = rec.value("cart_num").toString().toInt();
			int stock = rec.value("pro_amount").toString().toInt();
			int sales = rec.value("pro_sales").toString().toInt();
			int proId = rec.value("cart_pro_id").toString().toInt();

			QJsonObject updProd;
			updProd.insert("Product_amount", QString::number(stock - need));
			updProd.insert("Product_sales", QString::number(sales + need));
			updProd.insert("restriction", QString("Product_id = %1").arg(proId));

			if (!sql->updateSth("products", updProd)) {
				jsonResReady("3", QJsonArray(), port, "更新商品失败！");
				return;
			}
		}

		// 删除购物车项（真实列）
		{
			QString cond = QString("Cartitem_user_id = %1 AND (").arg(userId);
			for (int i = 0; i < wannabuy.size(); ++i) {
				cond += QString("Cartitem_pro_id = %1").arg(wannabuy[i].toString());
				if (i < wannabuy.size() - 1) cond += " OR ";
			}
			cond += ")";
			QJsonObject delObj;
			delObj.insert("restriction", cond);
			if (!sql->deleteSth("cartitems", delObj)) {
				jsonResReady("3", QJsonArray(), port, "删除购物车项失败！");
				return;
			}
		}

		// 生成订单
		QDateTime now = QDateTime::currentDateTime();
		QString orderId = getRandomOrderNum();
		QJsonObject orderObj;
		orderObj.insert("order_time", now.toString("yyyy-MM-dd hh:mm:ss"));
		orderObj.insert("order_user_id", QString::number(userId));
		orderObj.insert("order_tolprice", QString::number(tolprice));
		orderObj.insert("order_id", orderId);
		bool okInsertOrder = sql->insertSth("orders", orderObj);

		if (okInsertOrder && createOrderItems(wannabuy, map, orderId)) {
			// 再发一个成功（可选；如果不想重复发可以去掉）
			// jsonResReady("1", QJsonArray(), port);
		}
		else {
			jsonResReady("3", QJsonArray(), port, "生成订单失败！");
			return;
		}
	}
}

bool HandleServer::createOrderItems(QJsonArray wannabuy, QJsonObject map, QString ordernum) {
	bool res = true;
	for (const auto& v : wannabuy) {
		QJsonObject rec = map.value(v.toString()).toObject();
		int num = rec.value("cart_num").toString().toInt();
		int proId = rec.value("cart_pro_id").toString().toInt();
		int price = rec.value("pro_price").toString().toInt();

		QJsonObject item;
		item.insert("orderitem_order_id", ordernum);
		item.insert("orderitem_time", ordernum); // 这里似乎用 ordernum 当时间，可能要改
		item.insert("orderitem_pro_id", QString::number(proId));
		item.insert("orderitem_num", QString::number(num));
		item.insert("orderitem_pro_price", QString::number(price));

		if (!sql->insertSth("orderitems", item)) {
			res = false;
		}
	}
	return res;
}

QString HandleServer::getRandomOrderNum() {
	QJsonObject obj;
	QString context = "CONCAT('SD',DATE_FORMAT(now(), '%Y%m%d%H%i%s'),lpad(round(round(rand(),4)*1000),4,'0'))";
	obj.insert("want", context);
	QJsonArray result;
	if (!sql->selectSth("dual", obj, result) || result.isEmpty()) return "error";
	return result[0].toObject().value(context).toString();
}

void HandleServer::handleSearchOrder(QJsonObject body, qintptr port) {
	QJsonArray result;
	bool flag = sql->selectSth("orders", body, result);
	if (flag) jsonResReady("1", result, port);
	else jsonResReady("3", QJsonArray(), port, "查询订单失败！");
}

void HandleServer::handleSearchOrderItems(QJsonObject body, qintptr port) {
	QJsonArray result;
	bool flag = sql->selectSth("orders,orderitems,products", body, result);
	qDebug() << "orderitems num:" << result.size();
	if (flag) jsonResReady("1", result, port);
	else jsonResReady("3", QJsonArray(), port, "查询订单信息失败！");
}