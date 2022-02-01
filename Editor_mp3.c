#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#pragma pack(push, 1)

typedef struct
{
	char file_identifier[3];
	char version[2];
	char flags;
	char size[4];
} header;

typedef struct
{
	char frame_id[4];
	char size[4];
	char flags[2];
	char text_encoding;
	char *information;
} frame;

int get_size(char *size)
{
	return (size[0] << 21) | (size[1] << 14) | (size[2] << 7) | (size[3]);
}

void make_size(int size, char *frame_size)
{
	frame_size[3] = size & 127;
	frame_size[2] = (size >> 7) & 127;
	frame_size[1] = (size >> 14) & 127;
	frame_size[0] = (size >> 21) & 127;
}

void printf_header(header header_mp3)
{
	int size_mp3 = get_size(header_mp3.size);
	printf("ID3v2 header\nfile identifier: %c%c%c\n", header_mp3.file_identifier[0],
		   header_mp3.file_identifier[1], header_mp3.file_identifier[2]);
	printf("version: %d.%d\n", (int)header_mp3.version[0], (int)header_mp3.version[1]);
	printf("flags: %d\n", (int)header_mp3.flags);
	printf("size: %d\n", size_mp3);
}

void printf_frame(frame frame_mp3)
{
	int size_frame = get_size(frame_mp3.size);
	printf("frame id: %c%c%c%c\n", frame_mp3.frame_id[0],
		   frame_mp3.frame_id[1], frame_mp3.frame_id[2], frame_mp3.frame_id[3]);
	printf("size: %d\n", size_frame);
	printf("flags: %d %d\n", (int)frame_mp3.flags[0], (int)frame_mp3.flags[1]);
	printf("text encoding: %d\n", (int)frame_mp3.text_encoding);
	printf("information: ");
	for (int j = 0; j < size_frame - 1; ++j)
	{
		if (frame_mp3.information[j] == '\0')
			printf(" ");
		else
			printf("%c", frame_mp3.information[j]);
	}
	printf("\n\n");
}

void scanf_frame(FILE *file, frame *frame_mp3)
{
	fread(frame_mp3, 11, 1, file);
	int size_frame = get_size(frame_mp3->size);
	frame_mp3->information = (char *)malloc(size_frame - 1);
	fread(frame_mp3->information, size_frame - 1, 1, file);
}

void write_frame(FILE *file, char *prop_name, char *prop_value)
{
	fwrite(prop_name, 4, 1, file);
	char frame_size[4];
	make_size(strlen(prop_value) + 1, frame_size);
	fwrite(frame_size, 4, 1, file);
	fwrite("\0\0\0", 3, 1, file);
	fwrite(prop_value, strlen(prop_value), 1, file);
}

void make_new_file(FILE *file, char *file_name, char *prop_name, char *prop_value, int new_size, int old_size)
{
	char *new_file_name = (char *)malloc(strlen(file_name) + 5);
	strncpy(new_file_name, file_name, strlen(file_name) - 4);
	new_file_name[strlen(file_name) - 4] = '\0';
	strcat(new_file_name, "_new.mp3");
	FILE *new_file = fopen(new_file_name, "wb+");
	if (new_file == NULL)
	{
		printf("Error of file creating\n");
		exit(1);
	}
	rewind(file);
	char byte[1];
	fread(byte, 1, 1, file);
	for (int i = 0; i < 6; ++i)
	{
		fwrite(byte, 1, 1, new_file);
		fread(byte, 1, 1, file);
	}
	char header_size[4];
	make_size(new_size, header_size);
	fwrite(header_size, 4, 1, new_file);
	fgetc(file);
	fgetc(file);
	fgetc(file);
	int i = 0;
	fread(byte, 1, 1, file);
	while (i < old_size && byte != 0)
	{
		fseek(file, -1, SEEK_CUR);
		frame frame_mp3;
		scanf_frame(file, &frame_mp3);
		int size_frame = get_size(frame_mp3.size);
		char frame_id[5];
		strcpy(frame_id, frame_mp3.frame_id);
		frame_id[4] = '\0';
		if (strcmp(frame_id, prop_name) == 0)
		{
			write_frame(new_file, prop_name, prop_value);
			free(frame_mp3.information);
			break;
		}
		char *buffer = (char *)malloc(10 + size_frame);
		fseek(file, -size_frame - 10, SEEK_CUR);
		fread(buffer, 10 + size_frame, 1, file);
		fwrite(buffer, 10 + size_frame, 1, new_file);
		free(buffer);
		i += 10 + size_frame;
		free(frame_mp3.information);
		fread(byte, 1, 1, file);
	}
	if (i == old_size || byte == 0)
	{
		write_frame(new_file, prop_name, prop_value);
		fseek(file, old_size + 10, SEEK_SET);
	}
	fread(byte, 1, 1, file);
	while (!feof(file))
	{
		fwrite(byte, 1, 1, new_file);
		fread(byte, 1, 1, file);
	}
	fclose(file);
	fclose(new_file);
	remove(file_name);
	rename(new_file_name, file_name);
	free(new_file_name);
}

void show_information(FILE *file)
{
	header header_mp3;
	fread(&header_mp3, 10, 1, file);
	int size_mp3 = get_size(header_mp3.size);
	printf_header(header_mp3);
	printf("\nFrames\n");
	int i = 0;
	char byte = fgetc(file);
	while (i < size_mp3 && byte != 0)
	{
		ungetc(byte, file);
		frame frame_mp3;
		scanf_frame(file, &frame_mp3);
		int size_frame = get_size(frame_mp3.size);
		printf_frame(frame_mp3);
		i += 10 + size_frame;
		free(frame_mp3.information);
		byte = fgetc(file);
	}
}

