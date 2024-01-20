/*
 * console.c
 *
 *  Created on: 25 Nov 2021
 *      Author: andrew
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "main.h"
#include "console.h"

#include "lfs.h"
#include "fs.h"
#include "gps.h"
#include "clock.h"

typedef void (*pf_command)(const char *command_string);

#define HISTORY_DEPTH 5
static char input[60];
static char input_history[HISTORY_DEPTH][60];
static int8_t history_index = 0;
static uint32_t inputIndex = 0;

typedef struct _command
{
	const char * const command;
	const char * const help_string;
	const pf_command command_interpreter;
}
command_t;

// forward references
static void console_help_command(const char *command_string);
static void console_clock_command(const char *command_string);
static void console_test_command(const char *command_string);
static void console_info_command(const char *command_string);
static void console_format_command(const char *command_string);
static void console_cat_command(const char *command_string);
static void console_ls_command(const char *command_string);
static void console_rm_command(const char *command_string);
static void console_example_command(const char *command_string);
static void console_gps_command(const char *command_string);

static void console_history_command(const char *command_string);
static bool console_arg_is(const char *name, const char *arg, uint8_t length);
static const char* console_get_parameter( const char *command_string, uint8_t wanted_parameter, uint8_t *parameter_string_length);
static void console_print_array(uint32_t addr, uint8_t data[], uint16_t len);

/** list of commands */
static command_t commands[] =
{
    {"help", "help on commands", console_help_command},
    {"test", "test file system", console_test_command},
	{"clock", "clock get | set [HH:MM:SS DD/MM/YY] set or get the clock", console_clock_command},
    {"info", "info on file system", console_info_command},
    {"format", "format file system", console_format_command},
    {"cat", "read a specified file on flash", console_cat_command},
    {"ls", "list files on flash", console_ls_command},
    {"rm", "remove a specified file on flash", console_rm_command},
	{"gps", "gps fix | debug | standby check if gps fix obtained, set debug or standby modes", console_gps_command},
	{"example", "", console_example_command},
	{"history", "linux-like command history", console_history_command},
};

/** */
void console_gps_command(const char *command_string)
{
    uint8_t arg1_len;
    uint8_t arg2_len;
    const char *arg1 = console_get_parameter(command_string, 1, &arg1_len);
    const char *arg2 = console_get_parameter(command_string, 2, &arg2_len);

    if (console_arg_is("fix", arg1, arg1_len))
    {
    	printf("gps fix %s\n", gps_get_fix_obtained()?"obtained":"not obtained");
    }
    else if (console_arg_is("debug", arg1, arg1_len))
    {
    	if (console_arg_is("on", arg2, arg2_len))
    	{
    		gps_set_debug(true);
    	}
    	else if (console_arg_is("off", arg2, arg2_len))
    	{
    		gps_set_debug(false);
    	}
        else
        {
            printf("debug %s", gps_get_debug()?"on":"off");
        }
    }
    else if (console_arg_is("standby", arg1, arg1_len))
    {
    	if (console_arg_is("on", arg2, arg2_len))
    	{
    		gps_set_standby(true);
    	}
    	else if (console_arg_is("off", arg2, arg2_len))
    	{
    		gps_set_standby(false);
    	}
        else
        {
            printf("standby %s", gps_get_standby()?"on":"off");
        }
    }
    else
    {
        printf("incorrect argument\n");
    }
}

/** */
static void console_help_command(const char *command_string)
{
	for(int idx = 0; idx < sizeof(commands)/sizeof(command_t); idx++)
	{
		printf("%s: %s\n", commands[idx].command, commands[idx].help_string);
	}
}

/** */
static void console_clock_command(const char *command_string)
{
    uint8_t arg1_len;
    uint8_t arg2_len;
    const char *arg1 = console_get_parameter(command_string, 1, &arg1_len);
    const char *arg2 = console_get_parameter(command_string, 2, &arg2_len);

    if (console_arg_is("get", arg1, arg1_len))
    {
        int hours, minutes, seconds, date, month, year;
        get_clock(&hours, &minutes, &seconds, &date, &month, &year);
        printf("The date and time is set to %2.2d:%2.2d:%2.2d on %2.2d/%2.2d/%2.2d\n", hours, minutes, seconds, date, month, year);
    }
    else if (console_arg_is("set", arg1, arg1_len))
    {
        if (arg2)
        {
			unsigned int hours, minutes, seconds, date, month, year;
			if (sscanf(arg2, "%d:%d:%d %d/%d/%d", &hours, &minutes, &seconds, &date, &month, &year) == 6)
			{
				set_clock(hours, minutes, seconds, date, month, year);
				printf("Date and time set to %d:%d:%d %d/%d/%d\n", hours, minutes, seconds, date, month, year);
			}
			else
			{
				printf("Parse error. Enter date and time in the format HH:MM:SS DD/MM/YY\n");
			}
        }
        else
        {
            printf("missing argument");
        }
    }
    else
    {
        printf("incorrect argument\n");
    }
}

