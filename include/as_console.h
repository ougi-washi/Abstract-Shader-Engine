// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#pragma once
#include "as_types.h"

#ifdef _WIN32
#include <conio.h>
#define CLEAR_SCREEN "cls"
#else
#include <unistd.h>
#define CLEAR_SCREEN "clear"
#endif

#define MAX_COMMAND_LENGTH 256
static char current_command[MAX_COMMAND_LENGTH];
static int cursor_position = 0;
static bool is_init_command = false;

extern void process_console_command(const char *command)
{
    if (strncmp(command, "exit", strlen("exit")) == 0)
    {
        printf("Exiting program\n");
        // TODO: Cleanup 
    }
    else
    {
        printf("Unknown command: %s\n", command);
    }
}

const char* available_commands[] = {
    "add_object",
    "select_object",
    "remove_object",
    "attach_camera",
    "assign_shader",
    "assign_texture",
    "exit",
    NULL
};
void auto_fill(const char* input)
{
    if (strlen(input) == 0) {
        return;
    }

    bool found = false;
    for (int i = 0; available_commands[i] != NULL; ++i) {
        if (strncmp(input, available_commands[i], strlen(input)) == 0) {
            if (!found) {
                printf("\nAuto-fill suggestions:\n");
                found = true;
            }
            printf("%s\n", available_commands[i]);
        }
    }
    if (found) {
        printf("\n");
    }
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

char get_pressed_key()
{
#ifdef _WIN32
	return _getch();
#else
	int ch = getchar();
	return (ch == EOF) ? EOF : (char)ch;
#endif
}

void move_cursor(i32 col)
{
#ifdef _WIN32
	COORD coord;
	coord.X = col;
	coord.Y = 0;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
#else
	printf("\033[%dG", col + 1);
#endif
}

bool handle_special_keys(const i32 key)
{
	if (key == 8)
	{
		if (cursor_position > 0)
		{
			printf("\b \b");
			memmove(&current_command[cursor_position - 1], &current_command[cursor_position], strlen(current_command) - cursor_position + 1);
			cursor_position--;
			move_cursor(cursor_position);
			printf("%s", &current_command[cursor_position]);
		}
		return true;
	}
    if (key == 0 || key == 224 || key == -32)
	{
		const i32 special_key = get_pressed_key();
		switch (special_key)
		{
		case 72: // up
			break;

		case 80: // down
			break;
		case 77: // right
			if (cursor_position < strlen(current_command)) 
			{
				cursor_position++;
				move_cursor(cursor_position);
			}
			break;
		case 75: // left
			if (cursor_position > 0) 
			{
				cursor_position--;
				move_cursor(cursor_position);
			}
			break;
		default:
			printf("%d", special_key);
			break;
			/* ... etc ... */
		}
		return true;
	}
	return false;
}

void as_console_process_input()
{
	if (is_key_pressed())
	{
		const i32 key = get_pressed_key();
		if (handle_special_keys(key))
        {
            return;
        }
		if (key == '\r') {
			// Enter key pressed, process the command
			process_console_command(current_command);
			memset(current_command, 0, sizeof(current_command));  // Clear the command buffer
			cursor_position = 0;
			system(CLEAR_SCREEN);  // Clear the screen after processing the command
		}
		else if (strlen(current_command) < MAX_COMMAND_LENGTH - 1) {
			// Insert the pressed key at the current cursor position
			memmove(&current_command[cursor_position + 1], &current_command[cursor_position], strlen(current_command) - cursor_position + 1);
			current_command[cursor_position] = key;
			cursor_position++;
			printf("%c", key);
			move_cursor(cursor_position);
		}
	}
	else {
		
	}
}