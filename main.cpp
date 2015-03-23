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
	size_t memorySize;
	size_t terminalCount;
	size_t flags;
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
			if (++i == argc || 1 != sscanf(argv[i], "%lu", &param.memorySize))
				success = false;

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

typedef uint8_t command_t; // instruction type
typedef int8_t word_t; // machine word type

template < typename T >
class generic_free {
public:
	void operator()(T* arg) {
		assert(0 != arg);
		free(arg);
	}
};

template < typename T >
class Stack {
	size_t size;
	size_t top;
	testbed::scoped_ptr< T, generic_free > buffer;

	Stack(); // undefined

public:
	Stack(const size_t size)
	: size(size)
	, top(-1)
	, buffer(reinterpret_cast< T* >(std::malloc(sizeof(T) * size))) {
	}

	bool is_valid() const {
		return 0 != buffer();
	}

	bool is_stack_fault() const {
		return 0 == size;
	}

	void stack_fault() {
		size = 0;
	}

	void push(const T arg) {
		if (is_stack_fault())
			return;

		if (++top != size)
			buffer()[top] = arg;
		else
			stack_fault();
	}

	T getTop() {
		if (is_stack_fault())
			return T(-1);

		if (top < size)
			return buffer()[top];

		stack_fault();
		return T(-1);
	}

	void pop() {
		if (is_stack_fault())
			return;

		if (top == size_t(-1)) {
			stack_fault();
			return;
		}

		--top;
	}

	size_t count() const {
		return top + 1;
	}
};

static size_t seekBalancedClose(
	const command_t* const program,
	const size_t programLength,
	Stack< size_t >& ipStack) {

	size_t count = 0;
	size_t pos = 0;

	while (++pos < programLength) {
		if (program[pos] == command_t('[')) {
			++count;
			continue;
		}
		if (program[pos] == command_t(']')) {
			if (0 == count)
				return pos;
			--count;
		}
	}

	ipStack.stack_fault();
	return 0;
}

int main(
	int argc,
	char** argv) {

	using testbed::scoped_ptr;

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

	size_t programLength = 0;
	const scoped_ptr< command_t, generic_free > program(
		reinterpret_cast< command_t* >(testbed::get_buffer_from_file(param.filename, programLength)));

	if (0 == program()) {
		stream::cerr << "failed to open source file\n";
		return -1;
	}

	Stack< size_t > ipStack(programLength);

	if (!ipStack.is_valid()) {
		stream::cerr << "failed to provide ip stack\n";
		return -1;
	}

	const scoped_ptr< word_t, generic_free > mem(
		reinterpret_cast< word_t* >(std::calloc(param.memorySize, sizeof(word_t))));

	if (0 == mem()) {
		stream::cerr << "failed to provide data memory\n";
		return -1;
	}

	size_t ip = 0;
	size_t count = 0;
	const size_t terminalCount = param.terminalCount;
	size_t dp = 0;
	const size_t memorySize = param.memorySize;

	while (count < terminalCount &&
		   ip < programLength &&
		   dp < memorySize &&
		   !ipStack.is_stack_fault()) {

		switch (program()[ip]) {
			int input;
		case command_t('>'):
			++dp;
			break;
		case command_t('<'):
			--dp;
			break;
		case command_t('+'):
			++mem()[dp];
			break;
		case command_t('-'):
			--mem()[dp];
			break;
		case command_t('.'):
			if (print_ascii)
				stream::cout << char(mem()[dp]);
			else
				stream::cout << mem()[dp] << " ";
			break;
		case command_t(','):
			stream::cin >> input;
			mem()[dp] = word_t(input);
			break;
		case command_t('['):
			if (0 == mem()[dp])
				ip += seekBalancedClose(program() + ip, programLength - ip, ipStack);
			else // enter loop
				ipStack.push(ip);
			break;
		case command_t(']'):
			if (0 != mem()[dp]) {
				const size_t iq = ipStack.getTop();
				ip = (size_t(-1) != iq)? iq: ip;
			} else // exit loop
				ipStack.pop();
			break;
		default:
			// discard non-ops
			--count;
		}
		++ip;
		++count;
	}

	if (dp >= memorySize) {
		stream::cerr << "program error: out-of-bounds data pointer at ip " << ip - 1 << "\n";
		return -1;
	}

	if (ipStack.is_stack_fault()) {
		stream::cerr << "program error: branch balance breach at ip " << ip - 1 << "\n";
		return -1;
	}
	else // late check for imbalanced entered loops
	if (ipStack.count() && count != terminalCount) {
		stream::cerr << "program error: branch balance breach at ip " << ipStack.getTop() << "\n";
		return -1;
	}

	stream::cout << "\ninstructions executed: " << count << "\n";
	return 0;
}
