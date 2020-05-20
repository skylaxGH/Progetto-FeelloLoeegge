#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QTextStream>
#include <QFile>
#include <QDir>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    nRequest = new nameRequest(ui->listaPlaylist, nullptr); //Assegno al puntatore una locazione di memoria heap della grandezza nameRequest (2a finestra)
    connect(nRequest, &nameRequest::transmit, this, &MainWindow::updateName);
    /*Connetto uno slot di segnale tra la funzione transmit della finestra e updateName del MainWindow
      così con l'istruzione emit posso dare il segnale di input riuscito al MainWindow*/

    sPlaylist = new sceltaPlaylist(ui->listaPlaylist, nullptr); //Assegno al puntatore una locazione di memoria heap della grandezza sceltaPlaylist (3a finestra)
    connect(sPlaylist, &sceltaPlaylist::transmit, this, &MainWindow::insertSong);
    connect(ui->listaPlaylist, &QListWidget::currentRowChanged, this, &MainWindow::change);

    /*Connetto uno slot di segnale tra la funzione transmit della finestra e updateName del MainWindow
      così con l'istruzione emit posso dare il segnale di input riuscito al MainWindow*/

    player->setMedia(nullptr); //Di default il player non ha contenuto da riprodurre
    ui->songName->setText("CODA DI RIPRODUZIONE"); //Di default il player si trova sulla coda attuale di riproduzione

    playlistFile = PROJECT_PATH + playlistFile; //Concateno le stringhe per creare l'indirizzo assoluto per il file Playlist

    QFile filePlaylist(playlistFile);
    filePlaylist.open(QIODevice::ReadOnly); //Apro il file (di default un file di testo) in sola lettura per leggere le playlist presenti
    QTextStream stream(&filePlaylist);      //Dichiaro uno stream collegato al file

    while (!stream.atEnd()) {
        QString line = stream.readLine();               //Leggo riga per riga dal file finchè non attivo alla fine

        QFile playlistPath(line);                        //Attraverso fileName() di QFileInfo posso ottenere solo il nome della
        QFileInfo playlistInfo(playlistPath.fileName()); //directory e non tutto l'indirizzo
        QString playlistName(playlistInfo.fileName());

        ui->listaPlaylist->addItem(playlistName);
    }

    filePlaylist.close();  //Chiudo lo stream con il file

    this->setFixedSize(this->size());  //La grandezza della finestra non può essere modificata (dimensione fissa)
    ui->volumeSlider->setVisible(false);  //Lo slider del volume di default non è visibile
    ui->btnElimina->setVisible(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

/*-------------------------------GESTIONE RIPRODUZIONE TRACCE------------------------------*/
void MainWindow::change() {
    disconnect(ui->listaPlaylist, &QListWidget::currentRowChanged, this, &MainWindow::change);
    ui->listaPlaylist->setCurrentRow(-1);
}


void MainWindow::selezioneFile(int n) {
    QDir pathDir;  //La variabile pathDir serve per l'utilizzo della funzione toNativeSeparators

    path = QFileDialog::getOpenFileName(this, "Apri un file", "C://", "*.mp3 *.wav");  //Apro una finestra di comunicazione con
                                                                                       //l'utente per l'apertura del file
    if(path != "") {
        path = pathDir.toNativeSeparators(path);   //Converto la path Windows ottenuta per la traccia audio in una path Unix
                                                   //per lavorare con QT

        QFile songPath(path);                      //Prendo la path della traccia audio in una QFile per usare filename()
        QFileInfo songInfo(songPath.fileName());   //Salvo in una QFileInfo il filename di songpath (quindi solo il nome della canzone)
        QString songName(songInfo.completeBaseName());  //Salvo il nome ottenuto in una string per mostrarla all'utente

        ui->listaTitolo->addItem(songName);        //Aggiungo il nome della canzone alla lista titoli

        if(n == 0) {
            coda->addMedia(QUrl::fromLocalFile(path));  //Aggiungo la traccia alla coda
            coda->setCurrentIndex(1);                   //Visto che è la prima traccia ad essere aggiunta, setto l'indice a uno

            player->setPlaylist(coda);                  //Come fonte di tracce per il player uso la coda

            player->setVolume(ui->volumeSlider->value()); //Imposto il volume di base al valore del volumeSlider
            player->play();                               //Faccio partire la riproduzione

            QListWidgetItem* item = ui->listaTitolo->item(0);                       //Istruzioni per la grafica. La prima traccia, che sia
            ui->listaTitolo->setSelectionMode(QAbstractItemView::SingleSelection);  //della coda o della playlist, sarà circondata di arancione
            item->setSelected(true);                                                //per far capire all'utente cosa stà riproducendo
            ui->listaTitolo->setSelectionMode(QAbstractItemView::NoSelection);

            /* Connetto tutti gli slot di segnale tra lo stream dell'audio e i vari slider/progress bar
               per aumentare/diminuire posizione/volume */

            connect(ui->volumeSlider, SIGNAL(valueChanged(int)), player, SLOT(setVolume(int)));     //Il valore int ritornato da valueChanged del volumeSlider sara il valore int dato come parametro a setVolume del player
            connect(player, &QMediaPlayer::durationChanged, this, &MainWindow::on_durationChanged); //Il valore qint64 di durationChanged, al suo variare, manderà un segnale alla funzione on_durationChanged, che ha come parametro un valore qint64
            connect(player, &QMediaPlayer::positionChanged, this, &MainWindow::on_positionChanged); //Il valore qint64 di positionChanged, al suo variare, manderà un segnale alla funzione on_positionChanged, che ha come parametro un valore qint64
            connect(player, &QMediaPlayer::mediaStatusChanged, this, &MainWindow::updateCoda);      //Al variare dello stato di riproduzione del player, arriverà un segnale alla funzione updateCoda
        }

        else coda->addMedia(QUrl::fromLocalFile(path));
    }
}


void MainWindow::updateCoda() {
    if(player->mediaStatus() == QMediaPlayer::EndOfMedia) {  //Visto che l'utente può aggiungere canzoni alla coda mentre c'è
        if(coda->mediaCount() > 1) {                         //già una canzone in riproduzione, bisogna gestire lo scorrimento
            coda->setCurrentIndex(coda->currentIndex() + 1); //ogni volta che il player da il segnale mediaStatusChanged, scorrendo
            player->play();                                  //la coda in caso di EndOfMedia
        }
    }

    else if(coda->mediaCount() == 0) {                       //Se non sono più presenti canzoni nella coda, disconnetto il player da qualsiasi fonte di traccia audio
        if(player->state() == QMediaPlayer::StoppedState) player->setMedia(nullptr);
    }
}

/*-------------------------------GESTIONE PULSANTI/SLIDER------------------------------*/

void MainWindow::on_btnRiproduci_clicked() {
    QMessageBox box;

    if(path == "" || !player->isAudioAvailable()) {   //In caso non ci sia nessuna canzone in riproduzione..
        box.setWindowTitle("Errore");                 //Chiedo all'utente se vuole scegliere una traccia
        box.setText("Non hai selezionato nessuna traccia audio, vuoi selezionarne una?");
        box.addButton("Apri", QMessageBox::AcceptRole);
        box.addButton(QMessageBox::No);
        int ret = box.exec();

        if(ret == QMessageBox::AcceptRole) selezioneFile(0); //In caso positivo (AcceptRole) faccio scegliere la traccia
    }

    else if(player->state() == QMediaPlayer::PausedState) player->play();  //In caso il player sia in pausa (quindi c'è una canzone in riproduzione)
                                                                           //questo tasto funge da semplicissimo tasto Play

    else if(player->isAudioAvailable() && ui->songName->text() == "CODA DI RIPRODUZIONE") { //In caso l'utente stia navigando nella coda

            box.setWindowTitle("Canzone già in riproduzione");        //Chiedo all'utente se vuole aggiungere un altra canzone alla coda
            box.setText("C'è una canzone già in riproduzione, vuoi metterne un altra in coda?");
            box.addButton("Apri", QMessageBox::AcceptRole);
            box.addButton(QMessageBox::No);
            int ret = box.exec();

            if(ret == QMessageBox::AcceptRole) selezioneFile(1);
        }
}

void MainWindow::on_btnPausa_clicked() {
    QMessageBox box;

    if(player->isAudioAvailable() == false) { //In caso il player non stia riproducendo tracce..
        box.setWindowTitle("Errore");
        box.setText("Non c'è nessuna canzone in riproduzione");
        box.exec();
    }

    else player->pause();  //In caso contrario funge da semplice tasto Pause
}

void MainWindow::on_btnStop_clicked() {
    QMessageBox box;

    if(player->isAudioAvailable() == false) {  //In caso il player non stia riproducendo tracce..
        box.setWindowTitle("Errore");
        box.setText("Non c'è nessuna canzone in riproduzione");
        box.exec(); //Messaggio di errore
    }

    else {
        if(setCoda == true) {
            player->stop();             //Negli altri casi (caso della coda) stoppo la riproduzione, do valore nullptr a setMedia per segnalare che non è disponibile
            player->setMedia(nullptr);  //nessun file audio e pulisco la coda di riproduzione
            coda->clear();

            path = "";
            ui->timeLabel->setText("0:00");

            ui->listaTitolo->clear();
        }

        else {                          //In caso Playlist, rifaccio gli altri passaggi ma non pulisco la coda
            player->stop();
            player->setMedia(nullptr);
            ui->timeLabel->setText("0:00");
        }

        //In entrambi i casi disconnetto tutti gli slot di segnale
        disconnect(ui->volumeSlider, SIGNAL(valueChanged(int)), player, SLOT(setVolume(int)));     //Disconnetto i vari slot di segnale tra stream audio e slider/progress bar
        disconnect(player, &QMediaPlayer::positionChanged, this, &MainWindow::on_positionChanged);
        disconnect(player, &QMediaPlayer::durationChanged, this, &MainWindow::on_durationChanged);
        disconnect(player, &QMediaPlayer::mediaStatusChanged, this, &MainWindow::updateCoda);
    }
}

void MainWindow::on_btnVolume_clicked() { //Procedura che serve per gestire la visibilità/non visibilità dello sliderVolume
    if(slider == true) ui->volumeSlider->setVisible(false);
    else ui->volumeSlider->setVisible(true);

    slider = !slider;
}

void MainWindow::on_btnAvanti_clicked() {
    if(setCoda == true) {      //Se premo sul pulsante avanti mentre sono sulla coda, scorro la coda
        coda->setCurrentIndex(coda->currentIndex() - 1);
        player->play();
    }

    else {                     //Se lo premo mentre sono su una playlist, scorro la playlist
        playlist->setCurrentIndex(playlist->currentIndex() - 1);
        player->play();
    }
}

void MainWindow::on_btnIndietro_clicked() {  //Stessa cosa per il tasto indietro
    if(setCoda == true) {
        coda->setCurrentIndex(coda->currentIndex() + 1);
        player->play();
    }

    else {
        playlist->setCurrentIndex(playlist->currentIndex() + 1);
        player->play();
    }
}

void MainWindow::on_progressSlider_sliderMoved(int position) { //Procedura che emette come segnale in output il valore position
    if(player->isAudioAvailable() == false) {}                 //dato dallo scorrimento dello slider al player per modificare il volume in tempo reale
    else player->setPosition(position);
}

/*-------------------------------GESTIONE TEMPO DI RIPRODUZIONE/DURATA TOTALE------------------------------*/

void MainWindow::on_positionChanged(qint64 position) {  //Procedura per la progressione della label del tempo di riproduzione.
    double sec;                                         //Riceve in input come segnale il cambiamento di posizione nella riproduzione
                                                        //grazie allo slider.
    ui->progressSlider->setValue(position);

    sec = static_cast<double>(position) / 1000;  //Faccio lo static cast al valore position (qint64) in double.
                                                 //Scelgo il double perchè è il tipo che più si avvicina alla grandezza
    sec++;                                       //di qint64, quindi ho una perdita minima di valori. Divido poi per mille
                                                 //perchè QMediaPlayer::duration() restituisce un valore in millisecondi.

    if (int (sec) - (min * 60) == 60) { //In caso si arrivi al minuto...
        min++;
        ui->timeLabel->setText(QString::number(static_cast<int>(min)) + ":" + QString::number(static_cast<int>(sec) - (min * 60)));
    }

    else {  //Mentre sotto al minuto...
        if(int (sec) - (min * 60) < 0){
            min--;
            ui->timeLabel->setText(QString::number(static_cast<int>(min)) + ":" + QString::number(static_cast<int>(sec) - (min * 60)));
        }

        ui->timeLabel->setText(QString::number(static_cast<int>(min)) + ":" + QString::number(static_cast<int>(sec) - (min * 60)));
    }
}

void MainWindow::on_durationChanged(qint64 position) { //Procedura che mostra la durata totale della traccia in riproduzione.
    double min = 0, sec = 0;                           //Riceve in input il segnale di cambiamento di traccia

    ui->progressSlider->setMaximum(position);

    sec = static_cast<double>(position) / 1000;        //Static cast e divisione per mille per lo stesso motivo di prima

        if(int(sec) % 60 == 0) {  //In caso il numero sia divisibile interamente per 60...
            min = sec / 60;
            ui->durationLabel->setText(QString::number(min) + ":00");
        }

        else {                    //In caso ci siano dei secondi in eccesso...
            min = sec / 60;
            sec = (int(sec) % 60) + 1;
            ui->durationLabel->setText(QString::number(static_cast<int>(min)) + ":" + QString::number(sec));
        }
}

/*-------------------------------GESTIONE CODA/PLAYLIST (file di testo)------------------------------*/

void MainWindow::on_btnCreaplaylist_clicked() {
    nRequest->setModal(true);   //Imposto la finestra parente non cliccabile finchè la nuova finestra non finisce il suo corso
    nRequest->show();
}

void MainWindow::updateName(QString name) {
    ui->listaPlaylist->addItem(name);   //Aggiungo la playlist alla lista

    QDir dir(PROJECT_PATH + name);      //PROJECT_PATH costante che contiene la path del progetto (guardare .pro)
    if (!dir.exists()) dir.mkpath("."); //Se la cartella non esiste, la creo (. si riferisce alla path attualmente puntata da dir)

    QString plDir = PROJECT_PATH + name; //Concateno le stringhe per formare l'indirizzo assoluto della cartella
    QFile filePlaylist(playlistFile);
    filePlaylist.open(QIODevice::Append);

    QTextStream stream(&filePlaylist);
    stream << plDir << endl;            //Scrivo l'indirizzo della cartella nel file Playlist.txt

    filePlaylist.close();

    QString mod = "/" + name + ".txt";  //Per costruire la path assoluta per il file di testo aggiungo il nome del file più lo slash ed il .txt

    QString playlistName = PROJECT_PATH + name + mod;  //Concateno le stringhe per formare l'indirizzo assoluto del file .txt della playlist
    QFile textPlaylist(playlistName);
    textPlaylist.open(QIODevice::WriteOnly);
    textPlaylist.close();
}

void MainWindow::on_btnElimina_clicked() {
    QString name;

    if(ui->listaPlaylist->currentRow() == -1 || ui->listaPlaylist->count() == 0) {  //In caso non abbia selezionato nessuna playlist
        QMessageBox box;
        box.setWindowTitle("Errore");
        box.setText("Non hai selezionato una playlist o non ne hai create");
        box.exec();
    }

    else {
        QMessageBox box;
        box.setWindowTitle("Avvertenza");
        box.setText("Sei sicuro di voler cancellare la playlist?");
        box.addButton("Elimina", QMessageBox::AcceptRole);
        box.addButton(QMessageBox::No);
        int ret = box.exec();

        if(ret == QMessageBox::AcceptRole) {
            name = ui->listaPlaylist->item(ui->listaPlaylist->currentRow())->text();
            QListWidgetItem* item = ui->listaPlaylist->item(ui->listaPlaylist->currentRow());
            delete item;  //Punto all'item selezionato con un puntatore di tipo QListWidgetItem e svuoto la locazione di memoria associata.

            QDir dir(PROJECT_PATH + name);
            dir.removeRecursively();

            QFile filePlaylist(playlistFile);
            filePlaylist.open(QIODevice::ReadWrite);  //Apro il file (di default un file di testo) in sola lettura per leggere le playlist presenti
            QTextStream stream(&filePlaylist);        //Dichiaro uno stream collegato al file

            QString tmpstr;

            while (!stream.atEnd()) {
                QString line = stream.readLine();               //Leggo riga per riga dal file finchè non arrivo alla fine
                if(line != PROJECT_PATH + name) tmpstr += line; //In caso la riga sia diversa da quella da cancellare la salvo
            }                                                   //in una var temporanea

            filePlaylist.resize(0); //Svuoto il file e passo la nuova stringa allo stream
            stream << tmpstr;

            filePlaylist.close();

            QMessageBox ssc;
            ssc.setWindowTitle("Successo");
            ssc.setText("Playlist eliminata con successo");
            ssc.exec();

            ui->songName->clear();
        }
    }
}


void MainWindow::on_btnAggiungip_clicked() {
    sPlaylist->setModal(true);
    sPlaylist->show();
}

void MainWindow::insertSong(QString name) {
    if(path != "") {
        QString playlistPath, samestr = name;
        playlistPath = PROJECT_PATH + samestr;

        QFile songFile(path);
        QFileInfo songInfo(songFile.fileName());
        QString songName(songInfo.fileName());

        QString playlistText = playlistPath + "/" + samestr + ".txt";

        QMessageBox box;
        QFile songText(playlistText);
        bool ctr = false;

        if(songText.open(QIODevice::ReadOnly)) {
            QTextStream stream(&songText);
            while (!stream.atEnd()) {
                 QString line = stream.readLine();
                 if(playlistText + "/" + songName == line) ctr = true;
            }
            songText.close();
        }

        if(ctr == true) {
            box.setWindowTitle("Errore");
            box.setText("Canzone già presente nella playlist");
            box.exec();
        }

        else {
            playlistPath = playlistPath + "/" + songName;

            QFile songPath(playlistText);
            songPath.open(QIODevice::Append);
            QTextStream stream(&songPath);
            stream << playlistPath << endl;
            songPath.close();

            songFile.rename(playlistPath);

            box.setWindowTitle("Aggiunta correttamente");
            box.setText("Canzone aggiunta correttamente alla playlist");
            box.exec();
        }
    }
}

void MainWindow::on_listaPlaylist_itemClicked(QListWidgetItem *item) {
    playlist->clear();
    ui->btnElimina->setVisible(true);
    setCoda = false;

    ui->songName->setText(item->text());
    ui->listaTitolo->clear();

    QString playlistxtPath, samestr = item->text();
    playlistxtPath = PROJECT_PATH + samestr + "/" + samestr + ".txt";

    QFile songPaths(playlistxtPath);

    if (songPaths.open(QIODevice::ReadOnly)) {

        QTextStream stream(&songPaths);
        while (!stream.atEnd()) {
            QString line = stream.readLine();
            playlist->addMedia(QUrl::fromLocalFile(line));

            QFile songLine(line);
            QFileInfo songInfo(songLine.fileName());
            QString songName(songInfo.completeBaseName());

            ui->listaTitolo->addItem(songName);
        }
        songPaths.close();
    }

    playlist->setCurrentIndex(playlist->currentIndex() + 1);
    player->setPlaylist(playlist);
    player->setVolume(ui->volumeSlider->value());
    player->play();

    if(!player->isAudioAvailable()) {
        connect(ui->volumeSlider, SIGNAL(valueChanged(int)), player, SLOT(setVolume(int)));
        connect(player, &QMediaPlayer::durationChanged, this, &MainWindow::on_durationChanged);
        connect(player, &QMediaPlayer::positionChanged, this, &MainWindow::on_positionChanged);
        connect(player, &QMediaPlayer::mediaStatusChanged, this, &MainWindow::updateCoda);
    }
}


/*----------------AZIONI--------------------*/

//Imposto tutte le azioni della Menu Bar collegandole alle funzioni già create

void MainWindow::on_actionApri_un_file_triggered() {
    MainWindow::on_btnRiproduci_clicked();
}

void MainWindow::on_actionStart_triggered() {
    MainWindow::on_btnRiproduci_clicked();
}

void MainWindow::on_actionStop_triggered() {
    MainWindow::on_btnStop_clicked();
}

void MainWindow::on_actionPausa_triggered() {
    MainWindow::on_btnPausa_clicked();
}

void MainWindow::on_actionTorna_alla_coda_triggered() {
    ui->songName->setText("CODA DI RIPRODUZIONE");
    if(player->isAudioAvailable()) {
        player->stop();
        player->setMedia(nullptr);
    }

    setCoda = true;
    ui->listaTitolo->clear();
    ui->btnElimina->setVisible(false);
}

void MainWindow::on_actionAvanti_di_uno_triggered() {
    MainWindow::on_btnAvanti_clicked();
}

void MainWindow::on_actionIndietro_di_uno_triggered() {
    MainWindow::on_btnIndietro_clicked();
}

void MainWindow::on_actionCrea_playlist_triggered() {
    MainWindow::on_btnCreaplaylist_clicked();
}

void MainWindow::on_actionAggiungi_alla_playlist_triggered() {
    MainWindow::on_btnAggiungip_clicked();
}
