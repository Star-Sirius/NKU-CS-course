#ifndef BIRD_H
#define BIRD_H

#include <QMainWindow>
#include <qpainter.h>
#include <QRect>
#include <QTimer>
#include <QKeyEvent>
#include <end.h>

QT_BEGIN_NAMESPACE
namespace Ui { class Bird; }
QT_END_NAMESPACE


struct bar{
    QRect up,down;
};

class Bird : public QMainWindow
{
    Q_OBJECT

public:
    Bird(QWidget *parent = nullptr);
    ~Bird();
    void paintEvent(QPaintEvent *event);
    void InitBird();//初始化鸟


private slots:
    void Bird_update();
    void CreateBarrier();
    void BirdJudge();


    void on_stopBtn_clicked();

    void on_pushButton_clicked();

private:
    Ui::Bird *ui;
    QRect vBird;
    bool blsRun=false;//是否开始
    bool blsOver=false;//是否结束
    QTimer *timer1;
    int nDirection=0;
    void keyPressEvent(QKeyEvent *key);
    void keyReleaseEvent(QKeyEvent *key);
    bar barrier[4];
    int barriernum=0;
    End *e;
    bool blsStop=false;
    int jump_n=5;
    int total_jump=0;
    double speed=0;
    int time=0;
    int bar_num=1;

};
#endif // BIRD_H
