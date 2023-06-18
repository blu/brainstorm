#ifndef util_file_H__
#define util_file_H__

namespace testbed
{

char*
get_buffer_from_file(
	const char* const filename,
	size_t& length);

} // namespace testbed

#endif // util_file_H__
