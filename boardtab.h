#ifndef BOARDTAB_H
#define BOARDTAB_H

#include "boardtabhelper.h"
#include "chans.h"
#include "threadform.h"
#include "postform.h"
#include "filter.h"
#include <QSpacerItem>
#include <QWidget>
#include <QThread>

namespace Ui {
class BoardTab;
}

class BoardTab : public QWidget
{
	Q_OBJECT
	Qt::ConnectionType UniqueDirect = static_cast<Qt::ConnectionType>(Qt::DirectConnection | Qt::UniqueConnection);
	//QSpacerItem space = QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding);
public:
	explicit BoardTab(Chan *api, QString board, BoardType type = Index, QString search = "", QWidget *parent = 0);
	~BoardTab();
	Chan *api;
	QString board;
	BoardType type;
	QString search;
	QString tabType;
	QThread workerThread;
	BoardTabHelper helper;
	QString boardUrl;
	QNetworkReply *reply;
	QMap<QString,ThreadForm*> tfMap;
	QList<QPair<QString,ThreadForm*>> tfPairs;
	PostForm myPostForm;
	void openPostForm();

	void setShortcuts();
	void getPosts();
	void focusIt();
	ThreadForm* tfAtTop();

public slots:
	void findText(const QString text);
	void onNewThread(ThreadForm *tf);
	void onNewTF(ThreadForm *tf, ThreadForm *thread);
	//void addStretch();
	void clearMap();
	void setFontSize(int fontSize);
	void setImageSize(int imageSize);

private:
	Ui::BoardTab *ui;
	Filter filter;
	QString vimCommand;

private slots:
	void on_pushButton_clicked();
	void on_lineEdit_returnPressed();
	void updateVim();
};

Q_DECLARE_METATYPE(BoardTab*)

#endif // BOARDTAB_H
