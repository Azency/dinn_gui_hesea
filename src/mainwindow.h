#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "src/nn_hesea.h"
#include <QMainWindow>
#include <QFileDialog>
#include <QImage>
#include <QPixmap>
#include <QFileInfo>
#include <QMessageBox>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    DINN_HESEA m_dinn;

private slots:
    void on_openAction_clicked();

    void on_changeAction_clicked();

    void on_encryptAction_clicked();

    void on_decryptAction_clicked();
private:
    Ui::MainWindow *ui;

};
#endif // MAINWINDOW_H
