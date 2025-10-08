#ifndef CUSTOMSPINBOX_H
#define CUSTOMSPINBOX_H

#include <QSpinBox>
#include <QPainter>
#include <QStyleOptionSpinBox>
#include <QMouseEvent>
#include <QRect>

class CustomSpinBox : public QSpinBox
{
    Q_OBJECT

public:
    explicit CustomSpinBox(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    void drawButtons(QPainter& painter);
    void drawPlusButton(QPainter& painter, const QRect& rect, bool pressed = false);
    void drawMinusButton(QPainter& painter, const QRect& rect, bool pressed = false);
    QRect getUpButtonRect() const;
    QRect getDownButtonRect() const;

    bool m_upPressed = false;
    bool m_downPressed = false;
};

#endif // CUSTOMSPINBOX_H
