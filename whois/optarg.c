/***************************************************************************

MIT License

Copyright (c) 2021 FoxTeam

Permission is hereby granted, free of charge, to any person obtaining a copy
of this softwareand associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright noticeand this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*****************************************************************************/

#include "optarg.h"

#define OPTARG_BUFF_SIZE 128   // default print buffer size for optarg help
#define OPTARG_LIST_SIZE 4     // initial value of list size
#define OPTARG_LIST_EXPAND 4   // expandable value

extern OPTARG_RESULT __optarg_result = { 0 };

static POPTARG __cdecl optarg_check(TCHAR* arg) {
	POPTARG retval = NULL;
	BOOL longopt = FALSE;
	int found = 0;
	// check argument
	if (arg && arg[0] == _T('-')) {
		++arg;
		// check for long option
		if (arg[0] == _T('-')) {
			longopt = TRUE;
			++arg;
		}
		for (size_t i = 0; i < __optarg_size; i++) {
			found = (longopt) ? !strcmp(arg, __optarg[i].longname) :
				!strcmp(arg, __optarg[i].name);
			if (found) {
				retval = &__optarg[i];
				retval->longopt = longopt;
				break;
			}
		}
	}
	return retval;
}

static void optarg_list_create() {
	// do not create, if pointer is not null
	if (__optarg_result.list) return;
	// allocate size for list structure
	__optarg_result.list = malloc(sizeof(OPTARG_LIST));
	if (__optarg_result.list) {
		// save initial list size
		__optarg_result.list->size = OPTARG_LIST_SIZE;
		// allocate memory for list items
		__optarg_result.list->items = malloc(sizeof(TCHAR*) * OPTARG_LIST_SIZE);
	}
}

static void optarg_list_add(TCHAR* value) {
	size_t i;
	// do not save null value
	if (value) {
		// check if list is not created
		if (!__optarg_result.list) optarg_list_create();
		// skip non-zero items
		for (i = 0; i < __optarg_result.list->size; i++) {
			if (!__optarg_result.list->items[i]) {
				// save value as list item
				__optarg_result.list->items[i] = value;
				goto Exit;
			}
		}
		// list is full and need to be expanded

		// not implemented yet!
	}
Exit:
	return;
}

void __cdecl optarg_list_free() {
	if (__optarg_result.list) {
		free(__optarg_result.list->items);
		free(__optarg_result.list);
	}
}

void __cdecl optarg_init(int argc, TCHAR** argv) {
	if (argc > 1) {
		optarg_list_create();
		for (int i = 1; i < argc; i++) {
			TCHAR *arg = argv[i];
			__optarg_result.optarg = optarg_check(argv[i]);
			// if option found
			if (__optarg_result.optarg) {
				// check for non-options value already present
				if (__optarg_result.list->items[0]) {
					__optarg_result.error = E_OPTARG_AFTER_NON_OPTION;
					goto Exit;
				}
				// check for duplicate option
				if (__optarg_result.optarg->exist) {
					__optarg_result.error = E_OPTARG_ALREADY_EXIST;
					goto Exit;
				}
				__optarg_result.optarg->exist = TRUE; // set presence flag
				if (__optarg_result.optarg->flag) {
					// get next command line argument for an option parameter, if exist
					if ((i + 1) < argc) {
						++i;
						// check if parameter is no option
						if (optarg_check(argv[i])) {
							__optarg_result.error = E_OPTARG_PARAM_IS_OPTION;
							goto Exit;
						}
						__optarg_result.optarg->value = argv[i];
					}
					else {
						// return error if no parameter found!
						__optarg_result.error = E_OPTARG_NO_PARAM;
						goto Exit;
					}
				}
			}
			else {
				// check for non-option style
				if (*argv[i] == _T('-')) {
					__optarg_result.error = E_OPTARG_INVALID_OPTION;
					__optarg_result.invalid = argv[i];
					goto Exit;
				}

				// save value to optarg list;
				optarg_list_add(argv[i]);
			}
		}
		// cleanup
		__optarg_result.error = E_OPTARG_SUCCESS;
		__optarg_result.invalid = NULL;
		__optarg_result.optarg = NULL;
	}
Exit:
	return;
}

void __cdecl optarg_free(void) {
	optarg_list_free();
}

void __cdecl optarg_help(void) {
	static TCHAR _fmt_short[] = _T("-%s %s");
	static TCHAR _fmt_long[]  = _T("--%s %s");
	static TCHAR _fmt_all[]   = _T("-%s|--%s %s");
	static TCHAR _fmt_prn[]   = _T("\t%%-%ds %%s\n");
	static TCHAR _fmt_prn_buf[ARRAY_SIZE(_fmt_prn) * 2];
	static TCHAR _fmt_buf[OPTARG_BUFF_SIZE];
	size_t i, l, maxlevel = 0, len = 0;

	for (i = 0; i < __optarg_size; i++) {
		l = _tcslen(__optarg[i].name) + _tcslen(__optarg[i].longname) + _tcslen(__optarg[i].param) + 8;
		if (l > len) len = l;
	}

	for (i = 0; i < __optarg_size; i++) {
		if (_tcslen(__optarg[i].name)) {
			if (_tcslen(__optarg[i].longname)) {
				_stprintf_s(_fmt_buf, OPTARG_BUFF_SIZE, _fmt_all,
					__optarg[i].name, __optarg[i].longname, __optarg[i].param);
			}
			else {
				_stprintf_s(_fmt_buf, OPTARG_BUFF_SIZE, _fmt_short,
					__optarg[i].name, __optarg[i].param);
			}
		}
		else {
			if (_tcslen(__optarg[i].longname)) {
				_stprintf_s(_fmt_buf, OPTARG_BUFF_SIZE, _fmt_long,
					__optarg[i].longname, __optarg[i].param);
			}
			else {
				// show next help line
				_fmt_buf[0] = 0;
			}
		}

		_stprintf_s(_fmt_prn_buf, ARRAY_SIZE(_fmt_prn_buf), _fmt_prn, len);
		_tprintf_s(_fmt_prn_buf, _fmt_buf, __optarg[i].help);
	}
}
