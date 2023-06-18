#include <stdint.h>
#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "stream.hpp"
#include "scoped.hpp"
#include "util_file.hpp"

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
static const size_t mempage_size = 4096;
#if __LP64__ == 1
static const size_t cacheline_size = 64;

#else
static const size_t cacheline_size = 32;

#endif

struct cli_param {
	enum {
		FLAG_PRINT_ASCII = 1
	};
	uint64_t terminalCount;

	uint32_t memorySize;
	uint32_t flags;

	const char* filename;
};

static int __attribute__ ((noinline)) parse_cli(
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
			if (++i == argc || 1 != sscanf(argv[i], "%u", &param.memorySize))
				success = false;

			continue;
		}

#if ENABLE_DIAGNOSTICS
		if (!std::strcmp(argv[i] + prefix_len, arg_terminal_count)) {
			if (++i == argc || 1 != sscanf(argv[i], "%lu", &param.terminalCount))
				success = false;

			continue;
		}

#endif
#if PRINT_ASCII == 0
		if (!std::strcmp(argv[i] + prefix_len, arg_print_ascii)) {
			param.flags |= size_t(cli_param::FLAG_PRINT_ASCII);
			continue;
		}

#endif
		success = false;
	}

	if (!success || 0 == param.filename) {
		stream::cerr << "usage: " << argv[0] << " [<option> ...] <source_filename>\n"
			"options (multiple args to an option must constitute a single string, eg. -foo \"a b c\"):\n"
			"\t" << arg_prefix << arg_memory_size << " <positive_integer>\t\t: amount of memory available to program, in words; default is " << default_memory_size_kw << "Kwords\n"

#if ENABLE_DIAGNOSTICS
			"\t" << arg_prefix << arg_terminal_count << " <positive_integer>\t: number of steps after which program is forcefully terminated; default is " << default_terminal_count << '\n'

#endif
#if PRINT_ASCII == 0
			"\t" << arg_prefix << arg_print_ascii << "\t\t\t\t: print in ASCII instead of numbers\n"

#endif
			;
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
	OPCODE_INC_WORD, // '+'
	OPCODE_DEC_WORD, // '-'
	OPCODE_ADD_PTR,  // '>', repetitions of
	OPCODE_SUB_PTR,  // '<', repetitions of
	OPCODE_COND_L,   // '['
	OPCODE_COND_R,   // ']'
	OPCODE_INPUT,    // '.'
	OPCODE_OUTPUT,   // ','
};

class Command {
	uint16_t op; // encoding uses unsigned immediates (direction determined by the type of op)

	Command(); // undefined

public:
	enum { imm_range = 1 << (16 - 3) };

	Command(
		const Opcode an_op,
		const uint16_t an_imm) {

		assert(imm_range > an_imm);

		op = uint16_t(an_op) | an_imm << 3;
	}

	Opcode getOp() const {

		return Opcode(op & uint16_t(0x7));
	}

