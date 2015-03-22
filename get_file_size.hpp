#ifndef get_file_size_H__
#define get_file_size_H__

namespace testbed
{

bool
get_file_size(
	const char* const filename,
	size_t& size);

char*
get_buffer_from_file(
	const char* const filename,
	size_t& length);

} // namespace testbed

#endif // get_file_size_H__
