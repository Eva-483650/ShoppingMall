#ifndef MEITEM_H
#define MEITEM_H

#include <QWidget>

namespace Ui {
class MeItem;
}

class MeItem : public QWidget
{
    Q_OBJECT

public:
    explicit MeItem(QWidget *parent = nullptr);
    ~MeItem();

    void setKey(QString str);
    void setVal(int num);
    void setVal(QString str);
    void setPic(QString picadd);

private:
    Ui::MeItem *ui;
};

#endif // MEITEM_H
