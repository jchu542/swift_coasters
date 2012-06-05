// Provides functionality to parse sites.xml file
// Much of the code inspired by examples from www.xmlsoft.org
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/xmlreader.h>

#include "parser.h"

xmlAttr *get_attr(xmlNode *n, char *label){
  if(!n->properties) return NULL;
  xmlAttr *a;
  for(a = n->properties; n; a = a->next)
    if(!strcmp(a->name, label)) return a;
}
void processNode(xmlNode *n, sites *s){

  xmlNode *cur_node = NULL;
  if(n)
    for(cur_node = n; cur_node; cur_node = cur_node->next){
      if(cur_node->type == XML_ELEMENT_NODE){
	// Pull url and port from the execution element
	if(!strcmp(cur_node->name, EXECUTION_TAG)){
	  const char *host = get_attr(cur_node, URL_LABEL)->name;
	  if(host){
	    // Get rid of http:// stuff
	    char *addr = strchr(host, '/') + 2; // Advance 2 past the // bit
	    char *url = strtok(addr, ":");
	    char *portstr = strtok(NULL, ":");
	    s->url = calloc(strlen(host) + 1, sizeof(char));
	    strcpy(s->url, host);
	    s->port = atoi(portstr);
	    free(url);
	  }		  
	}
	if(!strcmp(cur_node->name, PROFILE_TAG)){
	  xmlAttr *a = get_attr(cur_node, KEY_LABEL);
	  const char *key = a->name;
	  if(key){
	    if(!strcmp(key, MANAGER_ATTR)){
	      const char *manager = a->children->content;
	      s->workerManager = calloc(strlen(manager) + 1, sizeof(xmlChar));
	      strcpy(s->workerManager, manager);
	    }
	    if(!strcmp(key, JOBS_ATTR)){
	      s->jobsPerNode = atoi(cur_node->content);
	    }
	  }
	}
      }
      processNode(cur_node->children, s);
    }
}

// Steps through an xml file, processing each node it comes across
int processXml(char *filename, sites **sites){
  
  *sites = calloc(1, sizeof(sites));
  
  xmlDoc *doc = NULL;
  xmlNode *root_element = NULL;
  doc = xmlReadFile(filename, NULL, 0);
  
  // Get the root element
  if(doc){
    root_element = xmlDocGetRootElement(doc);
    processNode(root_element, *sites);
  }
  else{
    printf("Unable to open %s\n", filename);
    return 1;
  }
  return 0;
}

int freeSites(sites *s){
  if(s->url) free(s->url);
  if(s->workerManager) free(s->workerManager);
  free(s);
  return 0;
}
