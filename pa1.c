/**********************************************************************
 * Copyright (c) 2021-2023
 *  Sang-Hoon Kim <sanghoonkim@ajou.ac.kr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTIABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 **********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

/* To avoid security error on Visual Studio */
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)

/*====================================================================*/
/*          ****** DO NOT MODIFY ANYTHING FROM THIS LINE ******       */
#define MAX_NR_TOKENS 16 /* Maximum length of tokens in a command */
#define MAX_TOKEN_LEN 32 /* Maximum length of single token */
#define MAX_ASSEMBLY 128 /* Maximum length of assembly string */

typedef unsigned char bool;
#define true 1
#define false 0
/*          ****** DO NOT MODIFY ANYTHING UP TO THIS LINE ******      */
/*====================================================================*/

/***********************************************************************
 * translate()
 *
 * DESCRIPTION
 *   Translate assembly represented in @tokens[] into a MIPS instruction.
 *   This translate should support following 13 assembly commands
 *
 *    - add
 *    - addi
 *    - sub
 *    - and
 *    - andi
 *    - or
 *    - ori
 *    - nor
 *    - lw
 *    - sw
 *    - sll
 *    - srl
 *    - sra
 *    - beq
 *    - bne
 *
 * RETURN VALUE
 *   Return a 32-bit MIPS instruction
 *
 */

typedef struct
{
	int type;
	int opcode;
} InstructionInfo;

/* table */
const char *registers[32] = {
		"zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
		"t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
		"s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
		"t8", "t9", "k1", "k2", "gp", "sp", "fp", "ra"};

/* R-format */
const char *r_instructions[] = {"add", "sub", "and", "or", "nor"};
int r_funct[] = {0x20, 0x22, 0x24, 0x25, 0x27};

/* R-Shift */
const char *r_shift_instructions[] = {"sll", "srl", "sra"};
int r_shift_funct[] = {0x00, 0x02, 0x03};

/* I-format */
const char *i_instructions[] = {"addi", "andi", "ori", "lw", "sw", "beq", "bne"};
int i_opcodes[] = {0x08, 0x0c, 0x0d, 0x23, 0x2b, 0x04, 0x05};

InstructionInfo detectType(const char *token)
{
	InstructionInfo info;
	info.type = -1; // 기본값

	for (int i = 0; i < 5; i++)
	{
		if (strcmp(token, r_instructions[i]) == 0)
		{
			info.type = 0; // R-format
			info.opcode = r_funct[i];
			return info;
		}
	}

	for (int i = 0; i < 3; i++)
	{
		if (strcmp(token, r_shift_instructions[i]) == 0)
		{
			info.type = 1; // R-format (Shift)
			info.opcode = r_shift_funct[i];
			return info;
		}
	}

	for (int i = 0; i < 7; i++)
	{
		if (strcmp(token, i_instructions[i]) == 0)
		{
			info.type = 2; // I-format
			info.opcode = i_opcodes[i];
			return info;
		}
	}

	return info;
}

const int getRegisterNum(const char *token)
{
	for (int i = 0; i < 32; i++)
	{
		if (strcmp(token, registers[i]) == 0)
		{
			return i;
		}
	}
	return 0;
}

static unsigned int translate(int nr_tokens, char *tokens[])
{
	unsigned int code = 0;
	InstructionInfo instructionInfo = detectType(tokens[0]);
	switch (instructionInfo.type)
	{
	case 0: // R-format (not Shift)
	{
		int rd = getRegisterNum(tokens[1]);
		int rs = getRegisterNum(tokens[2]);
		int rt = getRegisterNum(tokens[3]);
		code = (instructionInfo.opcode << 0) | (rd << 11) | (rt << 16) | (rs << 21);
		break;
	}
	case 1: // R-format (Shift)
	{
		int rd = getRegisterNum(tokens[1]);
		int rt = getRegisterNum(tokens[2]);
		int shamt = (int)strtol(tokens[3], NULL, 0);
		code = (instructionInfo.opcode << 0) | (shamt << 6) | (rd << 11) | (rt << 16);
		break;
	}
	case 2: // I-format
	{
		int immediate, rt, rs = 0;
		if (instructionInfo.opcode == 0x23 || instructionInfo.opcode == 0x2b) // lw, sw
		{
			rt = getRegisterNum(tokens[1]);
			rs = getRegisterNum(tokens[3]);
			immediate = (int)strtol(tokens[2], NULL, 0);
		}
		else
		{
			rt = getRegisterNum(tokens[1]);
			rs = getRegisterNum(tokens[2]);
			immediate = (int)strtol(tokens[3], NULL, 0);
		}
		if (immediate < 0)
		{
			immediate = (immediate & 0xFFFF);
		}

		code = (immediate << 0) | (rt << 16) | (rs << 21) | (instructionInfo.opcode << 26);

		break;
	}
	default:
		printf("wrong command");
		break;
	}

	return code;
}

