// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#include "as_utility.h"

char* read_file(const char* filename, size_t* size)
{
	FILE* file = fopen(filename, "rb");
	if (!file)
	{
		return NULL;
	}

	fseek(file, 0, SEEK_END);
	*size = ftell(file);
	fseek(file, 0, SEEK_SET);

	char* content = (char*)malloc(*size + 1);
	if (content)
	{
		fread(content, 1, *size, file);
		content[*size] = '\0';
	}

	fclose(file);
	return content;
}
