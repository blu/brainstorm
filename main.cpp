#include <stdint.h>
#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "stream.hpp"
#include "scoped.hpp"
#include "get_file_size.hpp"

// verify iostream-free status
#if _GLIBCXX_IOSTREAM
#error rogue iostream acquired
#endif

namespace stream {
// deferred initialization by main()
in cin;
out cout;
out cerr;
} // namespace stream

static const char arg_prefix[]         = "-";
static const char arg_memory_size[]    = "memory_size";
static const char arg_terminal_count[] = "terminal_count";
static const char arg_print_ascii[]    = "print_ascii";

static const size_t default_memory_size_kw = 32;
static const size_t default_terminal_count = 4096;

struct cli_param {
	enum {
		FLAG_PRINT_ASCII = 1
	};
	uint64_t terminalCount;

#if BAD_CASE
	uint32_t memorySize;
	uint32_t flags;

#else
	uint64_t memorySize;
	uint64_t flags;

#endif
	const char* filename;
};

static int
parse_cli(
	const int argc,
	char** const argv,
	cli_param& param) {

	const unsigned prefix_len = std::strlen(arg_prefix);
	bool success = true;

	param.filename = 0;

	for (int i = 1; i < argc && success; ++i) {
		if (std::strncmp(argv[i], arg_prefix, prefix_len)) {
			if (0 != param.filename)
				success = false;

			param.filename = argv[i];
			continue;
		}

		if (!std::strcmp(argv[i] + prefix_len, arg_memory_size)) {
#if BAD_CASE
			if (++i == argc || 1 != sscanf(argv[i], "%u", &param.memorySize))
				success = false;

#else
			if (++i == argc || 1 != sscanf(argv[i], "%lu", &param.memorySize))
				success = false;

#endif
			continue;
		}

		if (!std::strcmp(argv[i] + prefix_len, arg_terminal_count)) {
			if (++i == argc || 1 != sscanf(argv[i], "%lu", &param.terminalCount))
				success = false;

			continue;
		}

		if (!std::strcmp(argv[i] + prefix_len, arg_print_ascii)) {
			param.flags |= size_t(cli_param::FLAG_PRINT_ASCII);
			continue;
		}

		success = false;
	}

	if (!success || 0 == param.filename) {
		stream::cerr << "usage: " << argv[0] << " [<option> ...] <source_filename>\n"
			"options (multiple args to an option must constitute a single string, eg. -foo \"a b c\"):\n"
			"\t" << arg_prefix << arg_memory_size << " <positive_integer>\t\t: amount of memory available to program, in words; default is " << default_memory_size_kw << "Kwords\n"
			"\t" << arg_prefix << arg_terminal_count << " <positive_integer>\t: number of steps after which program is forcefully terminated; default is " << default_terminal_count << "\n"
			"\t" << arg_prefix << arg_print_ascii << "\t\t\t\t: print out in ASCII encoding rather than raw numbers\n";

		return 1;
	}

	return 0;
}

typedef uint8_t word_t; // machine word type

template < typename T >
class generic_free {
public:
	void operator()(T* arg) {
		assert(0 != arg);
		free(arg);
	}
};

template < bool >
struct compile_assert;

template <>
struct compile_assert< true > {
	compile_assert() {}
};

enum Opcode {
	OPCODE_INC_WORD,          // '+'
	OPCODE_DEC_WORD,          // '-'
	OPCODE_INC_PTR,           // '>'
	OPCODE_DEC_PTR,           // '<'
	OPCODE_INPUT,             // '.'
	OPCODE_OUTPUT,            // ','
	OPCODE_COND_L = 0x8000,   // '['
	OPCODE_COND_R = 0xc000,   // ']'

	OPCODE_COUNT = 8
};

namespace {
const compile_assert< 8 == OPCODE_COUNT > assert_opcode_count;
} // namespace annonymous

class Command {
	uint16_t op; // encoding uses unsigned offset (direction determined by the type of op)

	Command(); // undefined

public:
	enum { offset_range = 1 << 14 };

	Command(
		const Opcode an_op,
		const uint16_t an_offset) {

		switch (an_op) {
		case OPCODE_COND_L:
		case OPCODE_COND_R:
			op = an_op | an_offset;
			break;
		default:
			op = an_op;
		}
	}

	Opcode getOp() const {

		// is this a conditional branch?
		if (op & uint16_t(0x8000))
			return Opcode(op & uint16_t(0xc000));

		return Opcode(op);
	}

	uint16_t getOffset() const {
		return op & ~uint16_t(0xc000);
	}
};

namespace {
const compile_assert< 2 == sizeof(Command) > assert_sizeof_command;
} // namespace annonymous

static size_t seekBalancedClose(
	const Command* const program,
	const size_t programLength) {

	size_t count = 0;
	size_t pos = 0;

	while (++pos < programLength) {
		if (program[pos].getOp() == OPCODE_COND_L) {
			++count;
			continue;
		}
		if (program[pos].getOp() == OPCODE_COND_R) {
			if (0 == count)
				return pos;
			--count;
		}
	}

	return 0;
}

