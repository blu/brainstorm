#ifndef stream_H__
#define stream_H__

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#ifdef _MSC_VER
#include <io.h>
#else
#include <unistd.h>
#endif
#include <assert.h>
#include <string>
#include <cstring>

namespace stream {

class in {
	FILE* file;

public:
	in()
	: file(0) {
	}

	void close() {
		if (0 == file)
			return;

		fclose(file);
		file = 0;
	}

	bool open(const char* const filename) {
		close();

		file = fopen(filename, "r");
		return 0 != file;
	}

	bool open(FILE* const f) {
		close();

		const int fd = fileno(f);
		if (-1 != fd)
			file = fdopen(dup(fd), "r");

		return 0 != file;
	}

	~in() {
		close();
	}

	bool is_eof() const {
		if (0 != file)
			return static_cast< bool >(feof(file));

		return false;
	}

	bool is_good() const {
		return (0 != file) && (0 == ferror(file));
	}

	void set_good() const {
		if (0 != file)
			clearerr(file);
	}

	const in& operator >>(char& a) const {
		if (0 != file)
			a = getc(file);

		return *this;
	}

	const in& operator >>(int16_t& a) const {
		if (0 != file) {
			const int nread = fscanf(file, "%hd", &a);
			assert(1 == nread); (void) nread;
		}

		return *this;
	}

	const in& operator >>(uint16_t& a) const {
		if (0 != file) {
			const int nread = fscanf(file, "%hu", &a);
			assert(1 == nread); (void) nread;
		}

		return *this;
	}

	const in& operator >>(int32_t& a) const {
		if (0 != file) {
			const int nread = fscanf(file, "%d", &a);
			assert(1 == nread); (void) nread;
		}

		return *this;
	}

	const in& operator >>(uint32_t& a) const {
		if (0 != file) {
			const int nread = fscanf(file, "%u", &a);
			assert(1 == nread); (void) nread;
		}

		return *this;
	}

	const in& operator >>(int64_t& a) const {
		if (0 != file) {
#if _MSC_VER || __APPLE__ || __SIZEOF_LONG__ == 4
			const int nread = fscanf(file, "%lld", &a);

#else
			const int nread = fscanf(file, "%ld", &a);

#endif
			assert(1 == nread); (void) nread;
		}
		return *this;
	}

	const in& operator >>(uint64_t& a) const {
		if (0 != file) {
#if _MSC_VER || __APPLE__ || __SIZEOF_LONG__ == 4
			const int nread = fscanf(file, "%llu", &a);

#else
			const int nread = fscanf(file, "%lu", &a);

#endif
			assert(1 == nread); (void) nread;
		}
		return *this;
	}

#if __APPLE__ // type size_t is unrelated to same-size type uint*_t
#if _LP64 == 1
	const in& operator >>(size_t& a) const {
		return *this >> reinterpret_cast< uint64_t& >(a);
	}

#else
	const in& operator >>(size_t& a) const {
		return *this >> reinterpret_cast< uint32_t& >(a);
	}

#endif
#endif
	const in& operator >>(float& a) const {
		if (0 != file) {
			const int nread = fscanf(file, "%f", &a);
			assert(1 == nread); (void) nread;
		}

		return *this;
	}

	const in& operator >>(double& a) const {
		if (0 != file) {
			const int nread = fscanf(file, "%lf", &a);
			assert(1 == nread); (void) nread;
		}

		return *this;
	}

	const in& operator >>(void*& a) const {
		if (0 != file) {
			const int nread = fscanf(file, "%p", &a);
			assert(1 == nread); (void) nread;
		}

		return *this;
	}

	const in& operator >>(std::string& a) const {
		if (0 != file) {
			const size_t buffer_inc = 1024;
			size_t buffer_size = 0;
			size_t len = 0;
			char* buffer = 0;

			while (true) {
				if (feof(file) || ferror(file))
					break;

				if (len == buffer_size) {
					buffer_size += buffer_inc;
					buffer = (char*) realloc(buffer, buffer_size);
				}

				const char read = getc(file);
				if (' ' == read || '\t' == read || '\n' == read)
					break;

				buffer[len++] = read;
			}

			a.assign(buffer, len);
			free(buffer);
		}

		return *this;
	}
};

// different STL implementations do differently the types that control stream width and charater-filler formatting; we go straight:
struct setw {
	int width;
	setw(const int width) : width(width) {}
};

struct setfill {
	char fillchar;
	setfill(const char fillchar) : fillchar(fillchar) {}
};

// std::dec/oct/hex are functions which are passed by pointner to a stream; we don't care about the funtional aspect of those,
// just the form of their passing to a stream by identifier; for that we user an enum of function ids:

enum NumericBaseFuncId {
	dec, // std::ios_base& dec(std::ios_base&)
	hex, // std::ios_base& hex(std::ios_base&)
	oct  // std::ios_base& oct(std::ios_base&)
};


// std::endl/ends/flush are functions which are passed by pointer to a stream; we don't care about the functional aspect of those,
// just the form of their passing to a stream by identifier; for that we use an enum of function ids:

enum TerminatorFuncId {
	endl, // std::basic_ostream< char, std::char_traits< char > >& endl(std::basic_ostream< char, std::char_traits< char > >&)
	ends, // std::basic_ostream< char, std::char_traits< char > >& ends(std::basic_ostream< char, std::char_traits< char > >&)
	flush // std::basic_ostream< char, std::char_traits< char > >& flush(std::basic_ostream< char, std::char_traits< char > >&)
};

class out {
	FILE* file;
	int width;
	char fillchar;

