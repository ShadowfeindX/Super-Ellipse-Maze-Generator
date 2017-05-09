#ifndef CANVAS_HXX
#define CANVAS_HXX

#include <QtCore>
#include <QtCoreDepends>
#include <QtGui>
#include <QtGuiDepends>
#include <QtWidgets>
#include <QtWidgetsDepends>

class Canvas : public QWidget
{
    Q_OBJECT

public:
    Canvas(QWidget *parent = Q_NULLPTR);

public slots:
    void timerEvent(QTimerEvent *);
    void paintEvent(QPaintEvent *);
};

#endif // CANVAS_HXX
