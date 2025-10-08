#include "customspinbox.h"

CustomSpinBox::CustomSpinBox(QWidget* parent)
    : QSpinBox(parent)
{
    setStyleSheet(
        "QSpinBox {"
        "    background: rgba(255, 255, 255, 0.8);"
        "    border: 2px solid rgba(214, 201, 247, 0.5);"
        "    border-radius: 8px;"
        "    padding: 8px 25px 8px 8px;"  // 为按钮留出空间
        "    font: 10pt '微软雅黑';"
        "    color: #5a4c74;"
        "}"
        "QSpinBox:focus {"
        "    border: 2px solid #a18cd1;"
        "}"
        "QSpinBox::up-button {"
        "    background: transparent;"
        "    border: none;"
        "    width: 0px;"
        "}"
        "QSpinBox::down-button {"
        "    background: transparent;"
        "    border: none;"
        "    width: 0px;"
        "}"
        "QSpinBox::up-arrow {"
        "    image: none;"
        "    width: 0px;"
        "    height: 0px;"
        "}"
        "QSpinBox::down-arrow {"
        "    image: none;"
        "    width: 0px;"
        "    height: 0px;"
        "}"
        );
}

void CustomSpinBox::paintEvent(QPaintEvent* event)
{
    QSpinBox::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    drawButtons(painter);
}

void CustomSpinBox::drawButtons(QPainter& painter)
{
    QRect upRect = getUpButtonRect();
    QRect downRect = getDownButtonRect();

    drawPlusButton(painter, upRect, m_upPressed);
    drawMinusButton(painter, downRect, m_downPressed);
}

void CustomSpinBox::drawPlusButton(QPainter& painter, const QRect& rect, bool pressed)
{
    // 绘制按钮背景
    QColor bgColor = pressed ? QColor(161, 140, 209, 128) : QColor(184, 164, 230, 77);
    painter.fillRect(rect, QBrush(bgColor));

    // 绘制边框
    painter.setPen(QPen(QColor(161, 140, 209, 102), 1));
    painter.drawRoundedRect(rect, 3, 3);

    // 绘制 + 号
    painter.setPen(QPen(QColor("#6c5b7b"), 2));

    int centerX = rect.center().x();
    int centerY = rect.center().y();
    int size = 6;

    // 水平线
    painter.drawLine(centerX - size/2, centerY, centerX + size/2, centerY);
    // 垂直线
    painter.drawLine(centerX, centerY - size/2, centerX, centerY + size/2);
}

void CustomSpinBox::drawMinusButton(QPainter& painter, const QRect& rect, bool pressed)
{
    // 绘制按钮背景
    QColor bgColor = pressed ? QColor(161, 140, 209, 128) : QColor(184, 164, 230, 77);
    painter.fillRect(rect, QBrush(bgColor));

    // 绘制边框
    painter.setPen(QPen(QColor(161, 140, 209, 102), 1));
    painter.drawRoundedRect(rect, 3, 3);

    // 绘制 - 号
    painter.setPen(QPen(QColor("#6c5b7b"), 2));

    int centerX = rect.center().x();
    int centerY = rect.center().y();
    int size = 6;

    // 水平线
    painter.drawLine(centerX - size/2, centerY, centerX + size/2, centerY);
}

QRect CustomSpinBox::getUpButtonRect() const
{
    QRect widgetRect = rect();
    int buttonWidth = 20;
    int buttonHeight = (widgetRect.height() - 6) / 2;

    return QRect(widgetRect.right() - buttonWidth - 3,
                 widgetRect.top() + 3,
                 buttonWidth,
                 buttonHeight);
}

QRect CustomSpinBox::getDownButtonRect() const
{
    QRect widgetRect = rect();
    int buttonWidth = 20;
    int buttonHeight = (widgetRect.height() - 6) / 2;

    return QRect(widgetRect.right() - buttonWidth - 3,
                 widgetRect.top() + 3 + buttonHeight + 1,
                 buttonWidth,
                 buttonHeight);
}

void CustomSpinBox::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        QRect upRect = getUpButtonRect();
        QRect downRect = getDownButtonRect();

        if (upRect.contains(event->pos())) {
            m_upPressed = true;
            stepUp();
            update();
            return;
        } else if (downRect.contains(event->pos())) {
            m_downPressed = true;
            stepDown();
            update();
            return;
        }
    }

    QSpinBox::mousePressEvent(event);
}

void CustomSpinBox::mouseReleaseEvent(QMouseEvent* event)
{
    m_upPressed = false;
    m_downPressed = false;
    update();

    QSpinBox::mouseReleaseEvent(event);
}