/** */
static void console_info_command(const char *command_string)
{
	printf("Number of flash blocks: %ld\n", lfs_cfg.block_count);
	printf("Erase flash block size: %ld bytes\n", lfs_cfg.block_size);
	printf("Flash prog size: %ld bytes\n", lfs_cfg.prog_size);
	printf("--\n");
	uint32_t used = lfs_fs_size(&lfs) * lfs_cfg.block_size;
	uint32_t total = lfs_cfg.block_count * lfs_cfg.block_size;
	printf("total bytes = %ld, used bytes = %ld free bytes = %ld\n", total, used, total - used );
}

/** */
static void console_format_command(const char *command_string)
{
	printf("are you sure (y/n)\n");

	char ch = getchar();

	if(ch == 'y')
	{
		printf("\nreformatting\n");
	}

	if (lfs_format(&lfs, &lfs_cfg) == 0)
	{
		printf("formatting complete\n");

		int err = lfs_mount(&lfs, &lfs_cfg);

		if (err < 0)
		{
			printf("could not mount filesystem\n");
		}
	}
	else
	{
		printf("formatting failed\n");
	}
}

/** */
static void console_test_command(const char *command_string)
{
	lfs_file_t file;
	uint32_t test_count = 0;

	lfs_file_open(&lfs, &file, "test", LFS_O_RDWR | LFS_O_CREAT);
	lfs_file_read(&lfs, &file, &test_count, sizeof(test_count));

	test_count += 1;
	lfs_file_rewind(&lfs, &file);
	lfs_file_write(&lfs, &file, &test_count, sizeof(test_count));
	lfs_file_close(&lfs, &file);

	printf("test_count: %ld\n", test_count);
}

/** */
static void console_ls_command(const char *command_string)
{
	lfs_dir_t dir;
	struct lfs_info info;

	lfs_dir_open(&lfs, &dir, ".");

	while(lfs_dir_read(&lfs, &dir, &info) > 0)
	{
		printf("%s\t\t%ld\n", info.name, info.size);
	}

	lfs_dir_close(&lfs, &dir);
}

/** */
static void console_rm_command(const char *command_string)
{
    uint8_t arg1_len;
    const char *arg1 = console_get_parameter(command_string, 1, &arg1_len);

    if(arg1 !=NULL)
    {
		if(lfs_remove(&lfs, arg1) >=0)
		{
			printf("file %s removed\n", arg1);
		}
		else
		{
			printf("error removing file %s\n", arg1);
		}
    }
    else
    {
    	printf("no file name specified\n");
    }
}

/** */
static void console_cat_command(const char *command_string)
{
    uint8_t arg1_len;
    uint8_t arg2_len;
    const char *arg1 = console_get_parameter(command_string, 1, &arg1_len);
    const char *arg2 = console_get_parameter(command_string, 2, &arg2_len);

	lfs_file_t file;

    if(arg1 != NULL)
    {
    	char filename[32];
    	memset(filename,0, 32);
    	strncpy(filename,arg1, arg1_len);
		int err = lfs_file_open(&lfs, &file, filename, LFS_O_RDWR);

		if(err < 0)
		{
			printf("could not open file\n");
			return;
		}

		while(true)
		{
			unsigned char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			int read = lfs_file_read(&lfs, &file, buffer, sizeof(buffer));

			if(read <= 0 )
			{
				break;
			}

			if(console_arg_is("binary", arg2, arg2_len))
			{
				console_print_array(0, buffer, read);
			}
			else
			{
				printf("%.*s", read, buffer);
			}
		}

		lfs_file_close(&lfs, &file);
		printf("\n"); // <<<<<<<<<<<<<<<<<<<<<<<<<<<<< remove ??
    }
    else
    {
    	printf("no file name specified\n");
    }
}

/** */
void console_example_command(const char *command_string)
{
    uint8_t arg1_len;
    uint8_t arg2_len;
    const char *arg1 = console_get_parameter(command_string, 1, &arg1_len);
    const char *arg2 = console_get_parameter(command_string, 2, &arg2_len);

    if (console_arg_is("get", arg1, arg1_len))
    {
    }
    else if (console_arg_is("set", arg1, arg1_len))
    {
        if (arg2)
        {
        }
        else
        {
            printf("missing argument");
        }
    }
    else
    {
        printf("incorrect argument\n");
    }
}

/** */
static void put_history(char *input)
{
    strcpy(input_history[history_index], input);

    if (++history_index == HISTORY_DEPTH)
    {
        history_index = 0;
    }
}

