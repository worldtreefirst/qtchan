#include "filter.h"
#include "you.h"
#include <QStandardPaths>
#include <QDebug>

Filter::Filter()
{
	filters = loadFilterFile();
}

/*void Filter::htmlParse(QString search) {

}*/

QSet<QString> Filter::findQuotes(QString post)
{
	QRegularExpression quotelink;
	quotelink.setPattern("href=\\\"#p(\\d+)\\\"");
	QRegularExpressionMatch quotelinkMatch;
	QRegularExpressionMatchIterator quotelinkMatches;
	quotelinkMatches = quotelink.globalMatch(post);
	QSet<QString> quotes;
	while (quotelinkMatches.hasNext()) {
		quotelinkMatch = quotelinkMatches.next();
		quotes.insert(QString(quotelinkMatch.captured(1)));
	}
	return quotes;
}

//TODO allow change filter file location setting
//TODO listen for file changes and reload filter
QSet<QRegularExpression> Filter::loadFilterFile(){
	QSet<QRegularExpression> set;
	QString filterFile = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/qtchan/" + "filters.conf";
	QFile inputFile(filterFile);
	if (inputFile.open(QIODevice::ReadOnly))
	{
		QTextStream in(&inputFile);
		while (!in.atEnd())
		{
			QString line = in.readLine();
			//replace \ with \\ for c++ regexp
			if(line.at(0)=='#') continue;
			line = line.replace("\\\\","\\\\\\\\");
			set.insert(QRegularExpression(line));
		}
	}
	return set;
}

bool Filter::filterMatched(QString post){
	QSetIterator<QRegularExpression> i(filters);
	while (i.hasNext()){
		QRegularExpression temp = i.next();
		if(temp.match(post).hasMatch()){
			return true;
		}
	}
	return false;
}

/*
QSet<QString> Filter::crossthread(QString search) {
	QRegularExpression quotelink;
	quotelink.setPattern("href=\\\"#p(\\d+)\\\"");
	QRegularExpressionMatch quotelinkMatch;
	QRegularExpressionMatchIterator quotelinkMatches;
	quotelinkMatches = quotelink.globalMatch(post);
	QSet<QString> quotes;
	while (quotelinkMatches.hasNext()) {
		quotelinkMatch = quotelinkMatches.next();
		quotes.insert(QString(quotelinkMatch.captured(1)));
	}
	return quotes;
}*/

QRegularExpression Filter::quoteRegExp("class=\"quote\"");
QRegularExpression Filter::quotelinkRegExp("class=\"quotelink\"");
QString Filter::colorString("class=\"quote\" style=\"color:#8ba446\"");
QString Filter::quoteString("class=\"quote\" style=\"color:#897399\"");
/*{
	QSettings settings;
	QString colorValue(settings.value("colorString"));
	return colorValue.isEmpty() ? "class=\"quote\" style=\"color:#8ba446\"" : colorValue;
}*/


QString Filter::replaceQuoteStrings(QString &string){
	//QSettings settings(QSettings::IniFormat,QSettings::UserScope,"qtchan","qtchan");
	//QColor color = settings.value("quote_color",);
	string.replace(quoteRegExp,colorString);
	string.replace(quotelinkRegExp,quoteString);
	return string;
}

QString Filter::replaceYouStrings(QRegularExpressionMatchIterator i, QString &string){
	QList<QRegularExpressionMatch> matches;
	while (i.hasNext()) {
		matches.append(i.next());
	}
	for(int i=matches.size()-1; i>=0; i--){
		QRegularExpressionMatch match = matches.at(i);
		if(match.capturedEnd()){
			string.insert(match.capturedEnd()," (You)");
		}
	}
	return string;
}

QString Filter::htmlParse(QString &html){
	return html.replace("<br>","\n").replace("&amp;","&")
		.replace("&gt;",">").replace("&lt;","<")
		.replace("&quot;","\"").replace("&#039;","'")
		.replace("<wb>","\n").replace("<wbr>","\n");
}

QString Filter::titleParse(QString &title){
	QRegularExpression htmlTag;
	htmlTag.setPattern("</?span.*?>");
	return title.replace(htmlTag,"").replace("<br>"," ").replace("&amp;","&")
		.replace("&gt;",">").replace("&lt;","<")
		.replace("&quot;","\"").replace("&#039;","'")
		.replace("<wb>"," ").replace("<wbr>"," ");
}

QString Filter::toStrippedHtml(QString &text){
	QRegularExpression imgTag("<img.*?>");
	QRegularExpression htmlTag("<[^>]*>");
	return text.replace(QRegularExpression("<img.*?>")," ").replace(htmlTag,"").replace("&amp;","&")
		.replace("&gt;",">").replace("&lt;","<")
		.replace("&quot;","\"").replace("&#039;","'");
}
