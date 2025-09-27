#ifndef PERSON_H
#define PERSON_H

#include <QString>
#include <QJsonObject>

class Person
{
public:
	// 允许先构造空对象，再用 updateFromJson 填充
	Person();
	explicit Person(const QJsonObject& obj); // 兼容旧代码

	// 更新（可多次调用，未提供的字段不覆盖已有值）
	void updateFromJson(const QJsonObject& obj);

	// 基本 getter
	int getId() const { return id; }
	int getMoney() const { return money; }
	const QString& getName() const { return name; }
	const QString& getAddress() const { return address; }
	const QString& getGender()  const { return gender; }
	const QString& getPassword()const { return password; }

	// 你原有的命名（保持兼容）
	int getmoney() { return money; }
	void setmoney(int num) { money = num; }

	// 如果你确实需要单独设置 id（一般不建议随意改）可以放开：
	void setId(int v) { id = v; }

private:
	// 归一化：把 obj 所有 key 转为小写，便于统一解析
	static QJsonObject normalizeKeys(const QJsonObject& src);

private:
	int id = 0;
	int money = 0;
public:  // 保持你之前这些是 public（如果想改成 private 可再继续）
	QString name;
	QString address;
	QString gender;
	QString password;
};

#endif // PERSON_H