/*
 * =============================================================================
 *
 *       Filename:  hcalc.c
 *
 *    Description:  Hex Calc
 *
 *        Version:  1.0
 *        Created:  04/04/14 14:32:58
 *    Last Change:  11/24/2020 19:36:59
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  ShadowStar, <orphen.leiliu@gmail.com>
 *   Organization:  Gmail
 *
 * =============================================================================
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <termios.h>

#if defined(__x86_64__)
static inline int arch_clz64(uint64_t x)
{
	if (x == 0)
		return 64;
	__asm__(
	"bsrq	%1, %0"
	: "=r" (x)
	: "0" (x));
	return 63 - x;
}
#elif defined(__i386__)
static inline int arch_clz64(uint64_t x)
{
	if (x == 0)
		return 64;
	union {
		struct {
			uint32_t b;
			uint32_t a;
		} s;
		uint64_t u;
	} v = { .u = x };
	__asm__(
	"cmpl	$0, %0\n\t"
	"je	1f\n\t"
	"bsrl	%1, %0\n\t"
	"addl	$32, %0\n\t"
	"jmp	2f\n\t"
	"1:"
	"bsrl	%2, %0\n\t"
	"2:"
	: "=r" (v.s.a)
	: "0" (v.s.a), "r" (v.s.b));
	return 63 - v.s.a;
}
#elif defined(__mips64)
static inline int arch_clz64(uint64_t x)
{
	__asm__(
	"dclz	%0, %1"
	: "=r" (x)
	: "r" (x));
	return x;
}
#endif

static inline int ___constant_clz64(uint64_t x)
{
	int n;

	if (x == 0)
		return 64;
	n = 1;
	if ((x >> 32) == 0) {
		n += 32;
		x <<= 32;
	}
	if ((x >> 48) == 0) {
		n += 16;
		x <<= 16;
	}
	if ((x >> 56) == 0) {
		n += 8;
		x <<= 8;
	}
	if ((x >> 60) == 0) {
		n += 4;
		x <<= 4;
	}
	if ((x >> 62) == 0) {
		n += 2;
		x <<= 2;
	}
	n -= (x >> 63);
	return n;
}

#define clz64(x) (__builtin_constant_p(x) ?				\
	___constant_clz64(x) : arch_clz64(x))

static char inbuf[4096];
static unsigned int inlen = 0;
static int interactive;

static inline void clear_ibuf(void)
{
	memset(inbuf, 0, sizeof(inbuf));
	inlen = 0;
}

static inline void new_prompt(void)
{
	clear_ibuf();
	fprintf(stdout, "> ");
}

static void show_result(uint64_t result)
{
	int i = clz64(result), show = 0;
	char tmp[73] = { 0 }, *p = tmp;

	if (result == 0) {
		fprintf(stdout, "Bin  0\n");
		fprintf(stdout, "Oct  0\n");
		fprintf(stdout, "Dec  0\n");
		fprintf(stdout, "Hex  0\n");
		return;
	}
	i &= ~7U;
	for (;i < 64; i++) {
		if ((result << i) & (0x1ULL << 63))
			*p++ = '1';
		else
			*p++ = '0';
		if ((i & 7) == 7)
			*p++ = ' ';
	}

	fprintf(stdout, "Bin  %s\n", tmp);
	fprintf(stdout, "Oct  0%llo\n", result);
	fprintf(stdout, "Dec  %lld\n", result);
	if ((int64_t)result < 0) {
		if ((result >> 32) == 0xFFFFFFFF)
			fprintf(stdout, "     %u (u32)\n", (uint32_t)result);
		fprintf(stdout, "     %llu (u64)\n", result);
	}

	fprintf(stdout, "Hex  ");
	for (i = 48; i >= 0; i -= 16) {
		if (show || (result >> i) & 0xFFFF) {
			fprintf(stdout, "%04llX ",
				(result >> i) & 0xFFFF);
			show = 1;
		}
	}
	fprintf(stdout, "\n");
}

static inline void result_prompt(uint64_t result)
{
	int len = inlen;

	if (len)
		show_result(result);
	if (interactive) {
		new_prompt();
		if (len) {
			fprintf(stdout, "0x%llX ", result);
			inlen = sprintf(inbuf, "0x%llX ", result);
		}
	}
}

static inline void get_from_argv(int c, char **v)
{
	clear_ibuf();
	do {
		if (strncmp(*v, "-i", 3) == 0)
			interactive = 1;
		else
			inlen += snprintf(inbuf + inlen, sizeof(inbuf) - inlen,
					  "%s ", *v);
		v++;
	} while (--c && inlen < sizeof(inbuf));
}

static inline uint64_t str2hex(char **in, int *err)
{
	uint64_t rt = 0;

	*err = 1;
	while (isxdigit(**in)) {
		if (rt > (-1ULL / 16)) {
			*err = 1;
			break;
		} else
			*err = 0;
		switch (**in) {
		case '0' ... '9':
			rt = (rt << 4) | (**in - '0');
			break;
		case 'a' ... 'f':
			rt = (rt << 4) | (**in - 'a' + 10);
			break;
		case 'A' ... 'F':
			rt = (rt << 4) | (**in - 'A' + 10);
			break;
		}
		*in += 1;
	}
	return rt;
}

static inline uint64_t str2dec(char **in, int *err)
{
	uint64_t rt = 0;

	*err = 1;
	while (isdigit(**in)) {
		if (rt > (-1ULL / 10)) {
			*err = 1;
			break;
		} else
			*err = 0;
		switch (**in) {
		case '0' ... '9':
			rt = rt * 10 + (**in - '0');
			break;
		}
		*in += 1;
	}
	return rt;
}

static inline uint64_t str2oct(char **in, int *err)
{
	uint64_t rt = 0;

	*err = 1;
	while (isdigit(**in)) {
		if (rt > (-1ULL / 8)) {
			*err = 1;
			break;
		}
		switch (**in) {
		case '0' ... '7':
			*err = 0;
			rt = (rt << 3) | (**in - '0');
			break;
		default:
			*err = 1;
			goto out;
		}
		*in += 1;
	}
out:
	return rt;
}

static inline uint64_t str2bin(char **in, int *err)
{
	uint64_t rt = 0;

	*err = 1;
	while (isdigit(**in)) {
		if (rt > (-1ULL / 2)) {
			*err = 1;
			break;
		}
		if (**in == '0' || **in == '1') {
			*err = 0;
			rt = (rt << 1) | (**in - '0');
		} else {
			*err = 1;
			break;
		}
		*in += 1;
	}
	return rt;
}

static void help_number(void)
{
	fprintf(stdout, "\nSupport number:\n"
		" [bB]XXX         - Binary number\n"
		" [oO]XXX         - Octal number\n"
		" [dD]XXX         - Decimal number\n"
		" [hH]XXX         - Hexadecimal number\n"
		" [0x]XXX         - Hexadecimal number\n"
		" X[A-F]|[a-f]X   - Hexadecimal number\n"
		"                   Add prefix ``0x'' to the Hexadecimal number"
		" which starting with the `[bB]' or `[dD]'\n"
		" 0XXX            - Binary, or Octal if there is digit"
		"in X more than 1\n"
		" OTHERS          - Decimal if it is a number\n\n"
		" Suffix Support:\n"
		" k|K             - Kilo - 2^10\n"
		" m|M             - Mega - 2^20\n"
		" g|G             - Giga - 2^30\n"
		" t|T             - Tera - 2^40\n"
		" p|P             - Peta - 2^50\n"
		);
}

static inline char guess_number(char *p)
{
	char type = 0;

	switch (*p++) {
	case '0':
		type = 1;
		break;
	case '1' ... '9':
		type = 4;
		break;
	case 'a' ... 'f':
	case 'A' ... 'F':
		type = 8;
		break;
	}
	while (isxdigit(*p)) {
		switch (*p) {
		case '2' ... '7':
			if (type == 1)
				type = 2;
			break;
		case '8' ... '9':
			type |= 4;
			break;
		case 'a' ... 'f':
		case 'A' ... 'F':
			type |= 8;
			break;
		}
		p++;
	}

	if (type & 8)
		type = 'h';
	else if (type & 4)
		type = 'd';
	else if (type & 2)
		type = 'o';
	else if (type & 1)
		type = 'b';

	return type;
}

static inline char next_char(char **p)
{
	while (**p && isblank(**p))
		*p += 1;
	return **p;
}

static uint64_t get_number(char **in, int *err)
{
	char type = 0, mod = 0;
	uint64_t res;

	if (next_char(in) == '\0')
		return 0;

	switch (**in) {
	case '-':
	case '+':
	case '~':
		mod = **in;
		*in += 1;
	}
	switch (**in) {
	case 'h':
	case 'H':
		type = 'h';
		(*in)++;
		break;
	case 'o':
	case 'O':
		type = 'o';
		(*in)++;
		break;
	case 'b':
	case 'B':
		if (*((*in) + 1) == '0' || *((*in) + 1) == '1') {
			type = 'b';
			(*in)++;
		} else
			type = 'h';
		break;
	case 'd':
	case 'D':
		if (isdigit(*((*in) + 1))) {
			type = 'd';
			(*in)++;
		} else
			type = 'h';
		break;
	case '0':
		if (*(*in + 1) == 'x') {
			*in += 2;
			type = 'h';
		}
		break;
	}
	if (!type)
		type = guess_number(*in);
	switch (type) {
	case 'h':
		res = str2hex(in, err);
		break;
	case 'o':
		res = str2oct(in, err);
		break;
	case 'b':
		res = str2bin(in, err);
		break;
	case 'd':
		res = str2dec(in, err);
		break;
	default:
		*err = 1;
	}
	if (*err) {
		fprintf(stdout, "Unrecognized Number ``%c''\n", **in);
		return -1UL;
	}
	switch (**in) {
	case 'p':
	case 'P':
		res *= 1024;
	case 't':
	case 'T':
		res *= 1024;
	case 'g':
	case 'G':
		res *= 1024;
	case 'm':
	case 'M':
		res *= 1024;
	case 'k':
	case 'K':
		res *= 1024;
		(*in)++;
	}
	switch (mod) {
	case '-':
		return 0 - res;
	case '~':
		return ~res;
	case '+':
	default:
		return res;
	}
}

static void help_symbol(void)
{
	fprintf(stdout, "\nSupport Symbol:\n"
		" +, add, ADD     - arithmetic Addition     [priority lowest]\n"
		" -, sub, SUB     - arithmetic Subtraction  [priority lowest]\n"
		" *, mul, MUL     - arithmetic Multiply     [priority normal]\n"
		" /, div, DIV     - arithmetic Division     [priority normal]\n"
		" %%, mod, MOD     - arithmetic remainder    [priority normal]\n"
		" &, and, AND     - bitwise logical AND     [priority high]\n"
		" |, or, OR       - bitwise logical OR      [priority high]\n"
		" ^, xor, XOR     - bitwise logical XOR     [priority high]\n"
		"    nor, NOR     - bitwise logical NOR     [priority high]\n"
		" ~, not, NOT     - bitwise Inversion       [priority high]\n"
		"    >>           - logical shift Right     [priority high]\n"
		"    <<           - logical shift Left      [priority high]\n"
		"    ( )          - brackets                [priority highest]\n"
		);
}

static char get_symbol(char **in)
{
	if (next_char(in) == '\0')
		return '\0';

	switch (**in) {
	case '+':
	case '-':
	case '*':
	case '/':
	case '%':
	case '&':
	case '|':
	case '~':
	case '^':
	case '(':
	case ')':
		*in += 1;
		return *(*in - 1);
	case '>':
		if (!memcmp(*in, ">>", 2)) {
			*in += 2;
			return '>';
		}
		break;
	case '<':
		if (!memcmp(*in, "<<", 2)) {
			*in += 2;
			return '<';
		}
		break;
	case 'a':
	case 'A':
		if (!memcmp(*in, "and", 3) || !memcmp(*in, "AND", 3)) {
			*in += 3;
			return '&';
		}
		if (!memcmp(*in, "add", 3) || !memcmp(*in, "ADD", 3)) {
			*in += 3;
			return '+';
		}
		break;
	case 'd':
	case 'D':
		if (!memcmp(*in, "div", 3) || !memcmp(*in, "DIV", 3)) {
			*in += 3;
			return '/';
		}
	case 'm':
	case 'M':
		if (!memcmp(*in, "mul", 3) || !memcmp(*in, "MUL", 3)) {
			*in += 3;
			return '*';
		}
		if (!memcmp(*in, "mod", 3) || !memcmp(*in, "MOD", 3)) {
			*in += 3;
			return '%';
		}
	case 'n':
	case 'N':
		if (!memcmp(*in, "not", 3) || !memcmp(*in, "NOT", 3)) {
			*in += 3;
			return '~';
		}
		if (!memcmp(*in, "nor", 3) || !memcmp(*in, "NOR", 3)) {
			*in += 3;
			return 'v';
		}
		break;
	case 'o':
	case 'O':
		if (!memcmp(*in, "or", 2) || !memcmp(*in, "OR", 2)) {
			*in += 2;
			return '|';
		}
		break;
	case 's':
	case 'S':
		if (!memcmp(*in, "sub", 3) || !memcmp(*in, "SUB", 3)) {
			*in += 3;
			return '-';
		}
		break;
	case 'x':
	case 'X':
		if (!memcmp(*in, "xor", 3) || !memcmp(*in, "XOR", 3)) {
			*in += 3;
			return '^';
		}
		break;
	}
	fprintf(stdout, "Unrecognized Symbol ``%c''\n", **in);
	return -1;
}

static void help(void)
{
	fprintf(stdout, "\nUsage: hcalc [-i] [ EXPRESSION ]\n"
		">>> No Floating-Point Support <<<\n\n"
		"Option:\n"
		"    -i           - Interactive\n\n"
		"Binding Key:\n"
		" q, Q, <CTRL-D>  - Quit\n"
		" r, R, <ESC>     - Clear current line\n"
		" =, <Enter>      - Do calculate\n"
		"    ?            - Show this help message\n"
		);
	help_number();
	help_symbol();
	fprintf(stdout, "\nVersion: "VER"\n");
}

static uint64_t do_expression(char **p, int *perr, int bracket)
{
	int err = 0;
	char s0, sub = 0;
	uint64_t a = 0, b, tmp;

	*perr = 0;
	switch (next_char(p)) {
	case '\0':
		return 0;
	case '(':
		*p += 1;
		a = do_expression(p, &err, 1);
		break;
	default:
		a = get_number(p, &err);
	}
	if (err) {
err:
		*perr = 1;
		return -1UL;
	}
reget:
	s0 = get_symbol(p);
	switch (s0) {
	case '(':
		b = do_expression(p, &err, 1);
		if (err)
			goto err;
		a *= b;
		break;
	case ')':
		if (sub) {
			a = tmp - a;
			sub = 0;
		}
		if (bracket)
			return a;
		break;
	case '+':
		if (sub) {
			a = tmp - a;
			sub = 0;
		}
		b = do_expression(p, &err, 1);
		if (err)
			goto err;
		a += b;
		break;
	case '-':
		if (sub) {
			a = tmp - a;
			sub = 0;
		}
		tmp = a;
		if (next_char(p) == '(')
			a = do_expression(p, &err, 1);
		else
			a = get_number(p, &err);
		if (err)
			goto err;
		sub = 1;
		break;
	case '*':
		if (next_char(p) == '(')
			b = do_expression(p, &err, 1);
		else
			b = get_number(p, &err);
		if (err)
			goto err;
		a *= b;
		break;
	case '/':
		if (next_char(p) == '(')
			b = do_expression(p, &err, 1);
		else
			b = get_number(p, &err);
		if (err)
			goto err;
		a /= b;
		break;
	case '%':
		if (next_char(p) == '(')
			b = do_expression(p, &err, 1);
		else
			b = get_number(p, &err);
		if (err)
			goto err;
		a %= b;
		break;
	case '&':
		if (next_char(p) == '(')
			b = do_expression(p, &err, 1);
		else
			b = get_number(p, &err);
		if (err)
			goto err;
		a &= b;
		break;
	case '|':
		if (next_char(p) == '(')
			b = do_expression(p, &err, 1);
		else
			b = get_number(p, &err);
		if (err)
			goto err;
		a |= b;
		break;
	case '^':
		if (next_char(p) == '(')
			b = do_expression(p, &err, 1);
		else
			b = get_number(p, &err);
		if (err)
			goto err;
		a ^= b;
		break;
	case 'v':
		if (next_char(p) == '(')
			b = do_expression(p, &err, 1);
		else
			b = get_number(p, &err);
		if (err)
			goto err;
		a = ~(a | b);
		break;
	case '>':
		if (next_char(p) == '(')
			b = do_expression(p, &err, 1);
		else
			b = get_number(p, &err);
		if (err)
			goto err;
		a >>= b;
		break;
	case '<':
		if (next_char(p) == '(')
			b = do_expression(p, &err, 1);
		else
			b = get_number(p, &err);
		if (err)
			goto err;
		a <<= b;
		break;
	case '\0':
		if (sub) {
			a = tmp - a;
			sub = 0;
		}
		return a;
	case -1:
		err = 1;
	}
	if (err)
		goto err;
	goto reget;
	return 0;
}

static void __init_fd_tc(int fd)
{
	struct termios options;

	tcgetattr(fd, &options);
	tcflush(fd, TCIOFLUSH);

	options.c_lflag &= ~(ICANON | ECHO | ECHOE);

	tcsetattr(fd, TCSANOW, &options);
}

static inline void backspace(void)
{
	putc(8, stdout);	// BS
	putc(32, stdout);	// SPACE
	putc(8, stdout);	// BS
}

static inline void clear_line(unsigned int len)
{
	while (len--) {
		backspace();
	}
}

static void get_input(void)
{
	int in;

	while ((in = getc(stdin)) != EOF && inlen < sizeof(inbuf)) {
		switch (in) {
		case 3:		// ^C
		case 4:		// ^D
		case 'q':
		case 'Q':
			exit(EXIT_SUCCESS);
		case 8:		// BS
		case 127:	// DEL
			if (inlen > 0) {
				backspace();
				inbuf[--inlen] = '\0';
			}
			break;
		case 10:	// LF
		case 13:	// CR
		case '=':
			putc(10, stdout);
			return;
		case 27:	// ESC
		case 'r':
		case 'R':
			if (inlen) {
				clear_line(inlen);
				clear_ibuf();
			}
			break;
		case '?':
			return help();
		case ' ':
			if (inlen && !isspace(inbuf[inlen - 1])) {
		default:
				if (isprint(in)) {
					inbuf[inlen++] = in;
					putc(in, stdout);
				}
			}
		}
	}
}

int main(int argc, char *argv[])
{
	int err;
	char *p;
	uint64_t result;
	__init_fd_tc(0);
	if (argc < 2) {
		interactive = 1;
repeat_new:
		help();
		new_prompt();
repeat:
		get_input();
	} else {
		interactive = 0;
		get_from_argv(argc - 1, argv + 1);
	}
	p = inbuf;
	result = do_expression(&p, &err, 0);
	if (err) {
		if (!interactive)
			help();
		else
			goto repeat_new;
	} else {
		result_prompt(result);
		if (interactive)
			goto repeat;
	}
	return EXIT_SUCCESS;
}