	enum Base {
		BASE_DEC,
		BASE_HEX,
		BASE_OCT
	} base;

	static size_t setFillInFormatStr(
		char (& format)[64],
		size_t fmtlen,
		const char fillchar) {

		const char fill_space[] = "%*";
		const char fill_zero[] = "%0*";
		const char fill_other[] = "<bad_fill> %*";

		switch (fillchar) {
		case ' ':
			memcpy(format + fmtlen, fill_space, strlen(fill_space));
			fmtlen += strlen(fill_space);
			break;

		case '0':
			memcpy(format + fmtlen, fill_zero, strlen(fill_zero));
			fmtlen += strlen(fill_zero);
			break;

		default:
			memcpy(format + fmtlen, fill_other, strlen(fill_other));
			fmtlen += strlen(fill_other);
			break;
		}

		return fmtlen;
	}

	static size_t setBaseInFormatStr(
		char (& format)[64],
		size_t fmtlen,
		const Base base,
		const char* base_dec,
		const char* base_hex,
		const char* base_oct) {

		switch (base) {
		case BASE_DEC:
			memcpy(format + fmtlen, base_dec, strlen(base_dec));
			fmtlen += strlen(base_dec);
			break;

		case BASE_HEX:
			memcpy(format + fmtlen, base_hex, strlen(base_hex));
			fmtlen += strlen(base_hex);
			break;

		case BASE_OCT:
			memcpy(format + fmtlen, base_oct, strlen(base_oct));
			fmtlen += strlen(base_oct);
			break;
		}

		return fmtlen;
	}

public:
	out()
	: file(0)
	, width(0)
	, fillchar(' ')
	, base(BASE_DEC) {
	}

	void close() {
		if (0 == file)
			return;

		fclose(file);
		file = 0;
	}

	bool open(const char* const filename, const bool append = true) {
		close();

		const char* const mode = append ? "a" : "w";
		file = fopen(filename, mode);
		return 0 != file;
	}

	bool open(FILE* const f) {
		close();

		const int fd = fileno(f);
		if (-1 != fd)
			file = fdopen(dup(fd), "a");

		return 0 != file;
	}

	~out() {
		close();
	}

	out& write(const char* const src, const size_t len) {
		if (0 != file && 0 != src && 0 != len)
			fwrite(src, sizeof(*src), len, file);

		return *this;
	}

	void flush() const {
		if (0 != file)
			fflush(file);
	}

	bool is_good() const {
		return (0 != file) && (0 == ferror(file));
	}

	void set_good() const {
		if (0 != file)
			clearerr(file);
	}

	out& operator <<(const char a) {
		if (0 != file)
			putc(a, file);

		return *this;
	}

	out& operator <<(const int16_t a) {
		if (0 == file)
			return *this;

		char format[64];
		size_t fmtlen = 0;

		const char base_dec[] = "hd";
		const char base_hex[] = "hx";
		const char base_oct[] = "ho";

		fmtlen = setFillInFormatStr(format, fmtlen, fillchar);
		fmtlen = setBaseInFormatStr(format, fmtlen, base, base_dec, base_hex, base_oct);

		format[fmtlen++] = '\0';

		fprintf(file, format, width, a);

		// reset width as per std::ostream specs
		width = 0;
		return *this;
	}

	out& operator <<(const uint16_t a) {
		if (0 == file)
			return *this;

		char format[64];
		size_t fmtlen = 0;

		const char base_dec[] = "hu";
		const char base_hex[] = "hx";
		const char base_oct[] = "ho";

		fmtlen = setFillInFormatStr(format, fmtlen, fillchar);
		fmtlen = setBaseInFormatStr(format, fmtlen, base, base_dec, base_hex, base_oct);

		format[fmtlen++] = '\0';

		fprintf(file, format, width, a);

		// reset width as per std::ostream specs
		width = 0;
		return *this;
	}

	out& operator <<(const int32_t a) {
		if (0 == file)
			return *this;

		char format[64];
		size_t fmtlen = 0;

		const char base_dec[] = "d";
		const char base_hex[] = "x";
		const char base_oct[] = "o";

		fmtlen = setFillInFormatStr(format, fmtlen, fillchar);
		fmtlen = setBaseInFormatStr(format, fmtlen, base, base_dec, base_hex, base_oct);

		format[fmtlen++] = '\0';

		fprintf(file, format, width, a);

		// reset width as per std::ostream specs
		width = 0;
		return *this;
	}

