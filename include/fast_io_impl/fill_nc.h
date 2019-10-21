#pragma once

namespace fast_io
{

template<standard_output_stream output>
inline constexpr void fill_nc(output& out,std::size_t count,typename output::char_type const& ch)
{
	for(std::size_t i(0);i!=count;++i)
		out.put(ch);
}

}