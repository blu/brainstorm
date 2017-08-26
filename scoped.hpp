#ifndef scoped_H__
#define scoped_H__

namespace testbed
{

////////////////////////////////////////////////////////////////////////////////////////////////////
// scoped_ptr and scoped_linkage_ptr provide end-of-scope actions on arbitrary and external linkage
// pointers, respectively. In addition, scoped_ptr is resettable, while scoped_linkage_ptr comes at
// zero memory overhead.
////////////////////////////////////////////////////////////////////////////////////////////////////

class non_copyable
{
	non_copyable(const non_copyable&) {}
	non_copyable& operator =(const non_copyable&) { return *this; }

public:
	non_copyable() {}
};


template <
	typename T,
	template< typename F > class FTOR_T,
	T* PTR_T >
class scoped_linkage_ptr : non_copyable, FTOR_T< T >
{
public:
	~scoped_linkage_ptr()
	{
		FTOR_T< T >::operator()(PTR_T);
	}
};


template <
	typename T,
	template< typename F > class FTOR_T >
class scoped_ptr : non_copyable, FTOR_T< T >
{
	T* m;

public:
	scoped_ptr()
	: m(0)
	{}

	explicit scoped_ptr(T* arg)
	: m(arg)
	{}

	~scoped_ptr()
	{
		if (0 != m)
			FTOR_T< T >::operator()(m);
	}

	void reset()
	{
		m = 0;
	}

	void swap(scoped_ptr& oth)
	{
		T* const t = m;
		m = oth.m;
		oth.m = t;
	}

	T* operator ()() const
	{
		return m;
	}
};


template < typename T >
class scoped_functor
{
public:
	void operator()(T*);
};


typedef bool (deinit_resources_t)();

template <>
class scoped_functor< deinit_resources_t >
{
public:
	void operator()(deinit_resources_t* arg)
	{
		arg();
	}
};


template < typename T >
class generic_delete
{
public:

	void operator()(T* arg)
	{
		delete arg;
	}
};


template < typename T >
class generic_delete_arr
{
public:

	void operator()(T* arg)
	{
		delete [] arg;
	}
};

} // namespace testbed

#endif // scoped_H__
