#include "customcombobox.h"

CustomComboBox::CustomComboBox(QWidget* parent)
    : QComboBox(parent)
{
    setStyleSheet(
        "QComboBox {"
        "    background: rgba(255, 255, 255, 0.9);"
        "    border: 2px solid rgba(214, 201, 247, 0.5);"
        "    border-radius: 8px;"
        "    padding: 8px 30px 8px 12px;"
        "    font: 10pt '微软雅黑';"
        "    color: #5a4c74;"
        "    min-width: 100px;"
        "}"
        "QComboBox:hover {"
        "    border: 2px solid #a18cd1;"
        "}"
        "QComboBox:focus {"
        "    border: 2px solid #a18cd1;"
        "}"
        "QComboBox::drop-down {"
        "    border: none;"
        "    background: transparent;"
        "}"
        "QComboBox::down-arrow {"
        "    image: none;"
        "    width: 0px;"
        "    height: 0px;"
        "}"
        "QComboBox QAbstractItemView {"
        "    background: white;"
        "    border: 1px solid #d6c9f7;"
        "    border-radius: 8px;"
        "    selection-background-color: #e8e6fb;"
        "    outline: none;"
        "}"
        );
}

void CustomComboBox::paintEvent(QPaintEvent* event)
{
    QComboBox::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 获取下拉按钮区域
    QRect dropDownRect = rect();
    dropDownRect.setLeft(dropDownRect.right() - 25);

    // 绘制箭头
    drawArrow(painter, dropDownRect, false);
}

void CustomComboBox::drawArrow(QPainter& painter, const QRect& rect, bool isPressed)
{
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(QColor("#6c5b7b")));

    int centerX = rect.center().x();
    int centerY = rect.center().y();

    QPolygon arrow;
    if (isPressed) {
        // 向上箭头
        arrow << QPoint(centerX - 5, centerY + 2)
              << QPoint(centerX + 5, centerY + 2)
              << QPoint(centerX, centerY - 3);
    } else {
        // 向下箭头
        arrow << QPoint(centerX - 5, centerY - 2)
              << QPoint(centerX + 5, centerY - 2)
              << QPoint(centerX, centerY + 3);
    }

    painter.drawPolygon(arrow);
}
