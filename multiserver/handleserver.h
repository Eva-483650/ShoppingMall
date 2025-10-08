#ifndef HANDLESERVER_H
#define HANDLESERVER_H

#include <QObject>
#include<QJsonArray>
#include<QJsonObject>
#include<QJsonDocument>
#include<QJsonParseError>
#include <QRandomGenerator>
#include"sqlserver.h"
#include"tcpserver.h"

class HandleServer:public QObject
{
    Q_OBJECT
public:
    HandleServer(SQLServer*);
    void jsonResReady(QString head,QJsonArray res,qintptr port,QString errmsg="");
    QString getRandomOrderNum();
    QString getRandomReturnNum();  // 新增：生成退货单号

private:
    SQLServer *sql;
    TcpServer *tcp;
    void handleUserLogin(QJsonObject body,qintptr port);
    void handleManagerLogin(QJsonObject body,qintptr port);
    void handleRegister(QJsonObject body,qintptr port);
    void handleSearchProduct(QJsonObject body,qintptr port);
    void handleAddCart(QJsonObject body,qintptr port);
    void handleDelCart(QJsonObject body,qintptr port);
    void handleUpdateCart(QJsonObject body,qintptr port);
    void handleSearchCart(QJsonObject body,qintptr port);
    void handleBuySth(QJsonObject body,qintptr port);
    bool createOrderItems(QJsonArray wannabuy,QJsonObject map,QString ordernum);
    void handleSearchOrder(QJsonObject body,qintptr port);
    void handleSearchOrderItems(QJsonObject body,qintptr port);
    void handleManagerSearchOrderItems(QJsonObject body, qintptr port);   // 新增：管理员订单详情查询
    void handleUpdateOrderStatus(QJsonObject body, qintptr port);
    // 付款逻辑
    void handlePayOrder(QJsonObject body, qintptr port);

	// 退货逻辑 - 新增
	void handleReturnOrder(QJsonObject body, qintptr port);
	void handleGetReturnList(QJsonObject body, qintptr port);
	void handleUpdateReturnStatus(QJsonObject body, qintptr port);
    
    //优惠券逻辑
	// 在 HandleServer 类的 private 部分添加
	void handleGetUserCoupons(QJsonObject body, qintptr port);
	void handleUseCoupon(QJsonObject body, qintptr port);
	void handleGetMerchantDiscounts(QJsonObject body, qintptr port);
	void handleCalculateOrderPrice(QJsonObject body, qintptr port);

	// 辅助函数
	int calculateCouponDiscount(int couponId, int orderAmount, int userId);
	int calculateMerchantDiscount(const QJsonArray& products);
	bool validateCouponUsage(int userId, int couponId, int orderAmount);

    // 商家
	void handleAddProduct(QJsonObject body, qintptr port);
	void handleUpdateProduct(QJsonObject body, qintptr port);
	void handleDeleteProduct(QJsonObject body, qintptr port);
	void handleGetAllProducts(QJsonObject body, qintptr port);

public slots:
    void handleRequest(const QString&,const qintptr, const QByteArray);
signals:
    void signal_responeReady(const QByteArray,qintptr);
};

#endif // HANDLESERVER_H