/** */
static void get_history(char *input, bool reset)
{
    static int8_t index = 0;
    if (reset)
    {
        index = history_index -1;

        if (index < 0)
        {
            index = HISTORY_DEPTH;
        }
        return;
    }

    strcpy(input, input_history[index]);

    if (--index < 0)
    {
        index = HISTORY_DEPTH;
    }
}

/** */
static void console_history_command(const char *command_string)
{
    (void)command_string;

    for (uint8_t i = 0; i < HISTORY_DEPTH; i++)
    {
        printf("%d %s\n", i + 1, input_history[i]);
    }
}

/** */
static void console_print_array(uint32_t addr, uint8_t data[], uint16_t len)
{
    uint8_t str[17];

    for (int i = 0; i < len; i += 16)
    {
        uint16_t bytes_per_line = 16;

        if ((len - i) < 16)
        {
            bytes_per_line = len - i;
        }

        for (int j = 0; j < bytes_per_line; j++)
        {
            uint8_t ch = data[i + j];
            if (ch >= 0x20 && ch < 0x7f)
            {
                str[j] = ch;
            }
            else
            {
                str[j] = '.';
            }
        }

        str[bytes_per_line] = '\0';

        printf("%8.8lx  ", addr + i);
        for (int j = 0; j < bytes_per_line; j++)
        {
            printf("%2.2x ", data[i + j]);
        }

        printf("|%s|\n", str);
    }
}

/** */
static bool console_arg_is(const char *name, const char *arg, uint8_t arg_len)
{
    return arg != NULL && arg_len == strlen(name) && strncmp(name, arg, arg_len) == 0;
}

/** */
static const char* console_get_parameter( const char *command_string, uint8_t wanted_parameter, uint8_t *parameter_string_length)
{
	uint8_t parameters_found = 0;
	const char *return_parameter = NULL;

	*parameter_string_length = 0;

	while(parameters_found < wanted_parameter)
	{
		/* Index the character pointer past the current word.  If this is the start
		of the command string then the first word is the command itself. */
		while( ( ( *command_string ) != 0x00 ) && ( ( *command_string ) != ' ' ) )
		{
			command_string++;
		}

		/* Find the start of the next string. */
		while( ( ( *command_string ) != 0x00 ) && ( ( *command_string ) == ' ' ) )
		{
			command_string++;
		}

		/* Was a string found? */
		if( *command_string != 0x00 )
		{
			/* Is this the start of the required parameter? */
			parameters_found++;

			if( parameters_found == wanted_parameter )
			{
				/* How long is the parameter? */
				return_parameter = command_string;
				while( ( ( *command_string ) != 0x00 ) && ( ( *command_string ) != ' ' ) )
				{
					( *parameter_string_length )++;
					command_string++;
				}

				if( *parameter_string_length == 0 )
				{
					return_parameter = NULL;
				}

				break;
			}
		}
		else
		{
			break;
		}
	}

	return return_parameter;
}

/** */
static void process_command(const char * const input)
{
	for(int idx = 0; idx < sizeof(commands)/sizeof(command_t); idx++)
	{
		if(strncmp( commands[idx].command, input, strlen(commands[idx].command)) == 0)
		{
			commands[idx].command_interpreter(input);
			return;
		}
	}

	printf("unrecognised command %s\n", input);
}

/** */
void console_init(void)
{
    memset(input, 0, 60);
    printf("$ ");
}

/** */
void console_process_key(void)
{
    if(kbhit())
    {
    	int8_t ch = getchar();

        // look for up arrow key
        if ((ch == 0x1b) || (ch == 0x5b))
        {
           	ch = getchar();
        	HAL_Delay(1);
			ch = getchar();
        	HAL_Delay(1);

        	// clear the line to remove the uparrow
            printf(CLL);
            get_history(input,false);
            inputIndex = strlen(input);
            printf("\r$ %s", input);
            return;
        }
        else
		{
        	putchar(ch);
		}

        if (ch == '\r')
        {
            printf("\n");

            if (inputIndex == 0)
            {
                printf("$ ");
                return;
            }

			process_command(input);

            if(strcmp(input, "history")!=0)
            {
                put_history(input);
            }

            get_history(NULL, true);

            inputIndex = 0;
            memset(input, 0x00, sizeof(input));
            printf("\n$ ");
        }
        else
        {
            if (ch == '\n')
            {
                // ignore carriage returns
            }
            else if (ch == '\b')
            {
                // process backspace
                if (inputIndex > 0)
                {
                    inputIndex--;
                    input[inputIndex] = '\0';
                }

                printf(CLL "\r$ %s", input);
            }
            else if (ch >= ' ' && ch <= '~')
            {
                // save printable characters
                if (inputIndex < sizeof(input))
                {
                    input[inputIndex] = ch;
                    inputIndex++;
                }
            }
        }
    }
}
