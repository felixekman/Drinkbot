#include <stddef.h>
#include <stdarg.h>
#include "list.h"
#include "console.h"

#define MAX_ARGV_ENTRIES	4

typedef int (*cmd_fn_type)(int argc, char **argv);
struct cmd_handle {
	char *cmd_word;
	cmd_fn_type cmd_fn;
};

static struct console_cmd concmd;
static char *argv[MAX_ARGV_ENTRIES];
static int argc;

static int cmd_help(int argc, char **argv);
static int cmd_debug(int argc, char **argv);

const struct cmd_handle hcmd_arr[] = {
	{
		"help",
		cmd_help,
	},
	{
		"?",
		cmd_help,
	},
	{
		"debug",
		cmd_debug,
	},
	{
		NULL,
		NULL,
	}
};

static void os_printf(char *fmt, ... )
{
	char buf[128];
	va_list args;
	va_start(args, fmt );
	vsnprintf(buf, 128, fmt, args);
	va_end(args);
	Serial.print(buf);
}

void commands_register(struct console_cmd *conc)
{
	conc->hcmd_arr = hcmd_arr;
}

static int cmd_help(int argc, char **argv)
{
	const struct cmd_handle *c;
	os_printf("available commands:\n\r");
	for (c = &hcmd_arr[0]; c->cmd_word != NULL; c++)
		os_printf("\t%s\n\r", c->cmd_word);
	return 0;
}

static int cmd_debug(int argc, char **argv)
{
	os_printf("%s\n\r", __func__);
	return 0;
}


static void __cmd_char_push(struct console_cmd *conc, char ch)
{
	struct cmd_char *cc;

	if (conc->ccs.nc >= 0 && conc->ccs.nc < CMD_CHAR_BUF_SIZE) {
		cc = &conc->cbuf[conc->ccs.nc];
		cc->c = ch;
		list_add(&cc->list, &conc->ccs.s);
		conc->ccs.nc++;
	}
	conc->ccs.nc_tot++;
}

static void __cmd_char_pop(struct console_cmd *conc)
{
	struct cmd_char *cc;

	if (conc->ccs.nc_tot > conc->ccs.nc) {
		conc->ccs.nc_tot--;
		return;
	}
	if (conc->ccs.nc > 0 && conc->ccs.nc <= CMD_CHAR_BUF_SIZE) {
		cc = &conc->cbuf[conc->ccs.nc - 1];
		cc->c = 0;
		list_del(&cc->list);
		conc->ccs.nc--;
	}
	if (conc->ccs.nc_tot > 0)
		conc->ccs.nc_tot--;
}

static void __cmd_char_stack_flush(struct console_cmd *conc)
{
	struct cmd_char *cc, *cc_tmp;
	list_for_each_entry_safe(cc, cc_tmp, &conc->ccs.s, list) {
		list_del(&cc->list);
		cc->c = 0;
	}
	conc->ccs.nc = 0;
	conc->ccs.nc_tot = 0;
	memset(conc->cbuf, 0, sizeof(conc->cbuf));
}

static int parse_cmd(struct console_cmd *conc)
{
	char str[32] = { 0 };
	char *args = NULL;
	char *tok = NULL;
	static char *cmd_saveptr = NULL;
	int i;

	for (i = 0; i < conc->ccs.nc; i++)
		str[i] = conc->cbuf[i].c;

	tok = strtok_r(str, " ", &cmd_saveptr);

	for (i = 0; conc->hcmd_arr[i].cmd_word != NULL; i++) {
		if (strcmp(tok, conc->hcmd_arr[i].cmd_word) == 0) {
			conc->index = i;
			break;
		}
	}
	if (conc->index < 0) {
		os_printf("%s: command not found\n\r", tok);
		return 0;
	}

	memset(argv, 0, MAX_ARGV_ENTRIES * sizeof(char *));
	argv[0] = conc->hcmd_arr[conc->index].cmd_word;

	args = strtok_r(NULL, "\r", &cmd_saveptr);

	memset(conc->args, 0, CMD_CHAR_BUF_SIZE);
	i = 0;
	if (args != NULL) {
		strcpy(conc->args, args);
		cmd_saveptr = NULL;
		tok = strtok_r((char *)conc->args, " ", &cmd_saveptr);
		while (tok != NULL) {
			os_printf("argv[%d]: %s\n\r", ++i, tok);
			argv[i] = tok;
			if (i == MAX_ARGV_ENTRIES - 1)
				break;
			tok = strtok_r(NULL, " ", &cmd_saveptr);
		}
	}
	argc = i + 1;
	if (argc > 1)
		os_printf("argc: %d\n\r", argc);

	return strchr(args, '&') == NULL ? 0 : 1;
}

#ifdef DEBUG_CONSOLE
static int process_enter(struct console_cmd *conc)
{
	struct cmd_char *cc;
	list_for_each_entry_reverse(cc, &conc->ccs.s, list) {
		os_printf("%c", cc->c);
	}
	os_printf("\n\rnc: %d, tot: %d\n\r", conc->ccs.nc, conc->ccs.nc_tot);
	__cmd_char_stack_flush(conc);
	os_printf("$ ");
	return 0;
}
#else
static int process_enter(struct console_cmd *conc)
{
	int backend = 0;
	int ret;

	conc->index = -1;
	if (conc->ccs.nc > 0)
		backend = parse_cmd(conc);

	if (conc->index < 0)
		goto finish_enter;

	ret = conc->hcmd_arr[conc->index].cmd_fn(argc, argv);

	/* TODO Distinguish different kinds of errors per ret value */
	if (ret < 0) {
		os_printf("%s: exec failed %d\n\r",
				conc->hcmd_arr[conc->index].cmd_word, ret);
	}

finish_enter:
	__cmd_char_stack_flush(conc);
	Serial.print("$ ");

	return 0;
}
#endif /* DEBUG_CONSOLE */


void setup() {
	Serial.begin(9600);
	memset(&concmd, 0, sizeof(struct console_cmd));
	INIT_LIST_HEAD(&concmd.ccs.s);
	commands_register(&concmd);
	Serial.println("\n\rconsole starts...");
	Serial.println("Please press ENTER to activate it.");
}

void loop() {
	struct console_cmd *conc = &concmd;
	while (Serial.available()) {
		char ch = (char)Serial.read();
		switch (ch) {
		case '\r':
			Serial.print('\n');
			Serial.print('\r');
			process_enter(conc);
			break;
		case '\b':
			if (conc->ccs.nc != 0) {
				Serial.print('\b');
				Serial.print(' ');
				Serial.print('\b');
				__cmd_char_pop(conc);
			}
			break;
		default:
			Serial.print(ch);
			__cmd_char_push(conc, ch);
			break;
		}
	}
	delay(10);
}
