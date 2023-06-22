#ifndef dbXpath_H
#define dbXpath_H

#include <QByteArrayList>
//Forward declaration slightly difficult
#include <expected>
#include <libxml/xpath.h>
#include <optional>

auto cleanse1(const QByteArray& original) -> std::expected<QByteArray,std::string>;


void walkTree(xmlNode* a_node);

class XmlNode;
class XPath {
      public:
	struct Res {
		bool    ok = false;
		QString msg;
	};
	Res read(const char* data, int size);
	Res read(const QByteArray& data);
	XPath() = default;
	XPath(const QByteArray& data);
	XPath(const QString& data);
	~XPath();
	/**
	 * @brief getLeaf is meant for extracting a SINGLE leaf, if multiple are founded only the first is returned!
	 * This is just a helper function for quick stuff
	 * @param path something like "//bookstore/book[1]/author"
	 * @return
	 */
	std::optional<QByteArray> getLeaf(const char* path, xmlNodePtr node);

	QByteArrayList getLeafs(const char* path, xmlNodePtr outerNode = nullptr);

	std::vector<XmlNode> getNodes(const char* path, xmlNodePtr node = nullptr, uint limit = 0xFFFFFFFF);
	xmlDocPtr            doc       = nullptr;
	xmlXPathContextPtr   xpath_ctx = nullptr;
	//If on will relax check for malformed HTML
	bool HTMLMode = false;

	//what is this orror ? is this ever used ?
	//std::vector<std::vector<const char*>> getLeafs(std::vector<const char*> path, xmlNodeSetPtr nodes);
};

class XmlNode {
      public:
	XPath*     xml  = nullptr;
	xmlNodePtr node = nullptr;

	std::optional<XmlNode> getNode(const char* path) const;
	std::vector<XmlNode>   getNodes(const char* path) const;

	std::optional<QByteArray> getLeaf(const char* path) const;
	QByteArrayList            getLeafs(const char* path);

	QByteArray              getProp(const char* property) const;
	std::vector<QByteArray> getAllProp() const;
	QByteArray              getContent() const;
	XmlNode                 operator[](const char* path) const;

	operator xmlNodePtr() const;
};

#endif // dbXpath
