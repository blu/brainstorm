#if defined(__linux__) || defined(__APPLE__)
#include <sys/types.h>
#include <sys/stat.h>
#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "scoped.hpp"
#include "stream.hpp"
#include "util_file.hpp"

namespace testbed
{

template < typename T >
class generic_free
{
public:
	void operator()(T* arg)
	{
		assert(0 != arg);
		free(arg);
	}
};


template <>
class scoped_functor< FILE >
{
public:
	void operator()(FILE* arg)
	{
		assert(0 != arg);
		fclose(arg);
	}
};


#if defined(__linux__) || defined(__APPLE__)
static bool
get_file_size(
	const char* const filename,
	size_t& size)
{
	assert(0 != filename);

	struct stat filestat;

	if (-1 == stat(filename, &filestat))
	{
		stream::cerr << __FUNCTION__ << " failed to stat file '" << filename << "'\n";
		return false;
	}

	if (!S_ISREG(filestat.st_mode))
	{
		stream::cerr << __FUNCTION__ << " encountered non-regular file '" << filename << "'\n";
		return false;
	}

	size = filestat.st_size;
	return true;
}


char*
get_buffer_from_file(
	const char* const filename,
	size_t& length)
{
	assert(nullptr != filename);

	if (!get_file_size(filename, length))
	{
		stream::cerr << __FUNCTION__ << " cannot get size of file '" << filename << "'\n";
		return nullptr;
	}

	const scoped_ptr< FILE, scoped_functor > file(fopen(filename, "r"));

	if (nullptr == file())
	{
		stream::cerr << __FUNCTION__ << " cannot open file '" << filename << "'\n";
		return nullptr;
	}

	scoped_ptr< char, generic_free > source(
		reinterpret_cast< char* >(malloc(length)));

	if (nullptr == source())
	{
		stream::cerr << __FUNCTION__ << " cannot allocate memory for file '" << filename << "'\n";
		return nullptr;
	}

	if (1 != fread(source(), length, 1, file()))
	{
		stream::cerr << __FUNCTION__ << " cannot read from file '" << filename << "'\n";
		return nullptr;
	}

	char* const ret = source();
	source.reset();
	return ret;
}

#else // stat() unavailable
char*
get_buffer_from_file(
	const char* const filename,
	size_t& length)
{
	assert(nullptr != filename);

	const scoped_ptr< FILE, scoped_functor > file(fopen(filename, "r"));

	if (!file())
	{
		stream::cerr << __FUNCTION__ << " cannot open file '" << filename << "'\n";
		return nullptr;
	}

	if (fseek(file(), 0, SEEK_END))
	{
		stream::cerr << __FUNCTION__ << " cannot seek to the end of '" << filename << "'\n";
		return nullptr;
	}

	length = ftell(file());

	if (fseek(file(), 0, SEEK_SET))
	{
		stream::cerr << __FUNCTION__ << " cannot seek to the start of '" << filename << "'\n";
		return nullptr;
	}

	scoped_ptr< char, generic_free > source(
		reinterpret_cast< char* >(malloc(length)));

	if (nullptr == source())
	{
		stream::cerr << __FUNCTION__ << " cannot allocate memory for file '" << filename << "'\n";
		return nullptr;
	}

	if (1 != fread(source(), length, 1, file()))
	{
		stream::cerr << __FUNCTION__ << " cannot read from file '" << filename << "'\n";
		return nullptr;
	}

	char* const ret = source();
	source.reset();
	return ret;
}

#endif
} // namespace testbed
