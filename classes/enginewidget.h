#ifndef ENGINEWIDGET_H
#define ENGINEWIDGET_H

#include <QWidget>

class EngineWidget : public QWidget
{
    Q_OBJECT
public:
    explicit EngineWidget(QWidget *parent = 0);

signals:

public slots:
    void SetLayerImage(int layer, QImage* img);

protected:
    void paintEvent(QPaintEvent *);

private:
    QImage* layers[10];

};

#endif // ENGINEWIDGET_H
