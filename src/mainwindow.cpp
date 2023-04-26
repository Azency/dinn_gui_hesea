#include "mainwindow.h"
#include "src/nn_hesea.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QPalette>
#include <QString>

using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_dinn()  
{
    ui->setupUi(this);
    this->setWindowIcon(QIcon("../untitled/Image/icon.jpg"));
    this->setWindowTitle(("李鹏博"));
    //设置按钮颜色
//    QPalette pal = (ui->openAction)->palette();
//    pal.setColor(QPalette::ButtonText, Qt::red);
//    al.setColor(QPalette::Button, Qt::green);
//    ui->openAction->setPalette(pal);

}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::on_openAction_clicked()
{
    //读取图片的路径
    QString filename;
    filename = QFileDialog::getOpenFileName(this, tr("Select image:"),
        "D:\\Documents\\Pictures", tr("Images (*.png *.bmp *.jpg *.gif)"));
    if (filename.isEmpty()) {
        return ;
    }
    //读取的图片
    QImage image;
    if (!image.load(filename)) {
        QMessageBox::information(this, tr("Error"), tr("Open file error"));
        return ;
    }

    setWindowTitle(QFileInfo(filename).fileName() + tr(" - imageViewer"));
    
    //转换灰度
    QImage imagegry = image.convertToFormat(QImage::Format_Grayscale8);
    
    QPixmap pixmap = QPixmap::fromImage(imagegry); //
    qDebug() << "filname: " << pixmap;
    //放大图片
    qreal width = pixmap.width();
    qreal height = pixmap.height();
    pixmap = pixmap.scaled(width*4,height*4,Qt::KeepAspectRatio);

    ui->imageLabel->setPixmap(pixmap);
    ui->imageLabel->resize(pixmap.size());

    ui->imageLabel->setPixmap(pixmap.scaled(ui->imageLabel->size(), Qt::IgnoreAspectRatio     , Qt::SmoothTransformation));

    unsigned char *pData=imagegry.bits();
   for(int i=0;i<16*16;i+=1)
   {
       if (pData[i]<128){
           pData[i]=0;
           m_dinn.m_image[i]=-1;
       }
       else{
           pData[i]=1;
           m_dinn.m_image[i]=1;
       }
   }

}

void MainWindow::on_changeAction_clicked()
{
    m_dinn.enc_score = m_dinn.net(m_dinn.enc_image);
    cout<<"net runs over"<<endl;

}

void MainWindow::on_encryptAction_clicked()
{
    int count =0;
    int grey=0;
    m_dinn.enc_image = m_dinn.encrypt(m_dinn.m_image);
    QString filename;
    filename = QFileDialog::getOpenFileName(this, tr("Select image:"),
        "D:\\Documents\\Pictures", tr("Images (*.png *.bmp *.jpg *.gif)"));
}


void MainWindow::on_decryptAction_clicked()
{
    int image_class;
    image_class = m_dinn.decrypt(m_dinn.enc_score);
    ui->dataEdit->setText(QString::number(image_class));

}

