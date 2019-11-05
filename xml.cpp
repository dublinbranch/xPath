#include "xml.h"
#include <QDebug>
#include <libxml/xpathInternals.h>

void XPath::read(const char* data, int size) {
	doc       = xmlParseMemory(data, size);
	xpath_ctx = xmlXPathNewContext(doc);
}

void XPath::read(const QByteArray& data) {
	read(data.constData(), data.size());
}

XPath::XPath(const QByteArray& data) {
	read(data.constData(), data.size());
}

XPath::XPath(const QString& data) {
	read(data.toUtf8());
}

QByteArray XPath::getLeaf(const char* path, uint& founded) {
	auto list = getLeafs(path);
	if (list.isEmpty()) {
		founded = 0;
		qCritical() << "no result for " << path;
		return QByteArray();
	}
	founded = list.size();
	return list.at(0);
}

QByteArray XPath::getLeaf(const char* path) {
	uint num;
	return getLeaf(path, num);
}

QByteArrayList XPath::getLeafs(const char* path) {
	QByteArrayList    res;
	xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression((xmlChar*)path, xpath_ctx);
	if (xpathObj == nullptr) {
		return res;
	}
	auto nodes = xpathObj->nodesetval;

	if (nodes == nullptr) {
		return res;
	}
	for (int var = 0; var < nodes->nodeNr; ++var) {
		auto node = nodes->nodeTab[var];
		auto vv   = xmlNodeGetContent(node);
		if (vv != nullptr) {
			QByteArray v = QByteArray::fromRawData((const char*)vv, strlen((const char*)vv));
			res.append(QByteArray(v));
		}
		//printf("%s \n", vv);
		//		auto v1  = QString("//*[@id='field-table']/tbody/tr[%1]/th").arg(var).toUtf8();
		//		auto v   = v1.constData();
		//		auto ptr = xmlXPathEvalExpression((xmlChar*)v, xpath_ctx)->nodesetval;
		//		auto vv  = xmlNodeGetContent(ptr->nodeTab[0]);
		//		printf("%s \n", vv);
	}
	return res;
}
