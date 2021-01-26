#include "whois.h"
#include "optarg.h"

#pragma comment (lib, "ws2_32")
#pragma comment (lib, "mswsock")
#pragma comment (lib, "advapi32")

extern char* error_str = NULL;
extern int error_val = 0;

// ----------------------------------------
// Initializing OPTARG's external variables
// ----------------------------------------

extern OPTARG __optarg[] = {
	{ "h", "", "", "This help", FALSE },
	{ "s", "", "<whois-server>", "Use specified whois server", TRUE }
};

extern size_t __optarg_size = ARRAY_SIZE(__optarg);
// ----------------------------------------

void show_help() {
	puts("WHOIS, v1.0\nCopyright (c) 2020-2021 FoxTeam\n\nUsage: whois [options] <domain>");
	optarg_help();
}

int main(int argc, char* argv[]) {
	char* domain;
	whois_ptr whois;

	if (argc < 2) {
		show_help();
		return 0;
	}

	atexit(optarg_free);
	optarg_init(argc, argv);

	// check if '-h' option was selected
	if (__optarg[0].exist) {
		show_help();
		return 0;
	}

	if (__optarg_result.error) {
		printf("Error: ");
		switch (__optarg_result.error) {
		case E_OPTARG_INVALID_OPTION:
			printf("Invalid option '%s'!\n", __optarg_result.invalid);
			break;
		case E_OPTARG_NO_PARAM:
			printf("No parameter found for '%s' option!\n",
				__optarg_result.optarg->longopt ? __optarg_result.optarg->longname : __optarg_result.optarg->name);
			break;
		case E_OPTARG_PARAM_IS_OPTION:
			puts("Parameter has option style!");
			break;
		case E_OPTARG_AFTER_NON_OPTION:
			puts("Domain name must be a last parameter!");
			break;
		case E_OPTARG_ALREADY_EXIST:
			printf("Option '%s' is already present in the command line!\n",
				__optarg_result.optarg->longopt ? __optarg_result.optarg->longname : __optarg_result.optarg->name);
			break;
		default:
			puts("Unknown error!");
		}
		return 1;
	}

	// check for domain name present
	if (!__optarg_result.list->items[0]) {
		puts("Error: No domain name given!");
		return 1;
	}

	domain = __optarg_result.list->items[0];

	if (xmldb_load()) {
		atexit(xmldb_free);
		whois = whois_open(domain, 0);
		if (whois) {
			sockopt_t so = { 0 };
			so.non_blocking = false;
			so.send_timeout = 3;
			so.port = "43";
			so.query = whois->query;

			// use specified whois-server
			if (__optarg[1].value) {
				printf("Using whois server '%s'...\n\n", __optarg[1].value);
				so.state = 0;
				so.server = __optarg[1].value;
				sock_send(&so);
			}
			// use preconfigured whois-servers
			else {
				while (whois->req < whois->servers->size) {
					so.state = 0;
					so.server = whois->servers->items[whois->req];
					sock_send(&so);

					whois->req++;
				}
			}
			// close whois request
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