static Command* translate(
	const char* const source,
	const size_t sourceLength,
	size_t& programLength) {

	using testbed::scoped_ptr;

	scoped_ptr< Command, generic_free > program(
		reinterpret_cast< Command* >(std::calloc(sourceLength, sizeof(Command))));

	size_t i = 0;
	size_t j = 0;

	while (i < sourceLength) {
		switch (source[i]) {
		case '+':
			program()[j++] = Command(OPCODE_INC_WORD, 0);
			break;
		case '-':
			program()[j++] = Command(OPCODE_DEC_WORD, 0);
			break;
		case '>':
			program()[j++] = Command(OPCODE_INC_PTR, 0);
			break;
		case '<':
			program()[j++] = Command(OPCODE_DEC_PTR, 0);
			break;
		case ',':
			program()[j++] = Command(OPCODE_INPUT, 0);
			break;
		case '.':
			program()[j++] = Command(OPCODE_OUTPUT, 0);
			break;
		case '[':
			program()[j++] = Command(OPCODE_COND_L, 0); // defer offset calculation
			break;
		case ']':
			program()[j++] = Command(OPCODE_COND_R, 0); // defer offset calculation
			break;
		}
		++i;
	}

	// calculate offsets
	for (i = 0; i < j; ++i)
		if (OPCODE_COND_L == program()[i].getOp()) {
				const size_t offset = seekBalancedClose(program() + i, j - i);

				if (0 == offset)
					break;

				if (Command::offset_range > offset) {
					program()[i] = Command(OPCODE_COND_L, uint16_t(offset));
					program()[i + offset] = Command(OPCODE_COND_R, uint16_t(offset));
					continue;
				}

				stream::cerr << "program error: way too far jump at ip " << i << " - crash imminent\n";
		}

	if (i != j) {
		stream::cerr << "program error: unmached [ at ip " << i << "\n";
		return 0;
	}

	Command* const res = program();
	program.reset();
	programLength = j;

	return res;
}

int main(
	int argc,
	char** argv) {

	using testbed::scoped_ptr;
	using testbed::get_buffer_from_file;

	stream::cin.open(stdin);
	stream::cout.open(stdout);
	stream::cerr.open(stderr);

	cli_param param;
	param.memorySize = default_memory_size_kw << 10;
	param.terminalCount = default_terminal_count;
	param.flags = 0;
	param.filename = 0;

	const int result_cli = parse_cli(argc, argv, param);

	if (0 != result_cli)
		return result_cli;

	const bool print_ascii = bool(param.flags & cli_param::FLAG_PRINT_ASCII);

	size_t sourceLength = 0;
	const scoped_ptr< char, generic_free > source(
		reinterpret_cast< char* >(get_buffer_from_file(param.filename, sourceLength)));

	if (0 == source()) {
		stream::cerr << "failed to open source file\n";
		return -1;
	}

	size_t programLength = 0;
	const scoped_ptr< Command, generic_free > program(
		reinterpret_cast< Command* >(translate(source(), sourceLength, programLength)));

	if (!program()) {
		stream::cerr << "program error: unable to translate to IR\n";
		return -1;
	}

	const scoped_ptr< word_t, generic_free > mem(
		reinterpret_cast< word_t* >(std::calloc(param.memorySize, sizeof(word_t))));

	if (0 == mem()) {
		stream::cerr << "failed to provide data memory\n";
		return -1;
	}

	uint64_t count = 0;
	const uint64_t terminalCount = param.terminalCount;
	size_t ip = 0;
	size_t dp = 0;
	const size_t memorySize = param.memorySize;

	while (count < terminalCount &&
		   ip < programLength &&
		   dp < memorySize) {

		const Command cmd = program()[ip];
		int input;

		switch (cmd.getOp()) {
		case OPCODE_INC_WORD:
			++mem()[dp];
			break;
		case OPCODE_DEC_WORD:
			--mem()[dp];
			break;
		case OPCODE_INC_PTR:
			++dp;
			break;
		case OPCODE_DEC_PTR:
			--dp;
			break;
		case OPCODE_INPUT:
			stream::cin >> input;
			mem()[dp] = word_t(input);
			break;
		case OPCODE_OUTPUT:
			if (print_ascii)
				stream::cout << char(mem()[dp]);
			else
				stream::cout << mem()[dp] << " ";
			break;
		case OPCODE_COND_L:
			if (0 == mem()[dp])
				ip += size_t(cmd.getOffset());
			break;
		case OPCODE_COND_R:
			if (0 != mem()[dp])
				ip -= size_t(cmd.getOffset());
			break;
		}
		++ip;
		++count;
	}

	if (dp >= memorySize) {
		stream::cerr << "program error: out-of-bounds data pointer at ip " << ip - 1 << "\n";
		return -1;
	}

	stream::cout << "\ninstructions executed: " << count << "\n";
	return 0;
}
