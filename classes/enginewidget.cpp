#include <QPainter>
#include "enginewidget.h"

EngineWidget::EngineWidget(QWidget *parent) :
    QWidget(parent)
{
    for (int i=0 ; i<10 ; i++)
        this->layers[i] = NULL;
}

void EngineWidget::SetLayerImage(int layer, QImage* img)
{
    this->layers[layer] = img;
    this->repaint();
}

void EngineWidget::paintEvent(QPaintEvent *) {
    QPainter p(this); // Создаём новый объект рисовальщика
    p.fillRect(0,0,width(),height(),QBrush(Qt::white));

    for (int i=0 ; i<10 ; i++)
        if (this->layers[i] != 0)
            p.drawImage(0,0, *this->layers[i]);

    /*p.setPen(QPen(Qt::red,1,Qt::SolidLine)); // Настройки рисования
    p.drawLine(0,0,width(),height()); // Рисование линии*/
}
