#include "end.h"
#include "ui_end.h"
#include "bird.h"

End::End(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::End)
{
    ui->setupUi(this);
    this->setGeometry(QRect(600,300,480,480));
    QPixmap pixmap(":/myImage/images/sky.png");
    QPalette palette;
    palette.setBrush(backgroundRole(), QBrush(pixmap));
    setPalette(palette);
    this->setWindowTitle("游戏结束");
}

End::~End()
{
    delete ui;
}

void End::on_pushButton_clicked()
{
    this->close();
    Bird *b=new Bird;
    b->show();
}

void End::on_pushButton_2_clicked()
{
    this->close();
}

void End::manage(int a, int b){
    thisjump=a;
    if(b>maxijump){
        maxijump=b;
    }
    ui->this_num->setText(QString::number(thisjump));
    ui->maxi_num->setText(QString::number(maxijump));
}

int End::maxijump=0;
