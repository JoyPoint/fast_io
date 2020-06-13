#pragma once

namespace fast_io
{

template<buffer_input_stream src,dynamic_buffer_output_stream indire,typename func>
class basic_indirect_ibuffer
{
public:
	using source_buffer_type = src;
	using indirect_buffer_type = indire;
	using function_type = func;
	using char_type = typename indirect_buffer_type::char_type;
	source_buffer_type source;
	indirect_buffer_type indirect;
	function_type function;
	std::size_t current_position{};
};

template<buffer_input_stream src,dynamic_buffer_output_stream indire,typename function>
inline constexpr auto ibuffer_begin(basic_indirect_ibuffer<src,indire,function>& in)
{
	return obuffer_begin(in.indirect);
}

template<buffer_input_stream src,dynamic_buffer_output_stream indire,typename function>
inline constexpr auto ibuffer_curr(basic_indirect_ibuffer<src,indire,function>& in)
{
	return obuffer_begin(in.indirect)+in.current_position;
}

template<buffer_input_stream src,dynamic_buffer_output_stream indire,typename function>
inline constexpr auto ibuffer_end(basic_indirect_ibuffer<src,indire,function>& in)
{
	return obuffer_curr(in.indirect);
}

template<buffer_input_stream src,dynamic_buffer_output_stream indire,typename function>
inline constexpr auto ibuffer_set_curr(basic_indirect_ibuffer<src,indire,function>& in,typename indire::char_type* ptr)
{
	in.current_position=ptr-obuffer_begin(in.indirect);
}

template<buffer_input_stream src,dynamic_buffer_output_stream indire,typename function>
inline constexpr bool underflow(basic_indirect_ibuffer<src,indire,function>& in)
{
	auto curr{ibuffer_curr(in)};
	auto ed{ibuffer_end(in)};
	if(curr==ed)
	{
		if(!underflow(in.source))[[unlikely]]
			return false;
	}
	else
	{
		auto bg{ibuffer_begin(in)};
		std::copy(curr,ed,bg);
		in.current_position={};
		obuffer_set_curr(in.indirect,bg+(ed-curr));
		underflow(in.source);
	}
	ibuffer_set_curr(in.source,in.function(in.indirect,ibuffer_begin(in.source),ibuffer_end(in.source)));
	return true;
}

namespace details::indirect_ibuffer
{
template<typename T,std::contiguous_iterator Iter>
inline constexpr Iter read_cold(T& in,Iter begin,Iter end)
{
	while(underflow(in))
	{
		auto bg{ibuffer_begin(in)};
		auto curr{bg+in.current_position};
		auto ed{ibuffer_end(in)};
		std::size_t to_read{end-begin};
		std::size_t available_in_buffer{ed-curr};
		std::size_t read_this_round(std::min(to_read,available_in_buffer));
		if(std::is_constant_evaluated())
			std::copy_n(curr,read_this_round,begin);
		else
			memcpy(std::to_address(begin),curr,read_this_round*sizeof(*begin));
		begin+=read_this_round;
		if(to_read<=available_in_buffer)[[likely]]
		{
			in.position+=in.current_position;
			break;
		}
	}
	return begin;
}

}

template<buffer_input_stream src,dynamic_buffer_output_stream indire,typename function,std::contiguous_iterator Iter>
requires (std::same_as<std::iter_value_t<Iter>,typename indire::char_type>||std::same_as<typename indire::char_type,char>)
inline constexpr Iter read(basic_indirect_ibuffer<src,indire,function>& in,Iter begin,Iter end)
{
	if constexpr(std::same_as<std::iter_value_t<Iter>,typename indire::char_type>)
	{
		auto bg{ibuffer_begin(in)};
		auto curr{bg+in.current_position};
		auto ed{ibuffer_end(in)};
		std::size_t to_read{end-begin};
		if(ed<curr+to_read)[[unlikely]]
			return details::indirect_ibuffer::read_cold(in,begin,end);
		if (std::is_constant_evaluated())
		{
			std::copy_n(curr,to_read,begin);
			return end;
		}
		else
		{
			memcpy(std::to_address(begin),curr,to_read*sizeof(*begin));
			return end;
		}
	}
	else
	{
		auto b{reinterpret_cast<char*>(std::to_address(begin))};
		auto e{reinterpret_cast<char*>(std::to_address(end))};
		auto it{read(in,b,e)};
		return begin+(it-b)/sizeof(*begin);
	}
}

}