	size_t getImm() const {
		return size_t(op >> 3);
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

static bool is_nop(const char op) {
	return
		op != '+' &&
		op != '-' &&
		op != '>' &&
		op != '<' &&
		op != '[' &&
		op != ']' &&
		op != ',' &&
		op != '.';
}

static Command* __attribute__ ((noinline)) translate(
	const char* const source,
	const size_t sourceLength,
	Command* const program,
	size_t& programLength) {

	size_t i = 0;
	size_t j = 0;
	bool err = false;

	while (i < sourceLength) {
		size_t imm;
		size_t skip;

		switch (source[i]) {
		case '+':
			program[j++] = Command(OPCODE_INC_WORD, 0);
			break;
		case '-':
			program[j++] = Command(OPCODE_DEC_WORD, 0);
			break;
		case '>':
			for (imm = i + 1, skip = 0; imm < sourceLength; ++imm) {
				if (is_nop(source[imm]))
					++skip;
				else
				if ('>' != source[imm])
					break;
			}
			if (Command::imm_range > imm - i - skip) {
				program[j++] = Command(OPCODE_ADD_PTR, uint16_t(imm - i - skip));
			}
			else {
				program[j++] = Command(OPCODE_ADD_PTR, 0);
				stream::cerr << "program error: way too many '>' at ip " << i << '\n';
				err = true;
			}
			i = imm - 1;
			break;
		case '<':
			for (imm = i + 1, skip = 0; imm < sourceLength; ++imm) {
				if (is_nop(source[imm]))
					++skip;
				else
				if ('<' != source[imm])
					break;
			}
			if (Command::imm_range > imm - i - skip) {
				program[j++] = Command(OPCODE_SUB_PTR, uint16_t(imm - i - skip));
			}
			else {
				program[j++] = Command(OPCODE_SUB_PTR, 0);
				stream::cerr << "program error: way too many '<' at ip " << i << '\n';
				err = true;
			}
			i = imm - 1;
			break;
		case ',':
			program[j++] = Command(OPCODE_INPUT, 0);
			break;
		case '.':
			program[j++] = Command(OPCODE_OUTPUT, 0);
			break;
		case '[':
			program[j++] = Command(OPCODE_COND_L, 0); // defer offset calculation
			break;
		case ']':
			program[j++] = Command(OPCODE_COND_R, 0); // defer offset calculation
			break;
		}
		++i;
	}

	// calculate offsets
	for (i = 0; i < j; ++i)
		if (OPCODE_COND_L == program[i].getOp()) {
			const size_t offset = seekBalancedClose(program + i, j - i);

			if (0 == offset) {
				stream::cerr << "program error: unmached [ at ip " << i << '\n';
				err = true;
				break;
			}

			if (Command::imm_range > offset) {
				program[i] = Command(OPCODE_COND_L, uint16_t(offset));
				program[i + offset] = Command(OPCODE_COND_R, uint16_t(offset));
				continue;
			}

			stream::cerr << "program error: way too far jump at ip " << i << '\n';
			err = true;
		}

	if (err)
		return 0;

	programLength = j;
	return program;
}

template < typename WORD_T, uintptr_t ALIGNMENT = cacheline_size >
class AlignedPtr {
	WORD_T* m;

public:
	AlignedPtr(const uintptr_t ptr)
	: m(reinterpret_cast< WORD_T* >((ptr + ALIGNMENT - 1) & ~(ALIGNMENT - 1))) {
	}

	WORD_T* operator()() const {
		return m;
	}
};

template < typename WORD_T >
class Ptr { // just for notational consistency with AlignedPtr
	WORD_T* m;

public:
	Ptr(WORD_T* const ptr)
	: m(ptr) {
	}

	WORD_T* operator()() const {
		return m;
	}
};

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

	const size_t dataLength = param.memorySize;

	scoped_ptr< void, generic_free > space(
		std::calloc(sourceLength * sizeof(Command) + dataLength * sizeof(word_t) + mempage_size + cacheline_size, sizeof(int8_t)));

	if (0 == space()) {
		stream::cerr << "failed to provide program and data memory\n";
		return 0;
	}

	const AlignedPtr< Command, mempage_size > code((uintptr_t(space())));
	size_t programLength = 0;

	const Ptr< Command > program(
		translate(source(), sourceLength, code(), programLength));

	if (!program()) {
		stream::cerr << "unable to provide program IR\n";
		return -1;
	}

	const AlignedPtr< word_t, cacheline_size > mem(uintptr_t(code() + programLength));

	uint64_t count = 0;
	const uint64_t terminalCount = param.terminalCount;
	size_t ip = 0;
	size_t dp = 0;

#if ENABLE_DIAGNOSTICS
	while (count < terminalCount &&
		   ip < programLength &&
		   dp < dataLength) {

#else
	while (ip < programLength) {

#endif
		const Command cmd = program()[ip];
		int input;

		const uint32_t op = cmd.getOp();

#if __clang_major__ > 3 || __clang_major__ == 3 && __clang_minor__ >= 6
		__builtin_assume(op < 8);

#endif
		switch (op) {
		case OPCODE_INC_WORD:
			++mem()[dp];
			break;
		case OPCODE_DEC_WORD:
			--mem()[dp];
			break;
		case OPCODE_ADD_PTR:
			dp += cmd.getImm();
			break;
		case OPCODE_SUB_PTR:
			dp -= cmd.getImm();
			break;
		case OPCODE_COND_L:
			if (0 == mem()[dp])
				ip += cmd.getImm();
			break;
		case OPCODE_COND_R:
			if (0 != mem()[dp])
				ip -= cmd.getImm();
			break;
		case OPCODE_INPUT:
			stream::cin >> input;
			mem()[dp] = word_t(input);
			break;
		case OPCODE_OUTPUT:

#if PRINT_ASCII
			stream::cout << char(mem()[dp]);

#else
			if (print_ascii)
				stream::cout << char(mem()[dp]);
			else
				stream::cout << mem()[dp] << ' ';

#endif
			break;
		}

		++ip;
		++count;
	}

#if ENABLE_DIAGNOSTICS
	if (dp >= dataLength) {
		stream::cerr << "program error: out-of-bounds data pointer at ip " << ip - 1 << '\n';
		return -1;
	}

	stream::cout << "\ninstructions executed: " << count << '\n';

#endif
	return 0;
}
