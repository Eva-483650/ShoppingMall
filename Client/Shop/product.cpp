#include "product.h"
#include <QDebug>

Product::Product()
 : id(0), islimited(false), price(0), amount(0), sales(0)
{

}

Product::Product(QJsonObject obj) {
	// 按照数据库字段名首字母大写取值
	QString _name = obj.value("Product_name").toString();
	QString _id = obj.value("Product_id").toString();
	QString _price = obj.value("Product_price").toString();
	QString _amount = obj.value("Product_amount").toString();
	QString _sales = obj.value("Product_sales").toString();
	QString _classification = obj.value("Product_classification").toString();
	QString _about = obj.value("Product_about").toString();
	QString _istimelimited = obj.value("Product_istimelimited").toString().toLower();
	QString _pictureaddress = obj.value("Product_pictureaddress").toString();

	this->name = _name;
	this->id = _id.toInt();
	this->price = _price.toInt();
	this->amount = _amount.toInt();
	this->sales = _sales.toInt();
	this->classification = _classification;
	this->about = _about;
	this->pictureaddress = _pictureaddress;
	// "是"、"否" 或 bool 转换
	if (_istimelimited == "true" || _istimelimited == "是") { this->islimited = true; }
	else { this->islimited = false; }

	qDebug() << "[Product ctor]"
		<< "id=" << id
		<< "name=" << name
		<< "pic=" << pictureaddress;
}