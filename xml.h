#ifndef dbXpath_H
#define dbXpath_H

#include <QByteArrayList>
#include <libxml/xpath.h>

class XmlNode;
class XPath {
      public:
	void read(const char* data, int size);
	void read(const QByteArray& data);
	XPath() = default;
	XPath(const QByteArray& data);
	XPath(const QString& data);
	~XPath();
	/**
	 * @brief getLeaf is meant for extracting a SINGLE leaf, if multiple are founded only the first is returned!
	 * This is just a helper function for quick stuff
	 * @param path something like "//bookstore/book[1]/author"
	 * @param founded is just to check if
	 * @return
	 */
	QByteArray                            getLeaf(const char* path, uint& founded);
	QByteArray                            getLeaf(const char* path);
	QByteArray                            getLeaf(const char* path, xmlNodePtr node);
	QByteArrayList                        getLeafs(const char* path);
	std::vector<std::vector<const char*>> getLeafs(std::vector<const char*> path, xmlNodeSetPtr nodes);
	std::vector<XmlNode>                  getNodes(const char* path, xmlNodePtr node = nullptr, uint limit = 0xFFFFFFFF);
	xmlDocPtr                             doc;
	xmlXPathContextPtr                    xpath_ctx = nullptr;
};

class XmlNode {
      public:
	XPath*               xml  = nullptr;
	xmlNodePtr           node = nullptr;
	XmlNode              searchNode(const char* path);
	std::vector<XmlNode> searchNodes(const char* path);
	QByteArray           searchLeaf(const char* path);
	QByteArray           getProp(const char* property);
	QByteArray           getContent();
	XmlNode              operator[](const char* path);
};

#endif // dbXpath
