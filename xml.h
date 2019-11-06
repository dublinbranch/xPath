#ifndef dbXpath_H
#define dbXpath_H

#include <QByteArrayList>
#include <libxml/xpath.h>

class XPath {
      public:
	void read(const char* data, int size);
	void read(const QByteArray& data);
	XPath() = default;
	XPath(const QByteArray& data);
	XPath(const QString& data);
	/**
	 * @brief getLeaf is meant for extracting a SINGLE leaf, if multiple are founded only the first is returned!
	 * This is just a helper function for quick stuff
	 * @param path something like "//bookstore/book[1]/author"
	 * @param founded is just to check if
	 * @return
	 */
	QByteArray                            getLeaf(const char* path, uint& founded);
	QByteArray                            getLeaf(const char* path);
	QByteArrayList                        getLeafs(const char* path);
	std::vector<std::vector<const char*>> getLeafs(std::vector<const char*> path, xmlNodeSetPtr nodes);
	xmlNodeSetPtr                         getNodes(const char* path);
	xmlDocPtr                             doc;
	xmlXPathContextPtr                    xpath_ctx = nullptr;
};

#endif // dbXpath
