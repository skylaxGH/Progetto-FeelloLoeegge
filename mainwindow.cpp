#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    nRequest = new nameRequest(ui->listaPlaylist, nullptr);
    connect(nRequest, &nameRequest::transmit, this, &MainWindow::updateName);

    player->setMedia(nullptr);

    this->setFixedSize(this->size());  //La grandezza della finestra non può essere modificata (dimensione fissa)

    ui->volumeSlider->setVisible(false);                            //Lo slider del volume di default non è visibile
    ui->volumeSlider->setStyleSheet("QSlider::groove:horizontal {"  //Modifico la stylesheet dello slider del volume
                                                                    //per renderlo un po' più bello (quello di Windows
                                       "height: 7px;"               //di default è bruttissimo)
                                       "border: 1px solid #999999;"

                                    "}"

                                    "QSlider::handle:horizontal {"
                                       "width: 9px;"
                                       "background: rgb(255, 77, 0);"
                                       "margin: -5px 0;"
                                       "border-radius: 5px;"
                                       "border: 0.5px solid;"

                                    "}"

                                    "QSlider::sub-page:horizontal {"
                                       "background: rgb(255, 77, 0);"
                                    "}"

                                    "QSlider::add-page:horizontal {"
                                       "background: white;"
                                    "}");

    ui->progressSlider->setStyleSheet("QSlider::groove:horizontal {"      //Modifico la style sheet anche della progress bar/slider
                                        "height: 10px;"
                                        "border: 1px solid #999999;"
                                      "}"

                                      "QSlider::handle:horizontal {"
                                         "width: 10px;"
                                         "background: rgb(255, 77, 0);"
                                         "margin: -5px 0;"
                                         "border-radius: 5px;"
                                         "border: 0.5px solid;"
                                      "}"

                                      "QSlider::add-page:horizontal {"
                                         "background: white;"
                                      "}"

                                      "QSlider::sub-page:horizontal {"
                                         "background: rgb(255, 77, 0);"
                                      "}");

    ui->listaPlaylist->setStyleSheet("QListView {"                      //Modifico la style sheet della listWidget delle playlist
                                        "show-decoration-selected: 1;"
                                        "background-color: #202020;"
                                        "border-right: 1.2px solid rgb(255, 77, 0);"
                                        "color: white;"
                                     "}"

                                     "QListView::item:alternate {"
                                        "background: #EEEEEE;"
                                     "}"

                                     "QListView::item:selected {"
                                        "border: 1px solid #6a6ea9;"
                                     "}"

                                     "QListView::item:selected:!active {"
                                        "background:"
                                     "}"

                                     "QListView::item:selected:active {"
                                        "background:"
                                     "}"

                                     "QListView::item:hover {"
                                        "background:"

                                     "}");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::selezioneFile() {
    path = QFileDialog::getOpenFileName(this, "Apri un file", "C://", "*.mp3 *.mp4 *.wav *.wma *.aac *.m4a");  //Apro una finestra di comunicazione con
                                                                                                               //l'utente per l'apertura del file

    for(int i = 0; i < path.length() - 1; i++){   //Sostituisco gli slash con i backslash perchè
                                                  //QT non riconosce gli indirizzi di windows in quanto
        if(path[i] == '/') path[i] = '\\' ;       //usa gli indirizzi UNIX
    }

    QFile songPath(path);
    QFileInfo songInfo(songPath.fileName());
    QString songName(songInfo.fileName());

    songName.replace(".mp3", "");

    ui->songName->setText(songName);

    player->setMedia(QUrl::fromLocalFile(path));  //Imposto come file per la riproduzione il file scelto da utente per indirizzo

    player->setVolume(ui->volumeSlider->value()); //Imposto il volume di base al valore del volumeSlider
    player->play();                               //Faccio partire la riproduzione

    /* Connetto tutti gli slot di segnale tra lo stream dell'audio e i vari slider/progress bar
       per aumentare/diminuire posizione/volume */

    connect(player, &QMediaPlayer::durationChanged, this, &MainWindow::on_durationChanged);
    connect(ui->volumeSlider, SIGNAL(valueChanged(int)), player, SLOT(setVolume(int)));   //Il valore int ritornato da valueChanged del volumeSlider sara il valore int dato come parametro a setVolume del player
    connect(player, &QMediaPlayer::positionChanged, this, &MainWindow::on_positionChanged);
}



