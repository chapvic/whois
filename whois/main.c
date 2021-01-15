#include "whois.h"

#pragma comment (lib, "ws2_32")
#pragma comment (lib, "mswsock")
#pragma comment (lib, "advapi32")

extern char* error_str = NULL;
extern int error_val = 0;

int main(int argc, char* argv[]) {
	char* domain;
	whois_ptr whois;

	if (argc < 2) {
		puts("WHOIS, v1.0\nCopyright (c) 2020-2021 FoxTeam\n\nUsage: whois <domain>");
		return 0;
	}

	domain = argv[1];

	if (xmldb_load()) {
		atexit(xmldb_free);
		whois = whois_open(domain, 0);
		if (whois) {
			sockopt_t opt = { 0 };
			opt.non_blocking = false;
			opt.send_timeout = 3;
			opt.port = "43";
			opt.query = whois->query;
			while (whois->req < whois->servers->size) {
				opt.state = 0;
				opt.server = whois->servers->items[whois->req];
				sock_send(&opt);

				whois->req++;
			}
			//whois_request(whois);
			whois_close(&whois);
		}
		else {
			printf("No whois servers found for '%s'!\n", domain);
		}
	}

#ifdef _DEBUG
	system("pause");
#endif
	return 0;
}
