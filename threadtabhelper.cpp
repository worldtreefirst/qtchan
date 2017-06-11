#include "threadtabhelper.h"
#include "netcontroller.h"

ThreadTabHelper::ThreadTabHelper(){

}

void ThreadTabHelper::startUp(QString &board, QString &thread, QWidget* parent){
    this->board = board;
    this->thread = thread;
    this->parent = parent;
    this->threadUrl = "https://a.4cdn.org/"+board+"/thread/"+thread+".json";
    QSettings settings;
    this->expandAll = settings.value("autoExpand",false).toBool();
    QDir().mkpath(board+"/"+thread+"/thumbs");
    qDebug() << threadUrl;
    request = QNetworkRequest(QUrl(threadUrl));
    request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
    getPosts();
    updateTimer = new QTimer();
    updateTimer->setInterval(60000);
    updateTimer->start();
    if(settings.value("autoUpdate").toBool()){
        connectionUpdate = connect(updateTimer, &QTimer::timeout,
                                   this,&ThreadTabHelper::getPosts,UniqueDirect);
    }
}

ThreadTabHelper::~ThreadTabHelper(){
    updateTimer->stop();
    delete updateTimer;
    disconnect(connectionUpdate);
    disconnect(connectionPost);
    if(gettingReply){
        reply->abort();
        disconnect(reply);
        reply->deleteLater();
    }
    //delete updateTimer;
}

void ThreadTabHelper::setAutoUpdate(bool update){
    disconnect(connectionUpdate);
    if(update){
        connectionUpdate = connect(updateTimer, &QTimer::timeout,
                                   this,&ThreadTabHelper::getPosts,UniqueDirect);
    }
}

void ThreadTabHelper::getPosts(){
    qDebug() << "getting posts for" << threadUrl;
    reply = nc.jsonManager->get(request);
    gettingReply = true;
    connectionPost = connect(reply, &QNetworkReply::finished,
                             this,&ThreadTabHelper::loadPosts, UniqueDirect);
}

void ThreadTabHelper::writeJson(QString &board, QString &thread, QByteArray &rep){
    QFile jsonFile(board+"/"+thread+"/"+thread+".json");
    jsonFile.open(QIODevice::WriteOnly);
    QDataStream out(&jsonFile);
    out << rep;
    jsonFile.close();
}

void ThreadTabHelper::loadPosts(){
    gettingReply = false;
    if(reply->error()){
        qDebug().noquote() << "loading post error:" << reply->errorString();
        if(reply->error() == QNetworkReply::ContentNotFoundError){
            qDebug() << "Stopping timer for" << threadUrl;
            updateTimer->stop();
            emit thread404();
        }
        reply->deleteLater();
        return;
    }
    //write to file and make json array
    QByteArray rep = reply->readAll();
    //disconnect(connectionPost);
    QtConcurrent::run(&ThreadTabHelper::writeJson,board, thread, rep);
    posts = QJsonDocument::fromJson(rep).object().value("posts").toArray();
    int length = posts.size();
    qDebug().noquote() << QString("length of ").append(threadUrl).append(" is ").append(QString::number(length));
    int i = tfMap.size();
    QSettings settings;
    bool loadFile = settings.value("autoExpand",false).toBool() || this->expandAll;
    while(!abort && i<length){
        p = posts.at(i).toObject();
        ThreadForm *tf = new ThreadForm(board,thread,PostType::Reply,true,loadFile,parent);
        tf->load(p);
        tfMap.insert(tf->post.no,tf);
        emit newTF(tf);
        if(i==0){
            if(tf->post.sub.length())emit windowTitle("/"+board+"/"+thread + " - " + tf->post.sub);
            else if(tf->post.com.length()){
                QString temp = tf->post.com;
                emit windowTitle("/"+board+"/"+thread + " - " +
                     ThreadForm::htmlParse(temp
                        .replace(QRegExp("</?span( class=\"quote\" style=\"color:#[\\d|\\w]{6}\")?>"),""))
                        .replace("\n"," "));
            }
        }
        QPointer<ThreadForm> replyTo;
        foreach (const QString &orig, tf->quotelinks)
        {
            replyTo = tfMap.find(orig).value();
            if(replyTo != tfMap.end().value()){
                replyTo->replies.insert(tf->post.no.toDouble(),tf->post.no);
                replyTo->setReplies();
            }
        }
        i++;
        QCoreApplication::processEvents();   
    }
    if(!abort) emit addStretch();
    //emit scrollIt();
    reply->deleteLater();
    //reply->deleteLater();
}

void ThreadTabHelper::loadAllImages(){
    expandAll = !expandAll;
    qDebug() << "settings expandAll for" << threadUrl << "to" << expandAll;
    if(expandAll){
        QMapIterator<QString,ThreadForm*> mapI(tfMap);
        while (mapI.hasNext()) {
            mapI.next();
            mapI.value()->loadOrig();
        }
    }
}
