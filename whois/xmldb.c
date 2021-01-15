#include "whois.h"

#include "../resource.h"

static HGLOBAL xmlres;
static xmlDocPtr xmldoc;
static xmlXPathContextPtr xpath;

static const char* xpath_template[] = {
	"/domainList/domain[@name='%s']/whoisServer",
	"/domainList/domain[@name='%s']/domain[@name='%s.%s']/whoisServer"
};

bool __cdecl xmldb_load(void) {
	if (!xmldb_ready) {
		HMODULE handle = GetModuleHandle(NULL);
		HRSRC rc = FindResource(handle, MAKEINTRESOURCE(XML_DATA), MAKEINTRESOURCE(XML_RC));
		if (rc) {
			xmlres = LoadResource(handle, rc);
			const char* xml = (const char*)LockResource(xmlres);
			if (xml) {
				xmldoc = xmlParseMemory(xml, (int)SizeofResource(handle, rc));
				if (xmldoc) xpath = xmlXPathNewContext(xmldoc);
				return xmldb_ready;
			}
		}
		return false;
	}
	else return true;
}

void __cdecl xmldb_free(void) {
	if (xpath) {
		xmlXPathFreeContext(xpath);
		xpath = NULL;
	}
	if (xmldoc) {
		xmlFree(xmldoc);
		xmldoc = NULL;
	}
	if (xmlres) {
		FreeResource(xmlres);
		xmlres = NULL;
	}
}

xmldb_record_ptr __cdecl xmldb_get_record(char* domain) {
	static char path[XMLDB_PATH_LEN];
	size_t zones;
	xmlXPathObjectPtr obj;
	xmlChar* key;
	array_ptr dom = NULL, a = NULL, a1 = NULL, a2 = NULL;
	xmldb_zone_ptr zone = NULL;
	xmldb_record_ptr rec = NULL;
	if (!domain || !xmldb_ready) return NULL;

	// Lookup whois servers for domain name in XML database
	dom = explode(domain, '.');
	if (dom) {
		// At first find .XXX.TLD if 'dom' array size > 2
		if (dom->size > 2) {
			memset(path, 0, XMLDB_PATH_LEN);
			sprintf_s(path, XMLDB_PATH_LEN,
				"/domainList/domain[@name='%s']/domain[@name='%s.%s']/whoisServer",
				dom->items[dom->size - 1], dom->items[dom->size - 2], dom->items[dom->size - 1]);
			obj = xmlXPathEvalExpression(path, xpath);
			if (obj && obj->nodesetval && obj->nodesetval->nodeNr > 0) {
				a1 = array_new(obj->nodesetval->nodeNr);
				if (a1 && a1->size == obj->nodesetval->nodeNr) {
					for (int i = 0; i < obj->nodesetval->nodeNr; i++) {
						key = xmlGetProp(obj->nodesetval->nodeTab[i], "host");
						if (key) {
							array_set(a1, i, key);
							xmlFree(key);
						}
					}
				}
				xmlXPathFreeObject(obj);
			}
		}
		// Next lookup for .TLD
		memset(path, 0, XMLDB_PATH_LEN);
		sprintf_s(path, XMLDB_PATH_LEN,
			"/domainList/domain[@name='%s']/whoisServer",
			dom->items[dom->size - 1]);
		obj = xmlXPathEvalExpression(path, xpath);
		if (obj && obj->nodesetval && obj->nodesetval->nodeNr > 0) {
			a2 = array_new(obj->nodesetval->nodeNr);
			if (a2 && a2->size == obj->nodesetval->nodeNr) {
				for (int i = 0; i < obj->nodesetval->nodeNr; i++) {
					key = xmlGetProp(obj->nodesetval->nodeTab[i], "host");
					if (key) {
						array_set(a2, i, key);
						xmlFree(key);
					}
				}
			}
			xmlXPathFreeObject(obj);
		}
		// Building 'xmldb_record'
		zones = (a1 ? 1 : 0) + (a2 ? 1 : 0);
		if (zones) {
			// Allocate memory
			rec = malloc(sizeof(xmldb_record));
			if (rec) {
				rec->zones = zones;
				// Copy domain name
				rec->domain = malloc(strlen(domain) + 1);
				memcpy(rec->domain, domain, strlen(domain) + 1);
				// Allocate memory for each xmldb zone
				rec->list = malloc(sizeof(xmldb_zone_ptr) * zones);
				if (rec->list) {
					// First zone always is .TLD
					// Copy 1st level name
					rec->list[0] = malloc(sizeof(xmldb_zone));
					rec->list[0]->zone = bcopy(dom->items[dom->size - 1], strlen(dom->items[dom->size - 1]) + 1);
					// Assign array of whois servers
					rec->list[0]->whois_servers = a2;

					// Next zone is XXX.TLD if exist
					if (a1) {
						rec->list[1] = malloc(sizeof(xmldb_zone));
						rec->list[1]->zone = concat_s(dom->items[dom->size - 2], dom->items[dom->size - 1], '.');
						// Assign array of whois servers
						rec->list[1]->whois_servers = a1;
					}
				}
			}
		}
	}
	// Cleanup temporary array
	array_free(dom);

	return rec;
}

void __cdecl xmldb_free_record(xmldb_record_ptr rec) {
	size_t i = 0;
	if (rec) {
		// Cleanup each zone
		if (rec->zones) {
			for (i = 0; i < rec->zones; i++) {
				// Cleanup array of whois servers
				array_free((rec->list[i])->whois_servers);
				// Cleanup zone name
				free(rec->list[i]->zone);
				// Cleanup zone
				free(rec->list[i]);
			}
			// Cleanup zones array
			free(rec->list);
		}
		// Cleanup domain name
		free(rec->domain);
		// Cleanup record
		free(rec);
	}
}
