#include "xml.h"
#include "rbk/QStacker/exceptionv2.h"
#include "rbk/defines/stringDefine.h"
#include "rbk/fmtExtra/includeMe.h"
#include <libxml/HTMLparser.h>
#include <libxml/xpathInternals.h>

#include <tidy/tidy.h>
#include <tidy/tidybuffio.h>

/* only for debug,
#include <QDebug>
#include <libxml2/libxml/xpath.h>
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

void checkXPahtIsRelative(const xmlChar* xpathExpression) {
	std::string path = (char*)xpathExpression;

	if (path.starts_with("//")) {
		throw ExceptionV2(F("A xPath relative to current node must be .// ! You forget the . in {}", path));
	}
	if (path.starts_with("/")) {
		throw ExceptionV2(F("A xPath relative to current node must be ./ ! You forget the . in {}", path));
	}
}

void checkXPathIsValid(const xmlChar* xpathExpression) {
	xmlXPathCompExprPtr xpathCompExpr = xmlXPathCtxtCompile(NULL, xpathExpression);

	if (xpathCompExpr == NULL) {

		xmlErrorPtr error = xmlGetLastError();
		std::string errMsg;
		if (error != NULL) {
			errMsg.append(error->message);
			xmlResetLastError();
		}

		throw ExceptionV2(F("invalid Xpath {}: Error : {}", (char*)xpathExpression, errMsg));
	}

	xmlXPathFreeCompExpr(xpathCompExpr);
}

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

std::optional<QByteArray> XPath::getLeaf(const char* path, xmlNodePtr node) {

	if (node) {
		checkXPahtIsRelative((const xmlChar*)path);
	}

	checkXPathIsValid((const xmlChar*)path);

	xmlXPathObjectPtr xpathObj = xmlXPathNodeEval(node, (const xmlChar*)path, xpath_ctx);
	if (xpathObj != nullptr) {

		auto nodes = xpathObj->nodesetval;
		if (nodes->nodeNr == 0) {
			return {};
		}
		auto _node = nodes->nodeTab[0];
		auto vv    = xmlNodeGetContent(_node);
		if (vv != nullptr) {
			QByteArray res;
			res.setRawData((const char*)vv, strlen((const char*)vv));
			return res;
		}
	}
	return {};
}

QByteArrayList XPath::getLeafs(const char* path, xmlNodePtr outerNode) {
	QByteArrayList res;
	auto           nodes = getNodes(path, outerNode);
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

//std::vector<std::vector<const char*>> XPath::getLeafs(std::vector<const char*> xPaths, xmlNodeSet* nodes) {
//	std::vector<std::vector<const char*>> res;
//	res.resize(nodes->nodeNr);
//	if (nodes == nullptr) {
//		return res;
//	}
//	for (uint nodePos = 0; nodePos < (uint)nodes->nodeNr; ++nodePos) {
//		auto node = nodes->nodeTab[nodePos];
//		for (uint pathPos = 0; pathPos < xPaths.size(); ++pathPos) {
//			auto                      path = xPaths.at(pathPos);
//			std::vector<const char*>& cur  = res.at(nodePos);

//			if (node) {
//				checkXPahtIsRelative((const xmlChar*)path);
//			}

//			checkXPathIsValid((const xmlChar*)path);

//			xmlXPathObjectPtr xpathObj = xmlXPathNodeEval(node, (const xmlChar*)path, xpath_ctx);
//			if (xpathObj == nullptr) {
//				return res;
//			}
//			auto _nodes = xpathObj->nodesetval;
//			for (int var = 0; var < _nodes->nodeNr; ++var) {
//				auto _node = _nodes->nodeTab[var];
//				auto vv    = xmlNodeGetContent(_node);
//				if (vv != nullptr) {
//					cur.push_back((const char*)vv);
//				}
//			}
//		}
//	}
//	return res;
//}

std::vector<XmlNode> XPath::getNodes(const char* path, xmlNode* node, uint limit) {
	std::vector<XmlNode> nodeVec;
	xmlXPathObjectPtr    xpathObj;

	if (node) {
		checkXPahtIsRelative((const xmlChar*)path);
	}

	checkXPathIsValid((const xmlChar*)path);

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
			auto    _node = nodes->nodeTab[var];
			XmlNode nod{this, _node};
			nodeVec.push_back(nod);
		}
	}
	xmlXPathFreeObject(xpathObj);
	return nodeVec;
}

std::optional<XmlNode> XmlNode::getNode(const char* path) const {
	auto nodes = xml->getNodes(path, node, 1);
	switch (nodes.size()) {
	case 0:
		return {};
		break;
	case 1:
		return nodes.at(0);
	default:
		throw ExceptionV2(F("multiple nodes found for {}", path));
		break;
	}
}

std::vector<XmlNode> XmlNode::getNodes(const char* path) const {
	return xml->getNodes(path, node);
}

std::optional<QByteArray> XmlNode::getLeaf(const char* path) const {
	return xml->getLeaf(path, node);
}

QByteArrayList XmlNode::getLeafs(const char* path) {
	return xml->getLeafs(path, node);
}

void walkTree(xmlNode* a_node) {
	//use like walkTree(xmlDocGetRootElement(html.doc));

	xmlNode* cur_node = NULL;
	xmlAttr* cur_attr = NULL;
	for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
		// do something with that node information, like… printing the tag’s name and attributes
		printf("Got tag : %s\n", cur_node->name);
		for (cur_attr = cur_node->properties; cur_attr; cur_attr = cur_attr->next) {

			xmlChar* value = xmlNodeListGetString(cur_node->doc, cur_attr->children, 1);

			printf("  -> with attribute : %s : {%s} \n", cur_attr->name, value);
			xmlFree(value);
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
		res.append((const char*)vv, static_cast<int>(strlen((const char*)vv)));
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
	return getNode(pathFull.constData()).value_or(XmlNode());
}

XmlNode::operator xmlNodePtr() const {
	return node;
}

// Function to recursively traverse the parse tree and remove script tags
void removeScriptTags(TidyDoc tdoc, TidyNode node) {
	TidyNode child;
	for (child = tidyGetChild(node); child; child = tidyGetNext(child)) {
		TidyTagId tagId = tidyNodeGetId(child);
		if (tagId == TidyTag_SCRIPT) {
			child = tidyDiscardElement(tdoc, child);
		} else {
			removeScriptTags(tdoc, child); // Recursively process child nodes
		}
		if (!child) {
			break;
		}
	}
}

//https://www.html-tidy.org/developer/
auto cleanse1(const QByteArray& original) -> std::expected<QByteArray, std::string> {
	TidyBuffer output = {0, nullptr, 0, 0, 0};
	TidyBuffer errbuf = {0, nullptr, 0, 0, 0};
	int        rc     = -1;
	bool       ok;

	TidyDoc tdoc = tidyCreate(); // Initialize "document"

	ok = tidyOptSetBool(tdoc, TidyXhtmlOut, Bool::yes); // Convert to XHTML
	ok &= tidyOptSetBool(tdoc, TidyDropEmptyElems, Bool::yes);
	ok &= tidyOptSetInt(tdoc, TidyIndentContent, 2);

	if (ok)
		rc = tidySetErrorBuffer(tdoc, &errbuf); // Capture diagnostics
	if (rc >= 0)
		rc = tidyParseString(tdoc, original.constData()); // Parse the input
	if (rc >= 0)
		rc = tidyCleanAndRepair(tdoc); // Tidy it up!
	if (rc >= 0)
		rc = tidyRunDiagnostics(tdoc); // Kvetch
	if (rc > 1) {                          // If error, force output.
		rc = (tidyOptSetBool(tdoc, TidyForceOutput, yes) ? rc : -1);
	}

	if (rc >= 0) {
		removeScriptTags(tdoc, tidyGetRoot(tdoc));
		rc = tidySaveBuffer(tdoc, &output); // Pretty Print
	}

	if (rc > 1) {
		std::string msg = F("error {} in libtidy cleaning: {} ", (char*)tidyErrorCodeAsKey(rc), (char*)errbuf.bp);
		return std::unexpected(msg);
	}

	QByteArray res;
	res.append((const char*)output.bp, output.size);

	tidyBufFree(&output);
	tidyBufFree(&errbuf);
	tidyRelease(tdoc);
	return res;
}
