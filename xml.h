#ifndef dbXpath_H
#define dbXpath_H

#include <QByteArrayList>
#include <libxml/xpath.h>

class XPath {
      public:
	void read(const char* data, int size);
	void read(const QByteArray& data);

	/**
	 * @brief getLeaf is meant for extracting a SINGLE leaf, if multiple are founded only the first is returned!
	 * This is just a helper function for quick stuff
	 * @param path something like "//bookstore/book[1]/author"
	 * @param founded is just to check if
	 * @return
	 */
	QByteArray     getLeaf(const char* path, uint& founded);
	QByteArrayList getLeafs(const char* path);
	xmlDocPtr      doc;
};

#endif // dbXpath
