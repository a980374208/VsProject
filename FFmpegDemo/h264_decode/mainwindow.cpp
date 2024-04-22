#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_audioButton_clicked() {
    _audioThread = new VideoThread(this);
    _audioThread->start();
    connect(_audioThread, &VideoThread::decoder_finished, this, [&]() {
        QMessageBox::information(this, "finished", "decoder finished");
        });
}
