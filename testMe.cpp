#include "gumbo-parser/src/gumbo.h"
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <gumbo_libxml.h>

#include "libxml/tree.h"
#include "libxml/xpath.h"


static void
print_element_names(xmlNode* a_node) {
	xmlNode* cur_node = NULL;

	for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
		if (cur_node->type == XML_ELEMENT_NODE) {
			auto txt = xmlNodeGetContent(cur_node);
			printf("name: %s   content: %s \n", cur_node->name, txt);
		}

		print_element_names(cur_node->children);
	}
}
/**
 * print_xpath_nodes:
 * @nodes:		the nodes set.
 * @output:		the output file handle.
 *
 * Prints the @nodes content to @output.
 */
void print_xpath_nodes(xmlNodeSetPtr nodes, FILE* output = stdout) {
	xmlNodePtr cur;
	int        size;
	int        i;

	assert(output);
	size = (nodes) ? nodes->nodeNr : 0;

	fprintf(output, "Result (%d nodes):\n", size);
	for (i = 0; i < size; ++i) {
		assert(nodes->nodeTab[i]);
		auto curNode = nodes->nodeTab[i];
		print_element_names(curNode);
	}
}

static void delete_nodes(xmlDocPtr doc, const char* xpath_expr) {
	xmlXPathContextPtr xpath_ctx = xmlXPathNewContext(doc);
	if (xpath_ctx == NULL) {
		fprintf(stderr, "Error: unable to create new XPath context.\n");
		return;
	}
	xmlXPathObjectPtr xpath_obj = xmlXPathEvalExpression((xmlChar*)xpath_expr, xpath_ctx);
	if (xpath_obj == NULL) {
		fprintf(stderr, "Error: unable to create new XPath context.\n");
		xmlXPathFreeContext(xpath_ctx);
		return;
	}

	// It's often tricky to combine mutations and XPath in the same pass, because
	// XPath may select descendant nodes that are eliminated by the mutation.  For
	// this reason, you may want to iterate in reverse document order so that
	// children are mutated before they're thrown away.  In this particular
	// example, it doesn't matter since all the tags we scrub don't have children,
	// but see the note in:
	// http://www.xmlsoft.org/examples/xpath2.c
	for (int i = xpath_obj->nodesetval->nodeNr - 1; i >= 0; i--) {
		xmlNodePtr node = xpath_obj->nodesetval->nodeTab[i];
		xmlUnlinkNode(node);
		xmlFreeNode(node);
	}

	xmlXPathFreeObject(xpath_obj);
	xmlXPathFreeContext(xpath_ctx);
}

void getInfo(xmlDocPtr doc) {
	xmlXPathContextPtr xpath_ctx = xmlXPathNewContext(doc);
	xmlXPathObjectPtr  xpathObj  = xmlXPathEvalExpression((xmlChar*)"//*[@id='field-table']/tbody/tr", xpath_ctx);
	auto               nodes     = xpathObj->nodesetval;
	for (int var = 1; var <= nodes->nodeNr; ++var) {
		auto v1 = QString("//*[@id='field-table']/tbody/tr[%1]/th").arg(var).toUtf8();
		auto v = v1.constData();
		auto ptr = xmlXPathEvalExpression((xmlChar*) v, xpath_ctx)->nodesetval;
		auto vv = xmlNodeGetContent(ptr->nodeTab[0]);
		printf("%s \n",vv);
	}
	//print_xpath_nodes(, stdout);
}


int main() {
	int              x = 0;
	QCoreApplication a(x, nullptr);
	QFile            cached;
	cached.setFileName("cached.html");
	if (!cached.open(QIODevice::ReadOnly)) {
		qCritical() << "e che diamine";
	}
	QByteArray pay = cached.readAll();

	xmlDocPtr doc = gumbo_libxml_parse(pay.constData());

	QByteArray xml = R"EOD(
		<bookstore>
		
		<book category="COOKING">
		<title lang="en">Everyday Italian</title>
		<author>Giada De Laurentiis</author>
		<year>2005</year>
		<price>30.00</price>
		</book>
		
		<book category="CHILDREN">
		<title lang="en">Harry Potter</title>
		<author>J K. Rowling</author>
		<year>2005</year>
		<price>29.99</price>
		</book>
		
		<book category="WEB">
		<title lang="en">XQuery Kick Start</title>
		<author>James McGovern</author>
		<author>Per Bothner</author>
		<author>Kurt Cagle</author>
		<author>James Linn</author>
		<author>Vaidyanathan Nagarajan</author>
		<year>2003</year>
		<price>49.99</price>
		</book>
		
		<book category="WEB">
		<title lang="en">Learning XML</title>
		<author>Erik T. Ray</author>
		<year>2003</year>
		<price>39.95</price>
		</book>
		
		</bookstore>
		)EOD";
		
		
		XPath p(xml);
		auto v = p.getLeafs("//bookstore/book/author");
		
	//delete_nodes(doc, "//link");
	getInfo(doc);

	return 0;
}
