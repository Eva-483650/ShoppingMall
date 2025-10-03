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

				qDebug() << "收到请求 - 协议码:" << strHead
					<< "flag_inskind:" << flag_inskind
					<< "flag_ins:" << flag_ins;

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
				case 5:  // flag_inskind = 05
					qDebug() << "进入优惠券处理分支 - flag_ins:" << flag_ins;
					switch (flag_ins) {
					case 1:
						qDebug() << "调用 handleGetUserCoupons";
						handleGetUserCoupons(body, port);
						break;
					case 2:
						qDebug() << "调用 handleUseCoupon";
						handleUseCoupon(body, port);
						break;
					case 3: handleGetMerchantDiscounts(body, port); break;
					case 4: handleCalculateOrderPrice(body, port); break;
					default: jsonResReady("2", QJsonArray(), port, "未知优惠券操作"); break;
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

// 获取用户可用优惠券
void HandleServer::handleGetUserCoupons(QJsonObject body, qintptr port) {
	// 添加调试信息
	qDebug() << "=== handleGetUserCoupons 调试 ===";
	qDebug() << "请求体:" << body;

	if (!body.contains("user_id")) {
		qDebug() << "缺少 user_id 字段";
		jsonResReady("2", QJsonArray(), port, "缺少用户ID");
		return;
	}

	// 尝试不同的方式获取用户ID
	QJsonValue userIdValue = body.value("user_id");
	qDebug() << "user_id 原始值:" << userIdValue;
	qDebug() << "user_id 类型:" << userIdValue.type();

	int userId = 0;
	if (userIdValue.isString()) {
		userId = userIdValue.toString().toInt();
		qDebug() << "从字符串转换的用户ID:" << userId;
	}
	else if (userIdValue.isDouble()) {
		userId = userIdValue.toInt();
		qDebug() << "从数字转换的用户ID:" << userId;
	}

	if (userId <= 0) {
		qDebug() << "用户ID无效:" << userId;
		jsonResReady("2", QJsonArray(), port, "用户ID无效");
		return;
	}

	int orderAmount = body.value("order_amount").toInt(); // 可选，用于筛选可用优惠券
	qDebug() << "订单金额:" << orderAmount;

	QJsonObject queryObj;
	queryObj.insert("want",
		"uc.Coupon_id, uc.Coupon_code, uc.Coupon_name, uc.Coupon_type, "
		"uc.Coupon_value, uc.Coupon_min_amount, uc.Coupon_max_discount, "
		"uc.Coupon_start_time, uc.Coupon_end_time, uc.Coupon_description, "
		"uco.Is_used, uco.Obtained_time");

	QString restriction = QString(
		"uco.User_id = %1 AND uco.Coupon_id = uc.Coupon_id "
		"AND uco.Is_used = '未使用' AND uc.Coupon_status = '有效' "
		"AND NOW() BETWEEN uc.Coupon_start_time AND uc.Coupon_end_time"
	).arg(userId);

	// 如果提供了订单金额，只返回满足条件的优惠券
	if (orderAmount > 0) {
		restriction += QString(" AND uc.Coupon_min_amount <= %1").arg(orderAmount);
	}

	queryObj.insert("restriction", restriction);
	queryObj.insert("orderBy", "uc.Coupon_value DESC"); // 按优惠金额降序排列

	qDebug() << "最终查询条件:" << restriction;

	QJsonArray result;
	bool flag = sql->selectSth("user_coupon_ownership uco, user_coupons uc", queryObj, result);

	if (flag) {
		qDebug() << "查询到用户优惠券数量:" << result.size();
		if (result.size() > 0) {
			qDebug() << "查询结果:" << result;
		}
		jsonResReady("1", result, port);
	}
	else {
		qDebug() << "SQL查询失败";
		jsonResReady("3", QJsonArray(), port, "查询用户优惠券失败");
	}
}

// 修复后的使用优惠券函数
void HandleServer::handleUseCoupon(QJsonObject body, qintptr port) {
	qDebug() << "=== handleUseCoupon 调试 ===";
	qDebug() << "请求体:" << body;

	if (!body.contains("user_id") || !body.contains("coupon_id")) {
		jsonResReady("2", QJsonArray(), port, "缺少必要参数");
		return;
	}

	// 解析参数
	int userId = 0;
	int couponId = 0;

	QJsonValue userIdValue = body.value("user_id");
	if (userIdValue.isString()) {
		userId = userIdValue.toString().toInt();
	}
	else if (userIdValue.isDouble()) {
		userId = userIdValue.toInt();
	}

	QJsonValue couponIdValue = body.value("coupon_id");
	if (couponIdValue.isString()) {
		couponId = couponIdValue.toString().toInt();
	}
	else if (couponIdValue.isDouble()) {
		couponId = couponIdValue.toInt();
	}

	QString orderId = body.value("order_id").toString();
	int orderAmount = body.value("order_amount").toInt();

	qDebug() << "解析的参数:";
	qDebug() << "userId:" << userId;
	qDebug() << "couponId:" << couponId;
	qDebug() << "orderId:" << orderId;
	qDebug() << "orderAmount:" << orderAmount;

	if (userId <= 0 || couponId <= 0) {
		qDebug() << "参数无效:" << "userId=" << userId << "couponId=" << couponId;
		jsonResReady("2", QJsonArray(), port, "参数无效");
		return;
	}

	// 开始事务
	if (!sql->beginTransaction()) {
		qDebug() << "事务开始失败";
		jsonResReady("3", QJsonArray(), port, "数据库事务失败");
		return;
	}
	qDebug() << "事务开始成功";

	try {
		// 1. 检查优惠券是否存在且未使用
		QJsonObject queryObj;
		queryObj.insert("want", "Is_used");
		queryObj.insert("restriction", QString("User_id = %1 AND Coupon_id = %2").arg(userId).arg(couponId));

		qDebug() << "查询条件:" << queryObj.value("restriction").toString();

		QJsonArray result;
		bool flag = sql->selectSth("user_coupon_ownership", queryObj, result);

		if (!flag) {
			sql->rollbackTransaction();
			qDebug() << "查询优惠券失败";
			jsonResReady("3", QJsonArray(), port, "查询优惠券失败");
			return;
		}

		if (result.isEmpty()) {
			sql->rollbackTransaction();
			qDebug() << "优惠券不存在";
			jsonResReady("2", QJsonArray(), port, "优惠券不存在或不属于该用户");
			return;
		}

		QString isUsed = result[0].toObject().value("Is_used").toString();
		if (isUsed == "已使用") {
			sql->rollbackTransaction();
			qDebug() << "优惠券已使用";
			jsonResReady("2", QJsonArray(), port, "优惠券已使用");
			return;
		}

		// 2. 验证优惠券是否可用
		if (!validateCouponUsage(userId, couponId, orderAmount)) {
			sql->rollbackTransaction();
			qDebug() << "优惠券验证失败";
			jsonResReady("2", QJsonArray(), port, "优惠券不符合使用条件");
			return;
		}

		// 3. 更新优惠券状态为已使用
		QJsonObject updateObj;
		updateObj.insert("Is_used", "已使用");
		updateObj.insert("Used_time", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
		if (!orderId.isEmpty()) {
			updateObj.insert("Used_order_id", orderId);
		}

		updateObj.insert("restriction", QString("User_id = %1 AND Coupon_id = %2").arg(userId).arg(couponId));

		bool updateFlag = sql->updateSth("user_coupon_ownership", updateObj);

		if (!updateFlag) {
			sql->rollbackTransaction();
			qDebug() << "更新优惠券状态失败";
			jsonResReady("3", QJsonArray(), port, "更新优惠券状态失败");
			return;
		}

		// 4. 提交事务
		if (!sql->commitTransaction()) {
			sql->rollbackTransaction();
			qDebug() << "事务提交失败";
			jsonResReady("3", QJsonArray(), port, "事务提交失败");
			return;
		}

		qDebug() << "优惠券使用成功";
		jsonResReady("1", QJsonArray(), port, "优惠券使用成功");

	}
	catch (...) {
		sql->rollbackTransaction();
		qDebug() << "使用优惠券时发生异常";
		jsonResReady("3", QJsonArray(), port, "使用优惠券时发生异常");
	}
}

// 获取商家折扣
void HandleServer::handleGetMerchantDiscounts(QJsonObject body, qintptr port) {
	QJsonObject queryObj;
	queryObj.insert("want",
		"md.Discount_id, md.Product_id, md.Discount_type, md.Discount_value, "
		"md.Discount_start_time, md.Discount_end_time, md.Discount_description, "
		"p.Product_name, p.Product_price");

	QString restriction =
		"md.Product_id = p.Product_id AND md.Discount_status = '有效' "
		"AND NOW() BETWEEN md.Discount_start_time AND md.Discount_end_time";

	// 如果指定了商品ID，只查询该商品的折扣
	if (body.contains("product_ids")) {
		QJsonArray productIds = body.value("product_ids").toArray();
		if (!productIds.isEmpty()) {
			QString productFilter = " AND (";
			for (int i = 0; i < productIds.size(); ++i) {
				productFilter += QString("md.Product_id = %1").arg(productIds[i].toString());
				if (i < productIds.size() - 1) productFilter += " OR ";
			}
			productFilter += ")";
			restriction += productFilter;
		}
	}

	queryObj.insert("restriction", restriction);
	queryObj.insert("orderBy", "md.Discount_value DESC");

	QJsonArray result;
	bool flag = sql->selectSth("merchant_discounts md, products p", queryObj, result);

	if (flag) {
		jsonResReady("1", result, port);
	}
	else {
		jsonResReady("3", QJsonArray(), port, "查询商家折扣失败");
	}
}

// 计算订单价格（包含优惠券和商家折扣）
void HandleServer::handleCalculateOrderPrice(QJsonObject body, qintptr port) {
	if (!body.contains("products") || !body.contains("user_id")) {
		jsonResReady("2", QJsonArray(), port, "缺少商品信息或用户ID");
		return;
	}

	QJsonArray products = body.value("products").toArray();
	int userId = body.value("user_id").toString().toInt();
	int couponId = body.value("coupon_id").toInt(); // 可选

	// 1. 计算原始总价
	int originalPrice = 0;
	for (const QJsonValue& value : products) {
		QJsonObject product = value.toObject();
		int price = product.value("price").toInt();
		int quantity = product.value("quantity").toInt();
		originalPrice += price * quantity;
	}

	// 2. 计算商家折扣
	int merchantDiscount = calculateMerchantDiscount(products);

	// 3. 计算优惠券折扣
	int couponDiscount = 0;
	if (couponId > 0) {
		int priceAfterMerchantDiscount = originalPrice - merchantDiscount;
		couponDiscount = calculateCouponDiscount(couponId, priceAfterMerchantDiscount, userId);
	}

	// 4. 计算最终价格
	int finalPrice = originalPrice - merchantDiscount - couponDiscount;
	if (finalPrice < 0) finalPrice = 0;

	// 5. 返回结果
	QJsonArray response;
	QJsonObject result;
	result.insert("original_price", originalPrice);
	result.insert("merchant_discount", merchantDiscount);
	result.insert("coupon_discount", couponDiscount);
	result.insert("final_price", finalPrice);
	response.append(result);

	jsonResReady("1", response, port);
}

// 辅助函数：计算优惠券折扣
int HandleServer::calculateCouponDiscount(int couponId, int orderAmount, int userId) {
	// 查询优惠券信息
	QJsonArray couponResult;
	QJsonObject queryCoupon;
	queryCoupon.insert("want", "Coupon_type, Coupon_value, Coupon_min_amount, Coupon_max_discount");
	queryCoupon.insert("restriction", QString("Coupon_id = %1").arg(couponId));

	if (!sql->selectSth("user_coupons", queryCoupon, couponResult) || couponResult.isEmpty()) {
		return 0;
	}

	QJsonObject coupon = couponResult[0].toObject();
	QString type = coupon.value("Coupon_type").toString();
	double value = coupon.value("Coupon_value").toString().toDouble();
	int minAmount = coupon.value("Coupon_min_amount").toString().toInt();
	int maxDiscount = coupon.value("Coupon_max_discount").toString().toInt();

	// 检查最低使用金额
	if (orderAmount < minAmount) {
		return 0;
	}

	int discount = 0;
	if (type == "固定金额") {
		discount = (int)value;
	}
	else if (type == "百分比折扣") {
		discount = (int)(orderAmount * value / 100.0);
		if (maxDiscount > 0 && discount > maxDiscount) {
			discount = maxDiscount;
		}
	}

	// 折扣不能超过订单金额
	if (discount > orderAmount) {
		discount = orderAmount;
	}

	return discount;
}

// 辅助函数：计算商家折扣
int HandleServer::calculateMerchantDiscount(const QJsonArray& products) {
	int totalDiscount = 0;

	for (const QJsonValue& value : products) {
		QJsonObject product = value.toObject();
		int productId = product.value("product_id").toInt();
		int price = product.value("price").toInt();
		int quantity = product.value("quantity").toInt();

		// 查询该商品的有效折扣
		QJsonArray discountResult;
		QJsonObject queryDiscount;
		queryDiscount.insert("want", "Discount_type, Discount_value");
		queryDiscount.insert("restriction",
			QString("Product_id = %1 AND Discount_status = '有效' "
				"AND NOW() BETWEEN Discount_start_time AND Discount_end_time")
			.arg(productId));

		if (sql->selectSth("merchant_discounts", queryDiscount, discountResult) && !discountResult.isEmpty()) {
			QJsonObject discount = discountResult[0].toObject();
			QString type = discount.value("Discount_type").toString();
			double value = discount.value("Discount_value").toString().toDouble();

			int itemDiscount = 0;
			if (type == "固定金额") {
				itemDiscount = (int)value * quantity;
			}
			else if (type == "百分比折扣") {
				itemDiscount = (int)(price * quantity * value / 100.0);
			}

			totalDiscount += itemDiscount;
		}
	}

	return totalDiscount;
}

// 辅助函数：验证优惠券是否可用
// 辅助函数：验证优惠券是否可用
bool HandleServer::validateCouponUsage(int userId, int couponId, int orderAmount) {
	qDebug() << "=== validateCouponUsage 调试 ===";
	qDebug() << "userId:" << userId << "couponId:" << couponId << "orderAmount:" << orderAmount;

	// 查询用户是否拥有该优惠券且未使用
	QJsonArray ownershipResult;
	QJsonObject queryOwnership;
	queryOwnership.insert("want", "Is_used");
	queryOwnership.insert("restriction",
		QString("User_id = %1 AND Coupon_id = %2").arg(userId).arg(couponId));

	if (!sql->selectSth("user_coupon_ownership", queryOwnership, ownershipResult) ||
		ownershipResult.isEmpty()) {
		qDebug() << "用户不拥有该优惠券";
		return false;
	}

	QString isUsed = ownershipResult[0].toObject().value("Is_used").toString();
	qDebug() << "优惠券使用状态:" << isUsed;
	if (isUsed == "已使用") {
		qDebug() << "优惠券已使用";
		return false;
	}

	// 查询优惠券是否有效
	QJsonArray couponResult;
	QJsonObject queryCoupon;
	queryCoupon.insert("want", "Coupon_status, Coupon_min_amount, Coupon_start_time, Coupon_end_time");
	queryCoupon.insert("restriction", QString("Coupon_id = %1").arg(couponId));

	if (!sql->selectSth("user_coupons", queryCoupon, couponResult) || couponResult.isEmpty()) {
		qDebug() << "查询优惠券信息失败";
		return false;
	}

	QJsonObject coupon = couponResult[0].toObject();
	QString status = coupon.value("Coupon_status").toString();
	int minAmount = coupon.value("Coupon_min_amount").toString().toInt();
	QString startTime = coupon.value("Coupon_start_time").toString();
	QString endTime = coupon.value("Coupon_end_time").toString();

	qDebug() << "优惠券信息:";
	qDebug() << "status:" << status;
	qDebug() << "minAmount:" << minAmount;
	qDebug() << "startTime:" << startTime;
	qDebug() << "endTime:" << endTime;

	// 检查优惠券状态
	if (!status.isEmpty() && status != "有效") {
		qDebug() << "优惠券状态无效:" << status;
		return false;
	}

	// 检查最低使用金额
	if (orderAmount < minAmount) {
		qDebug() << "订单金额不满足最低使用金额:" << orderAmount << "<" << minAmount;
		return false;
	}

	// 检查有效期 - 处理ISO格式的日期时间
	QDateTime now = QDateTime::currentDateTime();

	// 处理开始时间
	QDateTime start;
	if (startTime.contains("T")) {
		// ISO格式: "2025-01-01T00:00:00.000Z"
		QString cleanStartTime = startTime;
		if (cleanStartTime.endsWith("Z")) {
			cleanStartTime.chop(1);
		}
		if (cleanStartTime.contains(".")) {
			cleanStartTime = cleanStartTime.left(cleanStartTime.indexOf("."));
		}
		cleanStartTime.replace("T", " ");
		start = QDateTime::fromString(cleanStartTime, "yyyy-MM-dd hh:mm:ss");
	}
	else {
		// 标准格式: "2025-01-01 00:00:00"
		start = QDateTime::fromString(startTime, "yyyy-MM-dd hh:mm:ss");
	}

	// 处理结束时间
	QDateTime end;
	if (endTime.contains("T")) {
		// ISO格式: "2025-12-31T23:59:59.000Z"
		QString cleanEndTime = endTime;
		if (cleanEndTime.endsWith("Z")) {
			cleanEndTime.chop(1);
		}
		if (cleanEndTime.contains(".")) {
			cleanEndTime = cleanEndTime.left(cleanEndTime.indexOf("."));
		}
		cleanEndTime.replace("T", " ");
		end = QDateTime::fromString(cleanEndTime, "yyyy-MM-dd hh:mm:ss");
	}
	else {
		// 标准格式: "2025-12-31 23:59:59"
		end = QDateTime::fromString(endTime, "yyyy-MM-dd hh:mm:ss");
	}

	qDebug() << "时间验证:";
	qDebug() << "当前时间:" << now.toString("yyyy-MM-dd hh:mm:ss");
	qDebug() << "开始时间:" << start.toString("yyyy-MM-dd hh:mm:ss");
	qDebug() << "结束时间:" << end.toString("yyyy-MM-dd hh:mm:ss");
	qDebug() << "start.isValid():" << start.isValid();
	qDebug() << "end.isValid():" << end.isValid();

	if (!start.isValid() || !end.isValid()) {
		qDebug() << "日期时间解析失败";
		return false;
	}

	if (now < start) {
		qDebug() << "优惠券尚未生效";
		return false;
	}

	if (now > end) {
		qDebug() << "优惠券已过期";
		return false;
	}

	qDebug() << "优惠券验证通过";
	return true;
}