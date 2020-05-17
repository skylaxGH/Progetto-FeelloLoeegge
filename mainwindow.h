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

    void on_btnAggiungip_clicked();

    void on_actionApri_un_file_triggered();

    void on_actionStart_triggered();

    void on_actionStop_triggered();

    void on_actionPausa_triggered();

private:
    Ui::MainWindow *ui;
    nameRequest *nRequest;  //Dichiaro un puntatore a nameRequest per chiamare la seconda finestra dalla prima
    bool slider = false;    //Bool per la visibilità/non visibilità dello sliderVolume

    QMediaPlayer* player = new QMediaPlayer; //Dichiaro un puntatore a QMediaPlayer per la riproduzione audio
    QString path = "";      //Dichiaro una variabile QString per il salvataggio della path del file selezionato da utente
    double min = 0;         //Dichiaro una variabile double per il conteggio dei minuti totali/di riproduzione della traccia
    int row = 1;
    QString playlistFile = "/Playlist.txt";

    void selezioneFile(int n);
};
#endif // MAINWINDOW_H
