#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtMultimedia/QMediaPlayer>
#include <QMessageBox>
#include <namerequest.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_btnVolume_clicked();

    void on_btnRiproduci_clicked();

    void on_btnPausa_clicked();

    void on_btnStop_clicked();

    void on_positionChanged(qint64 position);

    void on_durationChanged(qint64 position);

    void on_progressSlider_sliderMoved(int position);

    void on_btnCreaplaylist_clicked();

    void updateName(QString name);

private:
    Ui::MainWindow *ui;
    nameRequest *nRequest;
    bool slider = false;

    QMediaPlayer* player = new QMediaPlayer;
    QString path = "";
    double min = 0;

    void selezioneFile();
};
#endif // MAINWINDOW_H