	out& operator <<(const uint32_t a) {
		if (0 == file)
			return *this;

		char format[64];
		size_t fmtlen = 0;

		const char base_dec[] = "u";
		const char base_hex[] = "x";
		const char base_oct[] = "o";

		fmtlen = setFillInFormatStr(format, fmtlen, fillchar);
		fmtlen = setBaseInFormatStr(format, fmtlen, base, base_dec, base_hex, base_oct);

		format[fmtlen++] = '\0';

		fprintf(file, format, width, a);

		// reset width as per std::ostream specs
		width = 0;
		return *this;
	}

	out& operator <<(const int64_t a) {
		if (0 == file)
			return *this;

		char format[64];
		size_t fmtlen = 0;

#if _MSC_VER || __APPLE__ || __SIZEOF_LONG__ == 4
		const char base_dec[] = "lld";
		const char base_hex[] = "llx";
		const char base_oct[] = "llo";

#else
		const char base_dec[] = "ld";
		const char base_hex[] = "lx";
		const char base_oct[] = "lo";

#endif
		fmtlen = setFillInFormatStr(format, fmtlen, fillchar);
		fmtlen = setBaseInFormatStr(format, fmtlen, base, base_dec, base_hex, base_oct);

		format[fmtlen++] = '\0';

		fprintf(file, format, width, a);

		// reset width as per std::ostream specs
		width = 0;
		return *this;
	}

	out& operator <<(const uint64_t a) {
		if (0 == file)
			return *this;

		char format[64];
		size_t fmtlen = 0;

#if _MSC_VER || __APPLE__ || __SIZEOF_LONG__ == 4
		const char base_dec[] = "llu";
		const char base_hex[] = "llx";
		const char base_oct[] = "llo";

#else
		const char base_dec[] = "lu";
		const char base_hex[] = "lx";
		const char base_oct[] = "lo";

#endif
		fmtlen = setFillInFormatStr(format, fmtlen, fillchar);
		fmtlen = setBaseInFormatStr(format, fmtlen, base, base_dec, base_hex, base_oct);

		format[fmtlen++] = '\0';

		fprintf(file, format, width, a);

		// reset width as per std::ostream specs
		width = 0;
		return *this;
	}

#if __APPLE__ // type size_t is unrelated to same-size type uint*_t
#if _LP64 == 1
	out& operator <<(const size_t a) {
		return *this << reinterpret_cast< const uint64_t& >(a);
	}

#else
	out& operator <<(const size_t a) {
		return *this << reinterpret_cast< const uint32_t& >(a);
	}

#endif
#endif
	out& operator <<(const float a) {
		if (0 == file)
			return *this;

		char format[64];
		size_t fmtlen = 0;

		fmtlen = setFillInFormatStr(format, fmtlen, fillchar);

		format[fmtlen++] = 'f';
		format[fmtlen++] = '\0';

		fprintf(file, format, width, a);

		// reset width as per std::ostream specs
		width = 0;
		return *this;
	}

	out& operator <<(const double a) {
		if (0 == file)
			return *this;

		char format[64];
		size_t fmtlen = 0;

		fmtlen = setFillInFormatStr(format, fmtlen, fillchar);

		format[fmtlen++] = 'f';
		format[fmtlen++] = '\0';

		fprintf(file, format, width, a);

		// reset width as per std::ostream specs
		width = 0;
		return *this;
	}

	out& operator <<(const void* const a) {
		if (0 != file)
			fprintf(file, "%p", a);

		return *this;
	}

	out& operator <<(const char* const a) {
		if (0 != file)
			fputs(a, file);

		return *this;
	}

	out& operator <<(const std::string& a) {
		if (0 != file)
			fputs(a.c_str(), file);

		return *this;
	}

	out& operator <<(const setw& arg) {
		width = arg.width;
		return *this;
	}

	out& operator <<(const setfill& arg) {
		fillchar = arg.fillchar;
		return *this;
	}

	out& operator <<(const NumericBaseFuncId id) {

		if (stream::dec == id) {
			base = BASE_DEC;
		}
		else
		if (stream::hex == id) {
			base = BASE_HEX;
		}
		else
		if (stream::oct == id) {
			base = BASE_OCT;
		}
		else {
			assert(0);
		}

		return *this;
	}

	out& operator <<(const TerminatorFuncId id) {

		if (0 == file)
			return *this;

		if (stream::endl == id) {
			putc('\n', file);
		}
		else
		if (stream::ends == id) {
			putc('\0', file);
		}
		else
		if (stream::flush == id) {
			fflush(file);
		}
		else {
			assert(0);
		}

		return *this;
	}
};

extern in cin;
extern out cout;
extern out cerr;

} // namespace stream

#endif // stream_H__
