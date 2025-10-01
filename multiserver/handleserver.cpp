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
					case 6: handlePayOrder(body, port); break;  // 新增付款处理
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
		int userId = body.value("user_id").toString().toInt();

		// 开始事务处理
		if (!sql->beginTransaction()) {
			qDebug() << "开始事务失败";
		}

		try {
			// 1. 查询用户余额
			QJsonArray arrUser;
			QJsonObject qUser;
			qUser.insert("want", "user_money");
			qUser.insert("restriction", QString("user_id = '%1'").arg(userId));
			if (!sql->selectSth("users", qUser, arrUser) || arrUser.isEmpty()) {
				sql->rollbackTransaction();
				jsonResReady("3", QJsonArray(), port, "查询用户失败！");
				return;
			}
			int money = arrUser[0].toObject().value("user_money").toString().toInt();

			// 2. 查询购物车商品
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
				sql->rollbackTransaction();
				jsonResReady("3", QJsonArray(), port, "查询购物车失败！");
				return;
			}

			// 3. 建立商品映射
			QJsonObject map;
			for (const auto& v : allCart) {
				QJsonObject row = v.toObject();
				map.insert(row.value("cart_pro_id").toString(), row);
			}

			QJsonArray wannabuy = body.value("wannabuy").toArray();
			if (wannabuy.isEmpty()) {
				sql->rollbackTransaction();
				jsonResReady("2", QJsonArray(), port, "未选择商品！");
				return;
			}

			// 4. 验证库存和计算总价
			int tolprice = 0;
			for (const auto& v : wannabuy) {
				QString idStr = v.toString();
				if (!map.contains(idStr)) {
					sql->rollbackTransaction();
					jsonResReady("2", QJsonArray(), port, QString("商品ID %1 无效").arg(idStr));
					return;
				}
				QJsonObject rec = map.value(idStr).toObject();
				int need = rec.value("cart_num").toString().toInt();
				int stock = rec.value("pro_amount").toString().toInt();
				QString pname = rec.value("pro_name").toString();
				if (need > stock || stock == 0) {
					sql->rollbackTransaction();
					jsonResReady("2", QJsonArray(), port, QString("%1商品余量不足！").arg(pname));
					return;
				}
				tolprice += rec.value("pro_tolprice").toString().toInt();
			}

			if (money < tolprice) {
				sql->rollbackTransaction();
				jsonResReady("2", QJsonArray(), port, "您的余额不足！");
				return;
			}

			// 5. 扣除用户余额
			QJsonObject updUser;
			updUser.insert("user_money", QString::number(money - tolprice));
			updUser.insert("restriction", QString("user_id = %1").arg(userId));
			if (!sql->updateSth("users", updUser)) {
				sql->rollbackTransaction();
				jsonResReady("3", QJsonArray(), port, "更新用户金额失败！");
				return;
			}

			// 6. 更新商品库存和销量
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
					sql->rollbackTransaction();
					jsonResReady("3", QJsonArray(), port, "更新商品失败！");
					return;
				}
			}

			// 7. 删除购物车项
			QString cond = QString("Cartitem_user_id = %1 AND (").arg(userId);
			for (int i = 0; i < wannabuy.size(); ++i) {
				cond += QString("Cartitem_pro_id = %1").arg(wannabuy[i].toString());
				if (i < wannabuy.size() - 1) cond += " OR ";
			}
			cond += ")";
			QJsonObject delObj;
			delObj.insert("restriction", cond);
			if (!sql->deleteSth("cartitems", delObj)) {
				sql->rollbackTransaction();
				jsonResReady("3", QJsonArray(), port, "删除购物车项失败！");
				return;
			}

			// 8. 创建订单 - 使用正确的字段名
			QDateTime now = QDateTime::currentDateTime();
			QString orderId = getRandomOrderNum();
			QJsonObject orderObj;
			orderObj.insert("Order_time", now.toString("yyyy-MM-dd hh:mm:ss"));
			orderObj.insert("Order_user_id", QString::number(userId));
			orderObj.insert("Order_tolprice", QString::number(tolprice));
			orderObj.insert("Order_id", orderId);
			orderObj.insert("Order_status", "待付款");  // 设置默认状态

			if (!sql->insertSth("orders", orderObj)) {
				sql->rollbackTransaction();
				jsonResReady("3", QJsonArray(), port, "创建订单失败！");
				return;
			}

			// 9. 创建订单项 - 使用正确的字段名
			if (!createOrderItems(wannabuy, map, orderId)) {
				sql->rollbackTransaction();
				jsonResReady("3", QJsonArray(), port, "创建订单项失败！");
				return;
			}

			// 提交事务
			if (!sql->commitTransaction()) {
				sql->rollbackTransaction();
				jsonResReady("3", QJsonArray(), port, "提交事务失败！");
				return;
			}

			// 返回成功响应
			QJsonArray response;
			response.push_back(money - tolprice);
			response.push_back(tolprice);
			jsonResReady("1", response, port);

		}
		catch (...) {
			sql->rollbackTransaction();
			jsonResReady("3", QJsonArray(), port, "下单过程中发生异常！");
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
		// 使用正确的字段名
		item.insert("Orderitem_order_id", ordernum);
		item.insert("Orderitem_time", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
		item.insert("Orderitem_pro_id", QString::number(proId));
		item.insert("Orderitem_num", QString::number(num));
		item.insert("Orderitem_pro_price", QString::number(price));

		if (!sql->insertSth("orderitems", item)) {
			qDebug() << "插入订单项失败:" << item;
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
	// 只返回有订单项的订单
	if (body.value("want").toString() == "*") {
		body.insert("want", "DISTINCT orders.Order_id,orders.Order_user_id,orders.Order_tolprice,orders.Order_status,orders.Order_time");

		// 修改查询条件，只查询有订单项的订单
		QString originalRestriction = body.value("restriction").toString();
		QString newRestriction = QString("orders.Order_id = orderitems.Orderitem_order_id AND %1").arg(originalRestriction);
		body.insert("restriction", newRestriction);

		QJsonArray result;
		bool flag = sql->selectSth("orders,orderitems", body, result);

		qDebug() << "查询有订单项的订单数量:" << result.size();

		if (flag) jsonResReady("1", result, port);
		else jsonResReady("3", QJsonArray(), port, "查询订单失败！");
	}
	else {
		QJsonArray result;
		bool flag = sql->selectSth("orders", body, result);
		if (flag) jsonResReady("1", result, port);
		else jsonResReady("3", QJsonArray(), port, "查询订单失败！");
	}
}



void HandleServer::handleSearchOrderItems(QJsonObject body, qintptr port) {
	QJsonArray result;
	bool flag = sql->selectSth("orders,orderitems,products", body, result);
	qDebug() << "orderitems num:" << result.size();

	// 添加调试信息
	qDebug() << "查询条件:" << body.value("restriction").toString();

	if (flag) jsonResReady("1", result, port);
	else jsonResReady("3", QJsonArray(), port, "查询订单信息失败！");
}

// 添加付款处理函数
void HandleServer::handlePayOrder(QJsonObject body, qintptr port) {
	if (!body.contains("order_id") || !body.contains("user_id")) {
		jsonResReady("2", QJsonArray(), port, "缺少订单ID或用户ID");
		return;
	}

	QString orderId = body.value("order_id").toString();
	int userId = body.value("user_id").toString().toInt();

	qDebug() << "处理付款请求 - 订单ID:" << orderId << "用户ID:" << userId;

	// 开始事务
	if (!sql->beginTransaction()) {
		jsonResReady("3", QJsonArray(), port, "开始事务失败");
		return;
	}

	try {
		// 1. 验证订单是否存在且属于该用户
		QJsonArray orderResult;
		QJsonObject queryOrder;
		queryOrder.insert("want", "Order_id,Order_user_id,Order_tolprice,Order_status");
		queryOrder.insert("restriction", QString("Order_id = '%1' AND Order_user_id = %2").arg(orderId).arg(userId));

		if (!sql->selectSth("orders", queryOrder, orderResult) || orderResult.isEmpty()) {
			sql->rollbackTransaction();
			jsonResReady("2", QJsonArray(), port, "订单不存在或不属于当前用户");
			return;
		}

		QJsonObject order = orderResult[0].toObject();
		QString currentStatus = order.value("Order_status").toString();
		int orderPrice = order.value("Order_tolprice").toString().toInt();

		// 2. 检查订单状态
		if (currentStatus != "待付款") {
			sql->rollbackTransaction();
			if (currentStatus == "已付款") {
				jsonResReady("2", QJsonArray(), port, "订单已付款，无需重复付款");
			}
			else if (currentStatus == "已取消") {
				jsonResReady("2", QJsonArray(), port, "订单已取消，无法付款");
			}
			else {
				jsonResReady("2", QJsonArray(), port, "订单状态异常，无法付款");
			}
			return;
		}

		// 3. 检查用户余额
		QJsonArray userResult;
		QJsonObject queryUser;
		queryUser.insert("want", "user_money");
		queryUser.insert("restriction", QString("user_id = %1").arg(userId));

		if (!sql->selectSth("users", queryUser, userResult) || userResult.isEmpty()) {
			sql->rollbackTransaction();
			jsonResReady("3", QJsonArray(), port, "查询用户信息失败");
			return;
		}

		int userMoney = userResult[0].toObject().value("user_money").toString().toInt();
		if (userMoney < orderPrice) {
			sql->rollbackTransaction();
			jsonResReady("2", QJsonArray(), port, QString("余额不足！当前余额：%1，订单金额：%2").arg(userMoney).arg(orderPrice));
			return;
		}

		// 4. 扣除用户余额
		QJsonObject updateUser;
		updateUser.insert("user_money", QString::number(userMoney - orderPrice));
		updateUser.insert("restriction", QString("user_id = %1").arg(userId));

		if (!sql->updateSth("users", updateUser)) {
			sql->rollbackTransaction();
			jsonResReady("3", QJsonArray(), port, "扣除余额失败");
			return;
		}

		// 5. 更新订单状态为已付款
		QJsonObject updateOrder;
		updateOrder.insert("Order_status", "已付款");
		updateOrder.insert("Order_pay_time", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
		updateOrder.insert("restriction", QString("Order_id = '%1'").arg(orderId));

		if (!sql->updateSth("orders", updateOrder)) {
			sql->rollbackTransaction();
			jsonResReady("3", QJsonArray(), port, "更新订单状态失败");
			return;
		}

		// 6. 提交事务
		if (!sql->commitTransaction()) {
			sql->rollbackTransaction();
			jsonResReady("3", QJsonArray(), port, "提交事务失败");
			return;
		}

		// 7. 返回付款成功信息
		QJsonArray response;
		QJsonObject payResult;
		payResult.insert("order_id", orderId);
		payResult.insert("remaining_balance", QString::number(userMoney - orderPrice));
		payResult.insert("paid_amount", QString::number(orderPrice));
		payResult.insert("pay_time", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
		response.append(payResult);

		jsonResReady("1", response, port);
		qDebug() << "付款成功 - 订单ID:" << orderId << "金额:" << orderPrice;

	}
	catch (...) {
		sql->rollbackTransaction();
		jsonResReady("3", QJsonArray(), port, "付款过程中发生异常");
	}
}