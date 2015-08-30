#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <opencv2/highgui.hpp>
#include <opencv2/core.hpp>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0, int argc = 0, char** argv = NULL);
    ~MainWindow();

public slots:

    void selectFile();
    void update();

private:
    Ui::MainWindow *ui;

    int argc;
    char** argv;
    std::string vid;
    cv::Mat src;
    cv::VideoCapture inputvideo;
    QGraphicsScene *scn;
    QGraphicsPixmapItem *item;

};

#endif // MAINWINDOW_H