void MainWindow::on_btnRiproduci_clicked() {
    QMessageBox box;

    if(path == "") {
        box.setWindowTitle("Errore");
        box.setText("Non hai selezionato nessuna traccia audio, vuoi selezionarne una?");
        box.addButton(QMessageBox::Open);
        box.addButton(QMessageBox::No);
        box.exec();

        if(box.Open) selezioneFile();
    }

    /*else if(path.length() > 0) {
        QMessageBox::StandardButton answer = QMessageBox::question(this, "Avviso", "C'è già un file in riproduzione, vuoi selezionarne un'altro?",
                                                                   QMessageBox::Yes | QMessageBox::No);
        if(answer == QMessageBox::Yes) {
            player->stop();
            selezioneFile();
        }
    }*/

    else if(player->state() == QMediaPlayer::PausedState) player->play();
}




void MainWindow::on_btnPausa_clicked() {
    QMessageBox box;

    if(player->isAudioAvailable() == false) {
        box.setWindowTitle("Errore");
        box.setText("Non c'è nessuna canzone in riproduzione");
        box.exec();
    }

    else player->pause();
}

void MainWindow::on_btnStop_clicked() {
    QMessageBox box;

    if(player->isAudioAvailable() == false) {  //In caso il file audio non sia disponibile per la riproduzione..
        box.setWindowTitle("Errore");
        box.setText("Non c'è nessuna canzone in riproduzione");
        box.exec(); //Messaggio di errore
    }

    else {
        player->stop();             //Negli altri casi stoppo la riproduzione, do valore nullptr a setMedia per segnalare che non è disponibile
        player->setMedia(nullptr);  //nessun file audio
        path = "";
        ui->songName->clear();      //Pulisco il file songName così posso prendere in input il nome del file di un'altra canzone
        disconnect(ui->volumeSlider, SIGNAL(valueChanged(int)), player, SLOT(setVolume(int)));     //Disconnetto i vari slot di segnale tra stream audio e slider/progress bar
        disconnect(player, &QMediaPlayer::positionChanged, this, &MainWindow::on_positionChanged);
        disconnect(player, &QMediaPlayer::durationChanged, this, &MainWindow::on_durationChanged);
    }
}

void MainWindow::on_btnVolume_clicked() {
    if(slider == true) ui->volumeSlider->setVisible(false);
    else {
        ui->volumeSlider->setVisible(true);
    }

    slider = !slider;
}

void MainWindow::on_progressSlider_sliderMoved(int position) {
    if(player->isAudioAvailable() == false) {}
    else player->setPosition(position);
}

void MainWindow::on_positionChanged(qint64 position) {
    double sec = 0;

    ui->progressSlider->setValue(position);

    sec = static_cast<double>(position) / 1000;

    sec ++;

        if (int (sec) - (min * 60) == 60) {
            min++;
            ui->timeLabel->setText(QString::number(static_cast<int>(min)) + ":" + QString::number(static_cast<int>(sec) - (min * 60)));
   }

        else{

            ui->timeLabel->setText(QString::number(static_cast<int>(min)) + ":" + QString::number(static_cast<int>(sec) - (min * 60)));
        }
}

void MainWindow::on_durationChanged(qint64 position) {
    double min = 0, sec = 0;

    ui->progressSlider->setMaximum(position);

    sec = static_cast<double>(position) / 1000;
    if(int(sec) % 60 == 0) {
        min = sec / 60;
        ui->durationLabel->setText(QString::number(min) + ":00");
    }

    else {
        min = sec / 60;
        sec = (int(sec) % 60) + 1;
        ui->durationLabel->setText(QString::number(static_cast<int>(min)) + ":" + QString::number(sec));
    }
}

void MainWindow::on_btnCreaplaylist_clicked()
{
    nRequest->setModal(true);
    nRequest->show();
}

void MainWindow::updateName(QString name) {
    ui->listaPlaylist->addItem(name);
}
