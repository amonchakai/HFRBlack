/*
 * ExploreCategoryController.cpp
 *
 *  Created on: 27 mars 2014
 *      Author: PierreL
 */

#include "ExploreCategoryController.hpp"

#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QRegExp>
#include <QDateTime>

#include <bb/cascades/AbstractPane>
#include <bb/cascades/GroupDataModel>

#include  "Globals.h"
#include  "HFRNetworkAccessManager.hpp"
#include  "DataObjects.h"


ExploreCategoryController::ExploreCategoryController(QObject *parent)
	: QObject(parent), m_ListView(NULL), m_Datas(new QList<ThreadListItem*>()) {

}



void ExploreCategoryController::listTopics(const QString &url_string) {

	// list green + yellow flags
	const QUrl url(url_string);

	QNetworkRequest request(url);
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");


	QNetworkReply* reply = HFRNetworkAccessManager::get()->get(request);
	bool ok = connect(reply, SIGNAL(finished()), this, SLOT(checkReply()));
	Q_ASSERT(ok);
	Q_UNUSED(ok);

}



void ExploreCategoryController::checkReply() {
	QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());

	QString response;
	if (reply) {
		if (reply->error() == QNetworkReply::NoError) {
			const int available = reply->bytesAvailable();
			if (available > 0) {
				const QByteArray buffer(reply->readAll());
				response = QString::fromUtf8(buffer);
				parse(response);
			}
		} else {
			response = tr("Error: %1 status: %2").arg(reply->errorString(), reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString());
			qDebug() << response;
		}

		reply->deleteLater();
	}
}



void ExploreCategoryController::parse(const QString &page) {
	qDebug() << "start parsing";

	m_Datas->clear();

	QRegExp andAmp("&amp;");
	QRegExp quote("&#034;");
	QRegExp euro("&euro;");
	QRegExp inf("&lt;");
	QRegExp sup("&gt;");


	// ----------------------------------------------------------------------------------------------
	// Parse categories using regexp

	// Get favorite topics
	QRegExp regexp(QString("<a href=\".*\" class=\"cCatTopic\" title=\"Sujet n.[0-9]+\">(.+)</a></td>"));  	// topics' name


	regexp.setCaseSensitivity(Qt::CaseSensitive);
	regexp.setMinimal(true);


	QString today = QDateTime::currentDateTime().toString("dd-MM-yyyy");

	int pos = 0;
	int lastPos = regexp.indexIn(page, pos);
	QString caption;

	if(lastPos != -1) {
		caption = regexp.cap(1);
		caption.replace(andAmp,"&");
		caption.replace(quote,"\"");
		caption.replace(euro, "e");
		caption.replace(inf, "<");
		caption.replace(sup, ">");
	}

	while((pos = regexp.indexIn(page, lastPos)) != -1) {
		pos += regexp.matchedLength();

		// parse each post individually
		parseThreadListing(caption, page.mid(lastPos, pos-lastPos));


		lastPos = pos;
		caption = regexp.cap(1);
		caption.replace(andAmp,"&");
		caption.replace(quote,"\"");
		caption.replace(euro, "e");
		caption.replace(inf, "<");
		caption.replace(sup, ">");
	}
	parseThreadListing(caption, page.mid(lastPos, pos-lastPos));


	qDebug() << "end parsing";

	updateView();
	emit complete();

}


void ExploreCategoryController::parseThreadListing(const QString &caption, const QString &threadListing) {
	ThreadListItem *item = new ThreadListItem();
	QRegExp andAmp("&amp;");
	QRegExp nbsp("&nbsp;");

	item->setTitle(caption);

	QRegExp firstPostUrlRegexp("<td class=\"sujetCase4\">.*<a href=\"(.+)\" class=\"cCatTopic\">([0-9]+)</a></td>");
	firstPostUrlRegexp.setCaseSensitivity(Qt::CaseSensitive);
	firstPostUrlRegexp.setMinimal(true);

	if(firstPostUrlRegexp.indexIn(threadListing, 0) != -1) {
		QString s = firstPostUrlRegexp.cap(1);
		s.replace(andAmp, "&");
		item->setUrlFirstPage(s);

		item->setPages(firstPostUrlRegexp.cap(2));
	} else {
		item->setPages("1");
	}

	QRegExp lastReadPost("<td class=\"sujetCase9 cBackCouleurTab[0-9] \"><a href=\"(.*)\" class=\"Tableau\">(.*)<br /><b>(.*)</b>");
	lastReadPost.setCaseSensitivity(Qt::CaseSensitive);
	lastReadPost.setMinimal(true);

	if(lastReadPost.indexIn(threadListing, 0) != -1) {
		QString s = lastReadPost.cap(2);
		s.replace(nbsp, " ");
		item->setTimestamp(s);

		s = lastReadPost.cap(1);
		s.replace(andAmp, "&");
		item->setUrlFirstPage(s);

		item->setLastAuthor(lastReadPost.cap(3));
	}

	QRegExp flagTypeRegexp("<img src=\"http://forum-images.hardware.fr/themes_static/images_forum/1/favoris.gif\"");
	if(flagTypeRegexp.indexIn(threadListing, 0) != -1)
		item->setFlagType(Flag::FAVORITE);

	flagTypeRegexp = QRegExp("<img src=\"http://forum-images.hardware.fr/themes_static/images_forum/1/flag([0-1]).gif\"");
	if(flagTypeRegexp.indexIn(threadListing, 0) != -1) {
		switch(flagTypeRegexp.cap(1).toInt()) {
			case 0:
				item->setFlagType(Flag::READ);
				break;

			case 1:
				item->setFlagType(Flag::PARTICIPATE);
				break;
		}
	}


	if(!item->getLastAuthor().isEmpty())
		m_Datas->append(item);
}


void ExploreCategoryController::updateView() {

	// ----------------------------------------------------------------------------------------------
	// get the dataModel of the listview if not already available

	if(m_ListView == NULL) {
		qWarning() << "the list view is either not provided or not a listview...";
		return;
	}

	using namespace bb::cascades;

	GroupDataModel* dataModel = dynamic_cast<GroupDataModel*>(m_ListView->dataModel());
	if (dataModel) {
		dataModel->clear();
	} else {
		dataModel = new GroupDataModel(
						QStringList() << "title"
									  << "timestamp"
									  << "lastAuthor"
									  << "urlFirstPage"
									  << "urlLastPage"
									  << "pages"
									  << "flagType"
					);
		m_ListView->setDataModel(dataModel);
	}
	dataModel->setGrouping(ItemGrouping::ByFullValue);

	// ----------------------------------------------------------------------------------------------
	// push data to the view

	QList<QObject*> datas;
	for(int i = m_Datas->length()-1 ; i >= 0 ; --i) {
		datas.push_back(m_Datas->at(i));
	}

	dataModel->clear();
	dataModel->insertList(datas);

}



