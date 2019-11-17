#ifndef dbXpath_H
#define dbXpath_H

#include <QByteArrayList>
#include <libxml/xpath.h>

class xmlXPathObjectOwned {
      public:
	xmlXPathObjectOwned(xmlXPathObjectPtr ptr) {
		this->ptr = ptr;
	}
	~xmlXPathObjectOwned() {
		xmlXPathFreeObject(ptr);
	}
	xmlXPathObjectPtr get(){
		return ptr;
	}
	xmlNodeSetPtr operator->() {
		if(ptr == nullptr){
			return nullptr;
		}
		return ptr->nodesetval;
	}

      private:
	xmlXPathObjectPtr ptr = nullptr;
};

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
	QByteArray                            getLeaf(const char* path, xmlNodePtr node);
	QByteArrayList                        getLeafs(const char* path);
	std::vector<std::vector<const char*>> getLeafs(std::vector<const char*> path, xmlNodeSetPtr nodes);
	xmlXPathObjectOwned                   getNodes(const char* path);
	xmlDocPtr                             doc;
	xmlXPathContextPtr                    xpath_ctx = nullptr;
};

class XmlNode {
      public:
	XPath*     xml  = nullptr;
	xmlNodePtr node = nullptr;
	xmlNodePtr getNode(const char* path);
	QByteArray getLeaf(const char* path);
	void       swapLeaf(const char* path, double& val);
	void       swapLeaf(const char* path, quint64& val);
	void       swapLeaf(const char* path, QString& val);
	void       swapLeaf(const char* path, QByteArray& val);

	void       swapLeafValue(const char* path, double& val);
	void       swapLeafValue(const char* path, quint64& val);
	void       swapLeafValue(const char* path, QString& val);
	void       swapLeafValue(const char* path, QByteArray& val);
	QByteArray swapLeafValue(const char* path);

	void       swapAttr(const char* path, QByteArray& val);
	QByteArray getValue(const xmlNodePtr node);
};

#endif // dbXpath
