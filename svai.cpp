 
 
 //void print_element_names(xmlNode* a_node) {
 //	xmlNode* cur_node = NULL;
 
 //	for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
 //		if (cur_node->type == XML_ELEMENT_NODE) {
 //			auto txt = xmlNodeGetContent(cur_node);
 //			printf("name: %s   content: %s \n", cur_node->name, txt);
 //		}
 
 //		print_element_names(cur_node->children);
 //	}
 //}
 ///**
 // * print_xpath_nodes:
 // * @nodes:		the nodes set.
 // * @output:		the output file handle.
 // *
 // * Prints the @nodes content to @output.
 // */
 //void print_xpath_nodes(xmlNodeSetPtr nodes, FILE* output = stdout) {
 //	xmlNodePtr cur;
 //	int        size;
 //	int        i;
 
 //	assert(output);
 //	size = (nodes) ? nodes->nodeNr : 0;
 
 //	fprintf(output, "Result (%d nodes):\n", size);
 //	for (i = 0; i < size; ++i) {
 //		assert(nodes->nodeTab[i]);
 //		auto curNode = nodes->nodeTab[i];
 //		print_element_names(curNode);
 //	}
 //}
 
 //void delete_nodes(xmlDocPtr doc, const char* xpath_expr) {
 //	xmlXPathContextPtr xpath_ctx = xmlXPathNewContext(doc);
 //	if (xpath_ctx == NULL) {
 //		fprintf(stderr, "Error: unable to create new XPath context.\n");
 //		return;
 //	}
 //	xmlXPathObjectPtr xpath_obj = xmlXPathEvalExpression((xmlChar*)xpath_expr, xpath_ctx);
 //	if (xpath_obj == NULL) {
 //		fprintf(stderr, "Error: unable to create new XPath context.\n");
 //		xmlXPathFreeContext(xpath_ctx);
 //		return;
 //	}
 
 //	// It's often tricky to combine mutations and XPath in the same pass, because
 //	// XPath may select descendant nodes that are eliminated by the mutation.  For
 //	// this reason, you may want to iterate in reverse document order so that
 //	// children are mutated before they're thrown away.  In this particular
 //	// example, it doesn't matter since all the tags we scrub don't have children,
 //	// but see the note in:
 //	// http://www.xmlsoft.org/examples/xpath2.c
 //	for (int i = xpath_obj->nodesetval->nodeNr - 1; i >= 0; i--) {
 //		xmlNodePtr node = xpath_obj->nodesetval->nodeTab[i];
 //		xmlUnlinkNode(node);
 //		xmlFreeNode(node);
 //	}
 
 //	xmlXPathFreeObject(xpath_obj);
 //	xmlXPathFreeContext(xpath_ctx);
 //}
