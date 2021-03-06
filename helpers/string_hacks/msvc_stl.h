#pragma once

namespace fast_io::details::string_hack
{

template <class _Elem, class _Traits = char_traits<_Elem>, class _Alloc = allocator<_Elem>>
struct model
{
	using _Alty        = std::_Rebind_alloc_t<_Alloc, _Elem>;
	using _Alty_traits = std::allocator_traits<_Alty>;
	using _Scary_val = std::_String_val<std::conditional_t<std::_Is_simple_alloc_v<_Alty>, std::_Simple_types<_Elem>,
		std::_String_iter_types<_Elem, typename _Alty_traits::size_type, typename _Alty_traits::difference_type,
		typename _Alty_traits::pointer, typename _Alty_traits::const_pointer, _Elem&, _Elem const&>>>;	
	using compress_pair_type = std::_Compressed_pair<_Alty, _Scary_val>;
	compress_pair_type _Mypair;
};

template<typename elem,typename traits,typename alloc>
inline constexpr decltype(auto) hack_scary_val(std::basic_string<elem,traits,alloc>& str)
{
	using model_t = model<elem,traits,alloc>;
	using compress_pair_type = typename model_t::compress_pair_type;
	using _Scary_val = typename model_t::_Scary_val;
	return *reinterpret_cast<_Scary_val*>(reinterpret_cast<std::byte*>(std::addressof(str))+offsetof(model_t,_Mypair)+offsetof(compress_pair_type,_Myval2));
}

template<typename T>
inline constexpr bool is_local_and_null(T& str)
{
	decltype(auto) scv{hack_scary_val(str)};
	return !scv._Large_string_engaged()&&!scv._Mysize;
}

template<typename T>
inline constexpr void set_begin_ptr(T& str,typename T::value_type* ptr)
{
	decltype(auto) scv{hack_scary_val(str)};
	scv._Bx._Ptr=ptr;
}

template<typename T>
inline constexpr void set_end_ptr(T& str,typename T::value_type* ptr)
{
	decltype(auto) scv{hack_scary_val(str)};
	scv._Mysize=ptr-scv._Bx._Ptr;
}

template<typename T>
inline constexpr void set_cap_ptr(T& str,typename T::value_type* ptr)
{
	decltype(auto) scv{hack_scary_val(str)};
	scv._Myres=ptr-scv._Bx._Ptr;
}


}