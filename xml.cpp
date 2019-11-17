#include "xml.h"
#include <QDebug>
#include <libxml/xpathInternals.h>

void XPath::read(const char* data, int size) {
	doc               = xmlParseMemory(data, size);
	xmlErrorPtr error = xmlGetLastError();
	if (error != nullptr) {
		qCritical() << "Failed to parse document \n " << error->message;
		return;
	}
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

QByteArray XPath::getLeaf(const char* path, xmlNodePtr node) {
	QByteArray        res;
	xmlXPathObjectPtr xpathObj = xmlXPathNodeEval(node, (const xmlChar*)path, xpath_ctx);
	if (xpathObj == nullptr) {
		return res;
	} else {
		auto nodes = xpathObj->nodesetval;
		if (nodes->nodeNr == 0) {
			return res;
		}
		auto node = nodes->nodeTab[0];
		auto vv   = xmlNodeGetContent(node);
		if (vv != nullptr) {
			res.setRawData((const char*)vv, strlen((const char*)vv));
		}
		//		for (int var = 0; var < nodes->nodeNr; ++var) {

		//		}
	}
	return res;
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
			QByteArray v;
			v.append((const char*)vv, strlen((const char*)vv));
			res.append(v);
			xmlFree(vv);
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

xmlNodePtr XmlNode::getNode(const char* path) {
	xmlXPathObjectPtr xpathObj = xmlXPathNodeEval(node, (const xmlChar*)path, xml->xpath_ctx);
	if (xpathObj == nullptr) {
		return nullptr;
	}
	auto nodes = xpathObj->nodesetval;
	if (nodes->nodeNr == 0) {
		return nullptr;
	}
	return nodes->nodeTab[0];
}

QByteArray XmlNode::getLeaf(const char* path) {
	QByteArray        res;
	xmlXPathObjectPtr xpathObj = xmlXPathNodeEval(node, (const xmlChar*)path, xml->xpath_ctx);
	if (xpathObj == nullptr) {
		return res;
	} else {
		auto nodes = xpathObj->nodesetval;
		if (nodes->nodeNr == 0) {
			return res;
		}
		auto node = nodes->nodeTab[0];
		auto vv   = xmlNodeGetContent(node);
		if (vv != nullptr) {
			res.setRawData((const char*)vv, strlen((const char*)vv));
		}
		//		for (int var = 0; var < nodes->nodeNr; ++var) {

		//		}
	}
	return res;
}

void XmlNode::swapLeaf(const char* path, double& val) {
	val = getLeaf(path).toDouble();
}

void XmlNode::swapLeaf(const char* path, quint64& val) {
	val = getLeaf(path).toULongLong();
}

void XmlNode::swapLeaf(const char* path, QString& val) {
	val = getLeaf(path);
}

void XmlNode::swapLeaf(const char* path, QByteArray& val) {
	val = getLeaf(path);
}

void XmlNode::swapLeafValue(const char* path, double& val) {
	auto node = getNode(path);
	if (node) {
		val = getValue(node).toDouble();
	}
}

void XmlNode::swapLeafValue(const char* path, quint64& val) {
	auto node = getNode(path);
	if (node) {
		val = getValue(node).toULongLong();
	}
}

void XmlNode::swapLeafValue(const char* path, QString& val) {
	auto node = getNode(path);
	if (node) {
		val = getValue(node);
	}
}

void XmlNode::swapLeafValue(const char* path, QByteArray& val) {
	auto node = getNode(path);
	if (node) {
		val = getValue(node);
	}
}

QByteArray XmlNode::swapLeafValue(const char* path) {
	auto node = getNode(path);
	if (node) {
		return getValue(node);
	}
	return QByteArray();
}

QByteArray XmlNode::getValue(const xmlNodePtr node) {
	QByteArray q;
	auto       vv = xmlGetProp(node, (const xmlChar*)"value");
	q.setRawData((const char*)vv, strlen((const char*)vv));
	return q;
}
