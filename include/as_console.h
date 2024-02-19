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

extern void process_console_command(const char *command)
{
    // // Process different commands
    // if (strncmp(command, "add_object", strlen("add_object")) == 0)
    // {
    //     // Extract parameters from the command if needed
    //     // ...
    //     // Call the appropriate function from your engine
    //     as_object *newObject = as_object_create(/* parameters */);
    //     printf("Object added with ID %d\n", newObject->id);
    // }
    // else if (strncmp(command, "select_object", strlen("select_object")) == 0)
    // {
    //     // Extract parameters from the command if needed
    //     // ...
    //     // Call the appropriate function from your engine
    //     as_object *selectedObject = /* ... */;
    //     printf("Object selected with ID %d\n", selectedObject->id);
    // }
    // else if (strncmp(command, "remove_object", strlen("remove_object")) == 0)
    // {
    //     // Extract parameters from the command if needed
    //     // ...
    //     // Call the appropriate function from your engine
    //     as_object *objectToRemove = /* ... */;
    //     printf("Object removed with ID %d\n", objectToRemove->id);
    // }
    // else if (strncmp(command, "attach_camera", strlen("attach_camera")) == 0)
    // {
    //     // Extract parameters from the command if needed
    //     // ...
    //     // Call the appropriate function from your engine
    //     as_camera *camera = /* ... */;
    //     printf("Camera attached\n");
    // }
    // else if (strncmp(command, "assign_shader", strlen("assign_shader")) == 0)
    // {
    //     // Extract parameters from the command if needed
    //     // ...
    //     // Call the appropriate function from your engine
    //     as_shader *shader = /* ... */;
    //     printf("Shader assigned\n");
    // }
    // else if (strncmp(command, "assign_texture", strlen("assign_texture")) == 0)
    // {
    //     // Extract parameters from the command if needed
    //     // ...
    //     // Call the appropriate function from your engine
    //     as_texture *texture = /* ... */;
    //     printf("Texture assigned\n");
    // }
    // else 
    if (strncmp(command, "exit", strlen("exit")) == 0)
    {
        printf("Exiting program\n");
        // Cleanup and exit the program if needed
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

static char current_command[MAX_COMMAND_LENGTH];
static int cursor_position = 0;
static bool is_init_command = false;

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

void move_cursor(int col)
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

bool handle_special_keys()
{
	int ch;

	while ((ch = _getch()) != 27) /* 27 = Esc key */
	{
		printf("%d", ch);
		if (ch == 0 || ch == 224)
			printf(", %d", _getch());
		printf("\n");
	}

	char key = get_pressed_key();
    if (key == 91)
    {
		key = get_pressed_key();
    }
	switch (key) {
	case 27: // Escape key
		// Handle escape key
		break;
	case 65: // Up arrow key
		// Handle up arrow key
		break;
	case 66: // Down arrow key
		// Handle down arrow key
		break;
	case 68: // Left arrow key
		// Handle left arrow key
		if (cursor_position > 0) {
			cursor_position--;
			move_cursor(cursor_position);
		}
		break;
	case 67: // Right arrow key
		// Handle right arrow key
		if (cursor_position < strlen(current_command)) {
			cursor_position++;
			move_cursor(cursor_position);
		}
		break;
	case 8: // Backspace (Delete)
		// Handle delete key
		if (cursor_position > 0) {
			printf("\b \b");
			memmove(&current_command[cursor_position - 1], &current_command[cursor_position], strlen(current_command) - cursor_position + 1);
			cursor_position--;
			move_cursor(cursor_position);
			printf("%s", &current_command[cursor_position]);
		}
		break;
	default:
		// Handle other keys
        return false;
        break;
	}
    return true;
}

void as_console_process_input()
{
	if (is_key_pressed()) {
        if (handle_special_keys())
        {
            return;
        }
		char key = get_pressed_key();
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