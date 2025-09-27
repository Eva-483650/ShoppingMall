#include "person.h"
#include <QDebug>

Person::Person() {}

Person::Person(const QJsonObject& obj)
{
	updateFromJson(obj);
}

QJsonObject Person::normalizeKeys(const QJsonObject& src)
{
	QJsonObject lower;
	for (auto it = src.begin(); it != src.end(); ++it) {
		lower.insert(it.key().toLower(), it.value());
	}
	return lower;
}

void Person::updateFromJson(const QJsonObject& obj)
{
	QJsonObject o = normalizeKeys(obj);

	// id 兼容字段：user_id / userid / id
	if (o.contains("user_id"))
		id = o.value("user_id").toString().toInt();
	else if (o.contains("userid"))
		id = o.value("userid").toString().toInt();
	else if (o.contains("id"))
		id = o.value("id").toString().toInt();

	if (o.contains("user_money"))
		money = o.value("user_money").toString().toInt();
	else if (o.contains("money"))
		money = o.value("money").toString().toInt();

	if (o.contains("user_name"))
		name = o.value("user_name").toString();
	else if (o.contains("name"))
		name = o.value("name").toString();

	if (o.contains("user_password"))
		password = o.value("user_password").toString();
	else if (o.contains("password"))
		password = o.value("password").toString();

	if (o.contains("user_address"))
		address = o.value("user_address").toString();
	else if (o.contains("address"))
		address = o.value("address").toString();

	if (o.contains("user_gender"))
		gender = o.value("user_gender").toString();
	else if (o.contains("gender"))
		gender = o.value("gender").toString();

	qDebug() << "[Person] updated id=" << id
		<< "name=" << name
		<< "money=" << money;
}