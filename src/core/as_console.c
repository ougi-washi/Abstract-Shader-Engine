// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#include "core/as_console.h"
#include "as_types.h"
#include "as_memory.h"

#define AS_COMMAND_MAX_LENGTH			512
#define AS_COMMAND_ARG_MAX_LENGTH		512
#define AS_COMMAND_MAX_ARGS				3

#define AS_COMMAND_SEPARATION_LINE		30
#define AS_COMMAND_INPUT_LINE			32

#ifdef _WIN32
#include <conio.h>
#define CLEAR_SCREEN "cls"
#else
#include <unistd.h>
#define CLEAR_SCREEN "clear"
#endif

typedef struct as_console_command
{
	char command[AS_COMMAND_MAX_LENGTH];
	char argument[AS_COMMAND_ARG_MAX_LENGTH][AS_COMMAND_MAX_ARGS];
} as_console_command;

as_command_mapping* as_console_find_command_mapping(as_console* console, const char* command_name)
{
	AS_ASSERT(console, "Cannot find command mapping, invalid console");
	for (sz i = 0; i < AS_ARRAY_GET_SIZE(console->mappings); ++i)
	{
		as_command_mapping* mapped_cmd = AS_ARRAY_GET(console->mappings, i);
		if (!mapped_cmd || !mapped_cmd->func) continue;

		if (strcmp(command_name, mapped_cmd->name) == 0)
		{
			return mapped_cmd;
		}
	}
	return NULL;
}

void as_console_execute_command(as_console* console, as_console_command* cmd)
{
	AS_ASSERT(console, "Cannot execute command, invalid console");
	if (!cmd)
	{
		AS_LOG(LV_WARNING, "Cannot execute command, invalid cmd");
		return;
	}
	
	for (sz i = 0; i < AS_ARRAY_GET_SIZE(console->mappings); ++i) 
	{
		as_command_mapping* mapped_cmd = AS_ARRAY_GET(console->mappings, i);
		if (!mapped_cmd || !mapped_cmd->func) continue;

		if (strcmp(cmd->command, mapped_cmd->name) == 0)
		{
			if (mapped_cmd->arg_count == 0) 
			{
				mapped_cmd->func(NULL, NULL, NULL);
			}
			else if (mapped_cmd->arg_count == 1)
			{
				mapped_cmd->func(cmd->argument[0], NULL, NULL);
			}
			else if (mapped_cmd->arg_count == 2)
			{
				mapped_cmd->func(cmd->argument[0], cmd->argument[1], NULL);
			}
			else if (mapped_cmd->arg_count == 3)
			{
				mapped_cmd->func(cmd->argument[0], cmd->argument[1], cmd->argument[2]);
			}
			return;
		}
	}
	AS_FLOG(LV_LOG, "Unknown command: %s\n", cmd->command);
}

void* as_console_process_input(void* arg)
{
	if (!arg) { return NULL; }
	as_console* console = (as_console*)arg;
	while (console->is_running) 
	{
		as_console_command cmd = { 0 };
		i32 scanf_val = scanf("%s", cmd.command);

		if (scanf_val == EOF) 
		{
			continue;
		}

		as_command_mapping* mapping_found = as_console_find_command_mapping(console, cmd.command);
		if (mapping_found)
		{
			for (sz i = 0; i < mapping_found->arg_count; ++i)
			{
				scanf_val = scanf("%s", cmd.argument[i]);
			}
			as_console_execute_command(console, &cmd);
		}
		else
		{
			AS_FLOG(LV_LOG, "Unknown command: %s\n", cmd.command);
		}
	}
	return NULL;
}

as_console* as_console_create()
{
	as_console* console = AS_MALLOC_SINGLE(as_console);
	console->is_running = true;
	console->thread = as_thread_create(as_console_process_input, console);
	return console;
}

char get_pressed_key()
{
#ifdef _WIN32
	return _getch();
#else
	int ch = getchar();
	return (ch == EOF) ? EOF : (char)ch;
#endif
}

int is_key_pressed()
{
#ifdef _WIN32
	return _kbhit();
#else
	fd_set read_fds;
	FD_ZERO(&read_fds);
	FD_SET(STDIN_FILENO, &read_fds);

	struct timeval timeout = { 0, 0 };
	return select(STDIN_FILENO + 1, &read_fds, NULL, NULL, &timeout) > 0;
#endif
}

void as_console_destroy(as_console* console)
{
	if (console)
	{
		console->is_running = false;
		as_thread_terminate(console->thread);
		AS_FREE(console);
	}
}

as_command_mapping_128* as_console_get_mappings(as_console* console)
{
	AS_ASSERT(console, "Cannot get mappings, invalid console");
	return &console->mappings;
}

void move_cursor(const i32 x, const i32 y)
{
#ifdef _WIN32
	COORD coord;
	coord.X = x;
	coord.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
#else
	printf("\033[%d;%dH", y + 1, x + 1);
#endif
}



bool handle_special_keys(const i32 key)
{
	//if (key == 8)
	//{
	//	if (console_handle.cursor_position_x > 0)
	//	{
	//		printf("\b \b");
	//		memmove(&current_command[cursor_position - 1], &current_command[cursor_position], strlen(current_command) - cursor_position + 1);
	//		cursor_position--;
	//		move_cursor(AS_COMMAND_INPUT_LINE, cursor_position);
	//		printf("%s", &current_command[cursor_position]);
	//	}
	//	return true;
	//}
 //   if (key == 0 || key == 224 || key == -32)
	//{
	//	const i32 special_key = get_pressed_key();
	//	switch (special_key)
	//	{
	//	case 72: // up
	//		break;

	//	case 80: // down
	//		break;
	//	case 77: // right
	//		if (cursor_position < strlen(current_command)) 
	//		{
	//			cursor_position++;
	//			move_cursor(AS_COMMAND_INPUT_LINE, cursor_position);
	//		}
	//		break;
	//	case 75: // left
	//		if (cursor_position > 0) 
	//		{
	//			cursor_position--;
	//			move_cursor(AS_COMMAND_INPUT_LINE, cursor_position);
	//		}
	//		break;
	//	default:
	//		printf("%d", special_key);
	//		break;
	//		/* ... etc ... */
	//	}
	//	return true;
	//}
	return false;
}

