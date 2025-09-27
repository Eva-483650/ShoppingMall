#include "sqlserver.h"
#include<QMessageBox>

// 构造函数1：无参数
SQLServer::SQLServer()
{

}

// 构造函数2：含数据库连接参数
SQLServer::SQLServer(QString SQLkind, QString ip, int port, QString dbname, QString username, QString password, QObject* parent) :QObject(parent) {
    // 判断是否已有默认连接
    if (QSqlDatabase::contains("qt_sql_default_connection")) {
        db = QSqlDatabase::database("qt_sql_default_connection");
    }
    else {
        db = QSqlDatabase::addDatabase(SQLkind); // 新建连接
    }
    database_IP = ip;
    database_port = port;
    database_username = username;
    database_dbname = dbname;
    database_password = password;
}

// 连接数据库
bool SQLServer::connectToDataBase() {
    db.setHostName(database_IP);        // 设置数据库IP
    db.setPort(database_port);          // 设置端口
    db.setDatabaseName(database_dbname);// 设置库名
    db.setUserName(database_username);  // 设置用户名
    db.setPassword(database_password);  // 设置密码
    if (db.open()) {
        qDebug() << "连接数据库成功！";
        //emit signal_connectDatabase(true); // 可选信号
        return true;
    }
    else {
        qDebug() << "连接数据库失败！";
        //emit signal_connectDatabase(false); // 可选信号
    }
    return false;
}

// 将SQL查询结果转为JSON数组
QJsonArray SQLServer::getSQLRes(QSqlQuery* q) {
    QJsonArray res;
    // 执行完exec函数后先next一次才是第一个数据
    while (q->next())
    {
        QJsonObject obj;
        // 获取每条记录中属性（即列）的个数
        int columnNum = q->record().count();
        // 遍历所有列
        for (int i = 0; i < columnNum; i++) {
            // 统一转换为字符串
            obj.insert(q->record().fieldName(i), QJsonValue(q->value(i).toString()));
        }
        res.append(QJsonValue(obj)); // 将一行数据放入结果数组
    }
    return res;
}

// 获取表的所有列名
QList<QString> SQLServer::getTableInfo(QString table) {
    QList<QString> res;
    QSqlQuery q(db);
    q.exec(QString("show columns from %1 ").arg(table));
    while (q.next())
    {
        int columnNum = q.record().count();
        res.append(q.record().field(0).value().toString());//获取表头（第1列字段名）
    }
    /*
    // 调试输出所有字段名
    for(int i = 0; i < res.size(); i++){
        qDebug()<<res.at(i);
    }
    */
    return res;
}

// 查询数据
bool SQLServer::selectSth(QString table, QJsonObject obj, QJsonArray& result) {
    QString want;          // 查询的字段
    QString restriction = "";
    QJsonArray res;
    bool isDistinct = false;

    // 判断是否指定查询字段
    if (obj.contains("want")) {
        want = obj.value("want").toString();
        // 判断是否需要DISTINCT
        if (obj.contains("isDistinct") && obj.value("isDistinct").toString().toLower() == "true") { isDistinct = true; }
        // 判断是否有查询条件
        if (obj.contains("restriction")) {
            restriction = obj.value("restriction").toString();
        }
        QSqlQuery q(db);
        QString content = "SELECT ";
        if (isDistinct) { content += "DISTINCT "; }
        content += want;
        content += QString(" FROM %1 ").arg(table);
        if (restriction != "") {
            content += "WHERE ";
            content += restriction;
        }
        // 执行SQL查询
        q.exec(content);
        QJsonArray res = getSQLRes(&q); // 转为JSON
        result = res;
        qDebug() << "Select STH!" << content; // 调试输出SQL
        return true;
    }
    return false;
}

// 插入数据
bool SQLServer::insertSth(QString table, QJsonObject obj) {
    QVariantMap map;
    // 将obj转为map便于插入
    for (int i = 0; i < obj.keys().size(); i++) {
        QString key = obj.keys().at(i);
        QJsonValue value = obj.value(obj.keys().at(i));
        map.insert(key, value);
    }
    // 获取表所有字段名（用于校验）
    QList<QString> SQLhead = getTableInfo(table);
    if (SQLhead.isEmpty()) {
        QMessageBox::warning(nullptr, "错误", QString("表%1为空或不存在！").arg(table));
        return false;
    }
    else {
        // 拼接SQL语句
        QString insertTableContent = QString("INSERT INTO %1 (").arg(table);
        QString values = QString("VALUES (");
        for (int i = 0; i < map.keys().size(); i++) {
            insertTableContent += map.keys().at(i);
            values += "'" + map.value(map.keys().at(i)).toString() + "'";
            if (i < map.keys().size() - 1) { insertTableContent += ","; values += ","; }
        }
        insertTableContent += ")";
        values += ") ";
        insertTableContent += values;
        qDebug() << "INSERT STH!" << insertTableContent; // 调试输出
        QSqlQuery q(db);
        return q.exec(insertTableContent);
    }
}

// 更新数据
bool SQLServer::updateSth(QString table, QJsonObject obj) {
    QVariantMap map;
    QString restriction = "";
    // 获取WHERE条件
    if (obj.contains("restriction")) {
        restriction = obj.value("restriction").toString();
        obj.remove("restriction");
    }
    else { return false; }
    // 收集要更新的字段和值
    for (int i = 0; i < obj.keys().size(); i++) {
        QString key = obj.keys().at(i);
        QJsonValue value = obj.value(obj.keys().at(i));
        map.insert(key, value);
    }
    // 拼接SQL语句
    QString content = QString("UPDATE %1 SET ").arg(table);
    QMapIterator<QString, QVariant> it(map);
    while (it.hasNext()) {
        it.next();
        content += QString("%1 = '%2'").arg(it.key()).arg(it.value().toString());
        if (it.hasNext()) { content += ","; }
    }
    content += "WHERE ";
    content += restriction;
    qDebug() << "UPDATE STH!" << content; // 调试输出
    QSqlQuery q(db);
    return q.exec(content);
}

// 删除数据
bool SQLServer::deleteSth(QString table, QJsonObject obj) {
    QString restriction = "";
    // 获取WHERE条件
    if (obj.contains("restriction")) {
        restriction = obj.value("restriction").toString();
    }
    else { return  false; }
    QString content = QString("DELETE FROM %1 ").arg(table);
    if (restriction.trimmed() == "") {
        return false;
    }
    else {
        content += "WHERE ";
        content += restriction;
        QSqlQuery q(db);
        qDebug() << "DELETE STH!" << content; // 调试输出
        return q.exec(content);
    }
}