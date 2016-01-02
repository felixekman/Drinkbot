#ifndef __CONSOLE_H__
#define __CONSOLE_H__

/*#define DEBUG_CONSOLE*/
#define CMD_CHAR_BUF_SIZE	32

struct cmd_char_stack {
	int nc;
	int nc_tot;
	struct list_head s;
};

struct cmd_char {
	char c;
	struct list_head list;
};

struct console_cmd {
	const struct cmd_handle *hcmd_arr;
	struct cmd_char_stack ccs;
	struct cmd_char cbuf[CMD_CHAR_BUF_SIZE];
	char args[CMD_CHAR_BUF_SIZE];
	int index;
};

#endif /* __CONSOLE_H__ */
