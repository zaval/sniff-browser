#include "browserwindow.h"
#include "ui_browserwindow.h"

BrowserWindow::BrowserWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::BrowserWindow),
    nam(new InterceptorAccessManager())
{
    ui->setupUi(this);
    ui->webView_main->page()->setNetworkAccessManager(nam);
    connect(nam, SIGNAL(pendingReplyProxy(NetworkReplyProxy*)), this, SLOT(process_reply(NetworkReplyProxy*)));
    connect(nam, SIGNAL(requestCatched(QNetworkRequest,QByteArray)), this, SLOT(handle_request(QNetworkRequest,QByteArray)));
    ui->tableWidget_history->setColumnCount(3);
//    urlCompleter = new QCompleter(urlHistory);

//    ui->lineEdit_url->setCompleter(urlCompleter);

}

BrowserWindow::~BrowserWindow()
{
    disconnect(nam, SIGNAL(pendingReplyProxy(NetworkReplyProxy*)), this, SLOT(process_reply(NetworkReplyProxy*)));
    delete nam;
    delete ui;
}

void BrowserWindow::on_toolButton_go_clicked()
{
    QString url = ui->lineEdit_url->text();
    if (!url.startsWith("http://") || !url.startsWith("https://")){
        url = "http://" + url;
        ui->lineEdit_url->setText(url);
    }
    ui->webView_main->load(QUrl(url));
}

void BrowserWindow::on_lineEdit_url_returnPressed()
{
    on_toolButton_go_clicked();
}

void BrowserWindow::on_webView_main_loadFinished(bool arg1)
{
    Q_UNUSED(arg1);
    ui->lineEdit_url->setText(ui->webView_main->page()->mainFrame()->url().toString());
    ui->toolButton_undo->setEnabled(ui->webView_main->page()->history()->canGoBack());
    ui->toolButton_redo->setEnabled(ui->webView_main->page()->history()->canGoForward());


//    urlCompleter->deleteLater();
//    urlCompleter = new QCompleter(urlHistory);
//    ui->lineEdit_url->setCompleter(urlCompleter);

}

void BrowserWindow::on_toolButton_undo_clicked()
{
    ui->webView_main->page()->history()->back();
}

void BrowserWindow::on_toolButton_redo_clicked()
{
    ui->webView_main->page()->history()->forward();
}

void BrowserWindow::process_reply(NetworkReplyProxy *proxy)
{
    connect(proxy, SIGNAL(resourceIntercepted(QUrl,QByteArray,QByteArray)), this, SLOT(handle_reply(QUrl,QByteArray,QByteArray)));
}

void BrowserWindow::handle_reply(const QUrl &url, const QByteArray &data, const QByteArray &headers)
{
//    qDebug() << url << data << headers;
    QTableWidgetItem *itemUrl = new QTableWidgetItem(url.toString());
    QTableWidgetItem *itemData = new QTableWidgetItem(QString(data));
    QTableWidgetItem *itemHeaders = new QTableWidgetItem(QString(headers));
    ui->tableWidget_history->setRowCount(ui->tableWidget_history->rowCount()+1);
    ui->tableWidget_history->setItem(ui->tableWidget_history->rowCount()-1, 0, itemUrl);
    ui->tableWidget_history->setItem(ui->tableWidget_history->rowCount()-1, 1, itemData);
    ui->tableWidget_history->setItem(ui->tableWidget_history->rowCount()-1, 2, itemHeaders);
    RequestDetails rd = waitingRequestHistory.value(url.toString(), RequestDetails());
    rd.response.headers = headers;
    rd.response.body = data;
    waitingRequestHistory.remove(url.toString());
    requestHistory << rd;
}

void BrowserWindow::handle_request(const QNetworkRequest &request, const QByteArray &data)
{
    QString url = request.url().toString();
    RequestDetails rd;
    rd.url = url;
    rd.ts = QDateTime::currentDateTime().toTime_t();
    rd.request.body = data;
    rd.request.method = "GET";
    QList<QByteArray> headers = request.rawHeaderList();
    for (int i = 0; i < headers.count(); i++){
        rd.request.headers += headers[i] + ": " + request.rawHeader(headers[i]) + "\n";
        if (headers[i].toLower() == "content-length")
            rd.request.method = "POST";
    }


    waitingRequestHistory.insert(url, rd);
}

void BrowserWindow::on_actionSave_triggered()
{
    QJsonArray requests;
    for (int i = 0; i < requestHistory.count(); i++){
        QJsonObject obj;
//        {
//            {"time", requestHistory[i].ts},
//            {"url", requestHistory[i].url},
//            {
//                "request", {
//                    {"headers", requestHistory[i].request.headers},
//                    {"body", requestHistory[i].request.body},
//                },
//                "response", {
//                    {"headers", requestHistory[i].response.headers},
//                    {"body", requestHistory[i].response.body},
//                }
//            }
//        };
        obj.insert("time", QJsonValue( (qint64) requestHistory[i].ts));
        obj.insert("url", QJsonValue(requestHistory[i].url));

        QJsonObject requestObj;
        requestObj.insert("headers", QJsonValue(requestHistory[i].request.headers.toStdString().c_str()));
        requestObj.insert("body", QJsonValue(requestHistory[i].request.body.toStdString().c_str()));
        requestObj.insert("method", QJsonValue(requestHistory[i].request.method));

        QJsonObject responseObj;
        responseObj.insert("headers", QJsonValue(requestHistory[i].response.headers.toStdString().c_str()));
        responseObj.insert("body", QJsonValue(requestHistory[i].response.body.toStdString().c_str()));

        obj.insert("request", requestObj);
        obj.insert("response", responseObj);

        requests.append(obj);

    }

    QString fname = QFileDialog::getSaveFileName(this, tr("save file"), ".", tr("Json (*.json)"));
    QJsonDocument doc;
    doc.setArray(requests);
    QFile f(fname);
    f.open(QIODevice::WriteOnly);
    if (f.isOpen()){
        f.write(doc.toJson());
        f.close();
    }
}