/***********************************************************************
 * parse_command()
 *
 * DESCRIPTION
 *   Parse @assembly, and put each assembly token into @tokens[] and the number
 *   of tokes into @nr_tokens. You may use this implemention or your own
 *   from PA0.
 *
 *   A assembly token is defined as a string without any whitespace (i.e., space
 *   and tab in this programming assignment). For exmaple,
 *     command = "  add t1   t2 s0 "
 *
 *   then, nr_tokens = 4, and tokens is
 *     tokens[0] = "add"
 *     tokens[1] = "t0"
 *     tokens[2] = "t1"
 *     tokens[3] = "s0"
 *
 *   You can assume that the characters in the input string are all lowercase
 *   for testing.
 *
 *
 * RETURN VALUE
 *   Return 0 after filling in @nr_tokens and @tokens[] properly
 *
 */
static int parse_command(char *assembly, int *nr_tokens, char *tokens[])
{
	char *curr = assembly;
	int token_started = false;
	*nr_tokens = 0;

	while (*curr != '\0')
	{
		if (isspace(*curr))
		{
			*curr = '\0';
			token_started = false;
		}
		else
		{
			if (!token_started)
			{
				tokens[*nr_tokens] = curr;
				*nr_tokens += 1;
				token_started = true;
			}
		}
		curr++;
	}

	return 0;
}

/*====================================================================*/
/*          ****** DO NOT MODIFY ANYTHING BELOW THIS LINE ******      */

/***********************************************************************
 * The main function of this program.
 */
int main(int argc, char *const argv[])
{
	char assembly[MAX_ASSEMBLY] = {'\0'};
	FILE *input = stdin;

	if (argc > 1)
	{
		input = fopen(argv[1], "r");
		if (!input)
		{
			fprintf(stderr, "No input file %s\n", argv[0]);
			return EXIT_FAILURE;
		}
	}

	if (input == stdin)
	{
		printf("*********************************************************\n");
		printf("*          >> SCE212 MIPS translator  v0.10 <<          *\n");
		printf("*                                                       *\n");
		printf("*                                       .---.           *\n");
		printf("*                           .--------.  |___|           *\n");
		printf("*                           |.------.|  |=. |           *\n");
		printf("*                           || >>_  ||  |-- |           *\n");
		printf("*                           |'------'|  |   |           *\n");
		printf("*                           ')______('~~|___|           *\n");
		printf("*                                                       *\n");
		printf("*                                   Fall 2023           *\n");
		printf("*********************************************************\n\n");
		printf(">> ");
	}

	while (fgets(assembly, sizeof(assembly), input))
	{
		char *tokens[MAX_NR_TOKENS] = {NULL};
		int nr_tokens = 0;
		unsigned int instruction;

		for (size_t i = 0; i < strlen(assembly); i++)
		{
			assembly[i] = tolower(assembly[i]);
		}

		if (parse_command(assembly, &nr_tokens, tokens) < 0)
			continue;

		instruction = translate(nr_tokens, tokens);

		fprintf(stderr, "0x%08x\n", instruction);

		if (input == stdin)
			printf(">> ");
	}

	if (input != stdin)
		fclose(input);

	return EXIT_SUCCESS;
}
