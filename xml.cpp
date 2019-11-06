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
	QByteArrayList res;
	auto           nodes = getNodes(path);
	if (nodes == nullptr) {
		return res;
	}
	for (int var = 0; var < nodes->nodeNr; ++var) {
		auto node = nodes->nodeTab[var];
		auto vv   = xmlNodeGetContent(node);
		if (vv != nullptr) {
			QByteArray v = QByteArray::fromRawData((const char*)vv, strlen((const char*)vv));
			res.append(v);
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

std::vector<std::vector<const char*>> XPath::getLeafs(std::vector<const char*> xPaths, xmlNodeSetPtr nodes) {
	std::vector<std::vector<const char*>> res;
	res.resize(nodes->nodeNr);
	if (nodes == nullptr) {
		return res;
	}
	for (uint nodePos = 0; nodePos < nodes->nodeNr; ++nodePos) {
		auto node = nodes->nodeTab[nodePos];
		for (uint pathPos = 0; pathPos < xPaths.size(); ++pathPos) {
			auto                      path     = xPaths.at(pathPos);
			std::vector<const char*>& cur      = res.at(nodePos);
			xmlXPathObjectPtr         xpathObj = xmlXPathNodeEval(node, (const xmlChar*)path, xpath_ctx);
			if (xpathObj == nullptr) {
				return res;
			}
			auto nodes = xpathObj->nodesetval;
			for (int var = 0; var < nodes->nodeNr; ++var) {
				auto node = nodes->nodeTab[var];
				auto vv   = xmlNodeGetContent(node);
				if (vv != nullptr) {
					cur.push_back((const char*)vv);
				}
			}
		}
	}
	return res;
}

xmlNodeSetPtr XPath::getNodes(const char* path) {
	xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression((const xmlChar*)path, xpath_ctx);
	if (xpathObj == nullptr) {
		return nullptr;
	}
	auto nodes = xpathObj->nodesetval;
	return nodes;
}
