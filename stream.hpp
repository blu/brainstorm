#ifndef stream_H__
#define stream_H__

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string>

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

	const in& operator >>(int& a) const {
		if (0 != file)
			fscanf(file, "%d", &a);

		return *this;
	}

	const in& operator >>(unsigned& a) const {
		if (0 != file)
			fscanf(file, "%u", &a);

		return *this;
	}

	const in& operator >>(long& a) const {
		if (0 != file)
			fscanf(file, "%ld", &a);

		return *this;
	}

	const in& operator >>(unsigned long& a) const {
		if (0 != file)
			fscanf(file, "%lu", &a);

		return *this;
	}

	const in& operator >>(float& a) const {
		if (0 != file)
			fscanf(file, "%f", &a);

		return *this;
	}

	const in& operator >>(double& a) const {
		if (0 != file)
			fscanf(file, "%lf", &a);

		return *this;
	}

	const in& operator >>(void*& a) const {
		if (0 != file)
			fscanf(file, "%p", &a);

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

class out {
	FILE* file;

public:
	out()
	: file(0) {
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

	const out& operator <<(const char a) const {
		if (0 != file)
			putc(a, file);

		return *this;
	}

	const out& operator <<(const int a) const {
		if (0 != file)
			fprintf(file, "%d", a);

		return *this;
	}

	const out& operator <<(const unsigned a) const {
		if (0 != file)
			fprintf(file, "%u", a);

		return *this;
	}

	const out& operator <<(const long a) const {
		if (0 != file)
			fprintf(file, "%ld", a);

		return *this;
	}

	const out& operator <<(const unsigned long a) const {
		if (0 != file)
			fprintf(file, "%lu", a);

		return *this;
	}

	const out& operator <<(const float a) const {
		if (0 != file)
			fprintf(file, "%f", a);

		return *this;
	}

	const out& operator <<(const double a) const {
		if (0 != file)
			fprintf(file, "%f", a);

		return *this;
	}

	const out& operator <<(const void* const a) const {
		if (0 != file)
			fprintf(file, "%p", a);

		return *this;
	}

	const out& operator <<(const char* const a) const {
		if (0 != file)
			fprintf(file, "%s", a);

		return *this;
	}

	const out& operator <<(const std::string& a) const {
		if (0 != file)
			fprintf(file, "%s", a.c_str());

		return *this;
	}
};

extern in cin;
extern out cout;
extern out cerr;

} // namespace stream

#endif // stream_H__
