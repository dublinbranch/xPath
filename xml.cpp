#include "xml.h"
#include "define/qsl.h"
#include <libxml/HTMLparser.h>
#include <libxml/xpathInternals.h>

/* only for debug, 
#include <QDebug>
void errMgr(void* ctx, const char* msg, ...) {
	(void)ctx;
	const uint TMP_BUF_SIZE = 1024;
	char       string[TMP_BUF_SIZE];
	va_list    arg_ptr;
	va_start(arg_ptr, msg);
	vsnprintf(string, TMP_BUF_SIZE, msg, arg_ptr);
	va_end(arg_ptr);
	qDebug() << string;
	return;
}
*/

XPath::Res XPath::read(const char* data, int size) {

	//xmlSetGenericErrorFunc(NULL, errMgr);
	/*
	 * HTML_PARSE_NONET because 99.999% of the doc stuff is either wrong or offline by 15+ year
	 * HTML_PARSE_NOWARNING else will forcefully print message, no thanks
	 * HTML_PARSE_NOERROR same, but for error
	 */
	auto flag = HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING | HTML_PARSE_NONET;
	if (HTMLMode) {
		//Optionally cleaned before by libTidy + some hand made tweak if needed
		doc = htmlReadMemory(data, size, nullptr, "UTF-8", flag | HTML_PARSE_NOBLANKS | HTML_PARSE_RECOVER);
	} else {
		doc = xmlParseMemory(data, size);
	}
	xmlErrorPtr error = xmlGetLastError();
	if (error != nullptr) {
		auto msg = QSL("Failed to parse document msg: %7\n line: %1 : %2 : %3 \n msg %4 \n %5 \n %6 ")
		               .arg(error->line)
		               .arg(error->int1)
		               .arg(error->int2)
		               .arg(error->str1)
		               .arg(error->str2)
		               .arg(error->str3)
		               .arg(error->message);
		return Res{false, msg};
	}
	xpath_ctx = xmlXPathNewContext(doc);
	return Res{true, QString()};
}

XPath::Res XPath::read(const QByteArray& data) {
	auto res = read(data.constData(), data.size());
	return res;
}

XPath::XPath(const QByteArray& data) {
	read(data.constData(), data.size());
}

XPath::XPath(const QString& data) {
	read(data.toUtf8());
}

XPath::~XPath() {
	if (doc) {
		xmlFreeDoc(doc);
	}
	if (xpath_ctx) {
		xmlXPathFreeContext(xpath_ctx);
	}

	doc       = nullptr;
	xpath_ctx = nullptr;
}

QByteArray XPath::getLeaf(const char* path, uint& founded) {
	auto list = getLeafs(path);
	if (list.isEmpty()) {
		founded = 0;
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

std::vector<std::vector<const char*>> XPath::getLeafs(std::vector<const char*> xPaths, xmlNodeSet* nodes) {
	std::vector<std::vector<const char*>> res;
	res.resize(nodes->nodeNr);
	if (nodes == nullptr) {
		return res;
	}
	for (uint nodePos = 0; nodePos < (uint)nodes->nodeNr; ++nodePos) {
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

std::vector<XmlNode> XPath::getNodes(const char* path, xmlNode* node, uint limit) {
	std::vector<XmlNode> nodeVec;
	xmlXPathObjectPtr    xpathObj;
	if (node) {
		xpathObj = xmlXPathNodeEval(node, (const xmlChar*)path, xpath_ctx);
	} else {
		xpathObj = xmlXPathEvalExpression((const xmlChar*)path, xpath_ctx);
	}

	if (xpathObj == nullptr || xpathObj->nodesetval == nullptr) { //in case of invalid xpath
		xmlXPathFreeObject(xpathObj);
		return nodeVec;
	} else {
		auto nodes = xpathObj->nodesetval;
		if (nodes->nodeNr == 0) {
			return nodeVec;
		}
		for (uint var = 0; var < (uint)nodes->nodeNr; ++var) {
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

XmlNode XmlNode::searchNode(const char* path) const {
	auto nodes = xml->getNodes(path, node, 1);
	if (nodes.empty()) {
		return XmlNode();
	}
	return nodes.at(0);
}

std::vector<XmlNode> XmlNode::searchNodes(const char* path) const {
	return xml->getNodes(path, node);
}

QByteArray XmlNode::searchLeaf(const char* path) const {
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

void walkTree(xmlNode* a_node) {
	xmlNode* cur_node = NULL;
	xmlAttr* cur_attr = NULL;
	for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
		// do something with that node information, like… printing the tag’s name and attributes
		printf("Got tag : %s\n", cur_node->name);
		for (cur_attr = cur_node->properties; cur_attr; cur_attr = cur_attr->next) {

			printf("  -> with attribute : %s\n", cur_attr->name);
		}
		walkTree(cur_node->children);
	}
}
QByteArray XmlNode::getProp(const char* property) const {
	QByteArray q;
	auto       vv = xmlGetProp(node, (const xmlChar*)property);
	if (vv != nullptr) {
		q.append((const char*)vv, strlen((const char*)vv));
		xmlFree(vv);
	}
	return q;
}

std::vector<QByteArray> XmlNode::getAllProp() const {
	//walkTree(node);
	std::vector<QByteArray> res;
	for (xmlAttrPtr attr = node->properties; nullptr != attr; attr = attr->next) {
		res.push_back(QByteArray((char*)attr->name));
	}
	return res;
}

QByteArray XmlNode::getContent() const {
	auto       vv = xmlNodeGetContent(node);
	QByteArray res;
	if (vv != nullptr) {
		res.append((const char*)vv, strlen((const char*)vv));
		xmlFree(vv);
	}
	return res;
}

XmlNode XmlNode::operator[](const char* path) const {
	//this is a start from here XPath! what you most expect from a [] ...
	//If you want a search RECURSIVE from here is .//something and you have to use
	//searchNode
	/**
	 * or implement the nice thread local override to tweak the behaviour
	 */
	QByteArray pathFull = QBL("./*[name()='") + path + QBL("']");
	return searchNode(pathFull.constData());
}