void get_frame(FILE *file, char *prop_name)
{
	header header_mp3;
	fread(&header_mp3, 10, 1, file);
	int size_mp3 = get_size(header_mp3.size);
	int i = 0;
	char byte = fgetc(file);
	while (i < size_mp3 && byte != 0)
	{
		ungetc(byte, file);
		frame frame_mp3;
		scanf_frame(file, &frame_mp3);
		int size_frame = get_size(frame_mp3.size);
		char frame_id[5];
		strcpy(frame_id, frame_mp3.frame_id);
		frame_id[4] = '\0';
		if (strcmp(frame_id, prop_name) == 0)
		{
			printf_frame(frame_mp3);
			free(frame_mp3.information);
			return;
		}
		i += 10 + size_frame;
		free(frame_mp3.information);
		byte = fgetc(file);
	}
	if (i == size_mp3 || byte == 0)
	{
		printf("This frame was not found\n");
	}
}

void set_frame(FILE *file, char *file_name, char *prop_name, char *prop_value)
{
	header header_mp3;
	fread(&header_mp3, 10, 1, file);
	int size_mp3 = get_size(header_mp3.size);
	int free_size = size_mp3;
	int i = 0;
	char byte = fgetc(file);
	while (i < size_mp3 && byte != 0)
	{
		ungetc(byte, file);
		frame frame_mp3;
		scanf_frame(file, &frame_mp3);
		int size_frame = get_size(frame_mp3.size);
		char frame_id[5];
		strcpy(frame_id, frame_mp3.frame_id);
		frame_id[4] = '\0';
		if (strcmp(frame_id, prop_name) == 0)
		{
			int buffer_size = 0;
			int j = i + 10 + size_frame;
			byte = fgetc(file);
			while (j < size_mp3 && byte != 0)
			{
				ungetc(byte, file);
				frame frame_mp3_;
				scanf_frame(file, &frame_mp3_);
				int size_frame_ = get_size(frame_mp3_.size);
				buffer_size += 10 + size_frame_;
				free(frame_mp3_.information);
				j += 10 + size_frame_;
				byte = fgetc(file);
			}
			int new_size = i + 11 + strlen(prop_value) + buffer_size;
			if (new_size > size_mp3)
			{
				make_new_file(file, file_name, prop_name, prop_value, new_size, size_mp3);
				free(frame_mp3.information);
				return;
			}
			char *buffer = (char *)malloc(buffer_size);
			fseek(file, -buffer_size - 1, SEEK_CUR);
			fread(buffer, buffer_size, 1, file);
			fseek(file, i + 10, SEEK_SET);
			write_frame(file, prop_name, prop_value);
			fwrite(buffer, buffer_size, 1, file);
			for (int j = new_size + 1; j <= size_mp3; ++j)
			{
				fwrite("\0", 1, 1, file);
			}
			free(buffer);
			free(frame_mp3.information);
			return;
		}
		i += 10 + size_frame;
		free_size -= (size_frame + 10);
		free(frame_mp3.information);
		byte = fgetc(file);
	}
	if (i == size_mp3)
	{
		make_new_file(file, file_name, prop_name, prop_value, size_mp3 + 11 + strlen(prop_value), size_mp3);
		return;
	}
	if (byte == 0)
	{
		if (free_size < (11 + strlen(prop_value)))
		{
			make_new_file(file, file_name, prop_name, prop_value, size_mp3 - free_size + 11 + strlen(prop_value), size_mp3);
			return;
		}
		fseek(file, -1, SEEK_CUR);
		write_frame(file, prop_name, prop_value);
	}
}

int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		printf("Error\n");
		return 1;
	}
	char command1[12];
	strncpy(command1, argv[1], 11);
	command1[11] = '\0';
	if (strcmp(command1, "--filepath=") == 0)
	{
		char *file_name = (char *)malloc(strlen(argv[1]) - 10);
		for (int i = 0; i < (strlen(argv[1]) - 10); ++i)
		{
			file_name[i] = argv[1][i + 11];
		}
		FILE *file = fopen(file_name, "rb+");
		if (file == NULL)
		{
			printf("Error of file opening\n");
			return 1;
		}
		if (strcmp(argv[2], "--show") == 0)
		{
			show_information(file);
		}
		else
		{
			char command2[7];
			strncpy(command2, argv[2], 6);
			command2[6] = '\0';
			char prop_name[5];
			for (int i = 0; i < 5; ++i)
			{
				prop_name[i] = argv[2][i + 6];
			}
			prop_name[4] = '\0';
			if (strcmp(command2, "--get=") == 0)
			{
				get_frame(file, prop_name);
			}
			else
			{
				if (strcmp(command2, "--set=") == 0)
				{
					char command3[9];
					strncpy(command3, argv[3], 8);
					command3[8] = '\0';
					if (strcmp(command3, "--value=") == 0)
					{
						char *prop_value = (char *)malloc(strlen(argv[3]) - 7);
						for (int i = 0; i < (strlen(argv[3]) - 7); ++i)
						{
							prop_value[i] = argv[3][i + 8];
						}
						set_frame(file, file_name, prop_name, prop_value);
						free(prop_value);
					}
					else
					{
						printf("Commands cannot be recognized\n");
					}
				}
				else
				{
					printf("Commands cannot be recognized\n");
				}
			}
		}
		fclose(file);
		free(file_name);
	}
	else
	{
		printf("Commands cannot be recognized\n");
	}
	return 0;
}