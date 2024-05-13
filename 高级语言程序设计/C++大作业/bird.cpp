#include "bird.h"
#include "ui_bird.h"
Bird::Bird(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Bird)
{
    ui->setupUi(this);
    this->setGeometry(QRect(600,300,480,480));
    this->setWindowTitle("进击的小鸟！");
    this->grabKeyboard();
}

Bird::~Bird()
{
    delete ui;
}

void Bird::paintEvent(QPaintEvent *event){
    QPainter painter(this);
    if(!blsRun){
        InitBird();
    }

    //画游戏背景
    QRect background(0,0,480,400);
    painter.drawPixmap(background,QPixmap(":/myImage/images/sky.png"));
    painter.setPen(Qt::white);
    painter.setBrush(Qt::white);
    QRect bottom(0,400,480,100);
    painter.drawRect(bottom);

    //画鸟
    painter.drawPixmap(vBird,QPixmap(":/myImage/images/bird.png"));//插入图片

    //画障碍
    for(int i=0;i<4;i++){
        painter.drawPixmap(barrier[i].down,QPixmap(":/myImage/images/barrier.png"));
        painter.drawPixmap(barrier[i].up,QPixmap(":/myImage/images/barrier.png"));
    }

    if(blsOver){
        timer1->stop();
        this->close();
        e=new End;
        e->manage(total_jump,total_jump);
        e->show();
    }
}

void Bird::InitBird(){
    blsRun=true;
    QRect rect(80,160,40,40);
    vBird=rect;

    for(int i=0;i<4;i++){
        barrier[i].up.setBottom(0);
        barrier[i].down.setTop(480);
    }
    CreateBarrier();

    timer1=new QTimer(this);
    timer1->start(100);
    connect(timer1,SIGNAL(timeout()),SLOT(BirdJudge()));
    connect(timer1,SIGNAL(timeout()),SLOT(Bird_update()));

}

void Bird::keyPressEvent(QKeyEvent *event){
    QKeyEvent *key=(QKeyEvent*)event;
    switch(key->key()){
    case Qt::Key_Up:nDirection=1;
        break;
    case Qt::Key_Down:nDirection=2;
        break;
    default:;
    }
}

void Bird::keyReleaseEvent(QKeyEvent *event){
    QKeyEvent *key=(QKeyEvent*)event;
    switch(key->key()){
    case Qt::Key_Up:nDirection=0;
        break;
    case Qt::Key_Down:nDirection=0;
        break;
    default:;
    }
}

void Bird::Bird_update(){
    time++;
    speed+=0.1;
    switch(nDirection){
    case 1:
        vBird.setTop(vBird.top()-20);
        vBird.setBottom(vBird.bottom()-20);
        break;
    case 2:
        vBird.setTop(vBird.top()+20);
        vBird.setBottom(vBird.bottom()+20);
        break;
    default:;
    }

    for(int i=0;i<4;i++){
        barrier[i].up.setLeft(barrier[i].up.left()-(15+speed));
        barrier[i].up.setRight(barrier[i].up.right()-(15+speed));
        barrier[i].down.setLeft(barrier[i].down.left()-(15+speed));
        barrier[i].down.setRight(barrier[i].down.right()-(15+speed));
    }

    //判断是否需要新建障碍
    double judge_bar=15*time+0.05*time*time;
    if(judge_bar>=bar_num*150){
        CreateBarrier();
        bar_num++;
    }

    update();
    QString totaljump=QString::number(total_jump);
    ui->number->setText(totaljump);

}

void Bird::BirdJudge(){
    //判断游戏是否结束
    for(int i=0;i<4;i++){
        if((vBird.right()>barrier[i].up.left())&&(vBird.left()<barrier[i].up.right())){
            if((vBird.top()<barrier[i].up.bottom())||(vBird.bottom()>barrier[i].down.top())){
                blsOver=true;

                break;
                update();
                return;
            }
        }
    }

    //判断是否越过新的障碍
    for(int i=0;i<4;i++){
        if((vBird.right()>barrier[i].up.left())&&(vBird.left()<barrier[i].up.right())){
            if((vBird.top()>barrier[i].up.bottom())&&(vBird.bottom()<barrier[i].down.top())){
                if(i==jump_n){
                    break;
                }
                jump_n=i;
                total_jump++;
                break;
            }
        }
    }


    update();//paintEvent更新
}

//创建障碍
void Bird::CreateBarrier(){
    int BarrierUp;
    BarrierUp=2+qrand()%4;
    int BarrierLength;
    BarrierLength=2+qrand()%2;
    int m;
    m=BarrierUp+BarrierLength;

    QRect rectup(440,0,40,BarrierUp*40);
    QRect rectdown(440,m*40,40,(10-m)*40);
    barrier[barriernum%4].up=rectup;
    barrier[barriernum%4].down=rectdown;
    barriernum++;

    update();
}


void Bird::on_stopBtn_clicked()
{
    QString button_style="QPushButton{background-color:#4682B4;color:white}";
    QString button_style2="QPushButton:pressed{backgroung-color:white;color:#4682B4;border-style:inset;}";
    if(blsStop==false){
        ui->stopBtn->setText(QString::fromUtf8("继续"));
        ui->stopBtn->setStyleSheet(button_style2);
        timer1->stop();
        blsStop=true;
    }
    else{
        ui->stopBtn->setText(QString::fromUtf8("暂停"));
        ui->stopBtn->setStyleSheet(button_style);
        timer1->start(100);
        blsStop=false;
    }
}

void Bird::on_pushButton_clicked()
{
    Bird* newBird=new Bird();
    this->close();
    newBird->show();
}
