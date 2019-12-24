#ifndef dbXpath_H
#define dbXpath_H

#include <QByteArrayList>
//Forward declaration slightly difficult
#include <libxml/xpath.h>

//this toggles several qDebug in the code to print or not in case of missing value
inline thread_local bool xmlVerbose = true;

class XmlNode;
class XPath {
      public:
	bool read(const char* data, int size);
	bool read(const QByteArray& data);
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
	xmlDocPtr                             doc       = nullptr;
	xmlXPathContextPtr                    xpath_ctx = nullptr;
	//If on will relax check for malformed HTML
	bool HTMLMode = false;
};

class XmlNode {
      public:
	XPath*                  xml  = nullptr;
	xmlNodePtr              node = nullptr;
	XmlNode                 searchNode(const char* path) const;
	std::vector<XmlNode>    searchNodes(const char* path) const;
	QByteArray              searchLeaf(const char* path) const;
	QByteArray              getProp(const char* property) const;
	std::vector<QByteArray> getAllProp() const;
	QByteArray              getContent() const;
	XmlNode                 operator[](const char* path) const;
};

#endif // dbXpath
