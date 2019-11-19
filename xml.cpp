#include "xml.h"
#include <QDebug>
#include <libxml/xpathInternals.h>
#include "define.h"

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

XPath::~XPath() {
	xmlXPathFreeContext(xpath_ctx);
	xmlFreeDoc(doc);
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
	for (auto&& node : nodes) {

		auto vv = node.getContent();
		if (vv != nullptr) {
			QByteArray v;
			v.append((const char*)vv, strlen((const char*)vv));
			res.append(v);
		}
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

std::vector<XmlNode> XPath::getNodes(const char* path, xmlNodePtr node, uint limit) {
	std::vector<XmlNode> nodeVec;
	xmlXPathObjectPtr    xpathObj;
	if (node) {
		xpathObj = xmlXPathNodeEval(node, (const xmlChar*)path, xpath_ctx);
	} else {
		xpathObj = xmlXPathEvalExpression((const xmlChar*)path, xpath_ctx);
	}

	if (xpathObj == nullptr) { //in case of invalid xpath
		xmlXPathFreeObject(xpathObj);
		return nodeVec;
	} else {
		auto nodes = xpathObj->nodesetval;
		if (nodes->nodeNr == 0) {
			return nodeVec;
		}
		for (uint var = 0; var < nodes->nodeNr; ++var) {
			if (var >= limit) {
				break;
			}
			auto    node = nodes->nodeTab[var];
			XmlNode nod{this, node};
			nodeVec.push_back(nod);
		}
	}
	xmlXPathFreeObject(xpathObj);
	return nodeVec;
}

XmlNode XmlNode::searchNode(const char* path) {
	auto nodes = xml->getNodes(path, node, 1);
	if (nodes.empty()) {
		return XmlNode();
	}
	return nodes.at(0);
}

std::vector<XmlNode> XmlNode::searchNodes(const char* path) {
	return xml->getNodes(path, node);
}

QByteArray XmlNode::searchLeaf(const char* path) {
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

QByteArray XmlNode::getProp(const char* property) {
	QByteArray q;
	auto       vv = xmlGetProp(node, (const xmlChar*)property);
	q.append((const char*)vv, strlen((const char*)vv));
	xmlFree(vv);
	return q;
}

QByteArray XmlNode::getContent() {
	auto       vv = xmlNodeGetContent(node);
	QByteArray res;
	if (vv != nullptr) {
		res.append((const char*)vv, strlen((const char*)vv));
		xmlFree(vv);
	}
	return res;
}

XmlNode XmlNode::operator[](const char *path) {
	//this is a start from here XPath! what you most expect from a [] ...
	QByteArray pathFull = QBL("./*[name()='") + path + QBL("']");
	return searchNode(pathFull.constData());
}


