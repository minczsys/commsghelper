#pragma once
 
#include<cstdint>
#include <sstream>
#include <string>
#include <type_traits>
#include <tuple>
#include <cctype>


#define ComSplitter "!@#$%*:,\r\n"
#define HighlevelSplitter ','

namespace alpaca{
	namespace commsg{
		class string_tokenizer{
		private:
			const char* buffer;
			unsigned pos;
			unsigned length;
		public:
			static bool is_com_splitter(char token)
			{
				const char* s = ComSplitter;
				for (int i = 0; s[i]; ++i)
					if (s[i] == token)
						return true;
				return false;
            }
			static bool is_com_splitter(...)
			{
				return false;
			}
		public:
			string_tokenizer(const char* b, unsigned offset, unsigned len) : buffer(b), pos(offset), length(len){}
			string_tokenizer(std::string& source, unsigned from) : string_tokenizer(source.c_str(), from, source.size()){}
			string_tokenizer(std::string& source) : string_tokenizer(source.c_str(), 0, source.size()){}

			void expect_token(const char* token)
			{
				while (pos < length && isblank(buffer[pos])) ++pos;
				int j = 0;
				while (pos < length && token[j] != 0 && buffer[pos] == token[j])
					++pos; ++j;
				if (token[j] != 0)
				{
					//TODO: 错误记录
				}
			}
			void expect_token(char token)
			{
				while (pos < length && isblank(buffer[pos])) ++pos;
				if (pos < length && buffer[pos] == token)
				{
					++pos;
				}
				else
				{
					//TODO: 错误记录
				}
			}
			char peek_next() const
			{
				unsigned i = pos;
				while (i < length && isblank(buffer[i])) ++i;
				return buffer[i];
			}
			void walk_until(char token)
			{
				while (pos < length&&buffer[pos] != token) ++pos;
				if (pos < length && buffer[pos] == token)
				{
					return;
				}
				else
				{
					//TODO: 错误记录
				}
			}
			void find_com_splitter()
			{
				while (pos < length && is_com_splitter(buffer[pos]) && buffer[pos] != HighlevelSplitter) ++pos;
				return;
			}
			void walk_until_com_splitter()
			{
				while (pos < length&& !is_com_splitter(buffer[pos])) ++pos;
				return;
			}
			unsigned tell() const { return pos; }
			void forward() { ++pos; }
			void assign(unsigned from,unsigned to,std::string& out)
			{
				out.assign(buffer + from, to - from);
			}
		};

		template<typename> class com_forwarder;

#define ALPACA_BASE_TYPE_COM_STATE(TName) \
	inline void com_encode(std::stringstream& out,const TName& t) \
			{\
		out << t;\
			}\
	inline void com_decode(string_tokenizer& in,TName& t) \
			{\
		int start_pos = in.tell();\
		in.walk_until_com_splitter();\
		if(start_pos == in.tell()){\
			in.find_com_splitter();\
		}\
		std::string string_data;\
		in.assign(start_pos,in.tell(),string_data);\
		std::istringstream is(string_data);\
		is >> t;\
	}

		ALPACA_BASE_TYPE_COM_STATE(bool)
		ALPACA_BASE_TYPE_COM_STATE(float)
		ALPACA_BASE_TYPE_COM_STATE(double)
		ALPACA_BASE_TYPE_COM_STATE(int8_t)
		ALPACA_BASE_TYPE_COM_STATE(int16_t)
		ALPACA_BASE_TYPE_COM_STATE(int32_t)
		ALPACA_BASE_TYPE_COM_STATE(int64_t)
		ALPACA_BASE_TYPE_COM_STATE(uint8_t)
		ALPACA_BASE_TYPE_COM_STATE(uint16_t)
		ALPACA_BASE_TYPE_COM_STATE(uint32_t)
		ALPACA_BASE_TYPE_COM_STATE(uint64_t)
		ALPACA_BASE_TYPE_COM_STATE(std::string)

		inline void com_encode(std::stringstream& out, const char* t)
		{
			out << t; 
		}
		template<typename T>
		class com_forwarder{
		private:
			template<typename C>
			static auto check_com_state(C*)
				-> typename std::is_same<decltype(std::declval<C>().encode_com_state(std::declval<std::stringstream&>())), void>::type;
			
			template<typename>
			static std::false_type check_com_state(...);

			template<typename C>
			static auto p_check_com_state(C*)
				-> typename std::is_same<decltype(std::declval<C>()->encode_com_state(std::declval<std::stringstream&>())),void>::type;
			
			template<typename>
			static std::false_type p_check_com_state(...);

			typedef decltype(check_com_state<T>(0)) has_com_state;
			typedef decltype(p_check_com_state<T>(0)) p_has_com_state;

			static void encode_inner(std::stringstream& out, const T& t, std::false_type, std::false_type)
			{
				com_encode(out,t);
			}
			static void encode_inner(std::stringstream& out, const T& t, std::false_type, std::true_type)
			{
				t->encode_com_state(out);
			}
			static void encode_inner(std::stringstream& out, const T& t, std::true_type, std::false_type)
			{
				t.encode_com_state(out);
			}
			static void encode_inner(std::stringstream& out, const T& t, std::true_type, std::true_type)
			{
				t->encode_com_state(out);
			}

			static void decode_inner(string_tokenizer& in, T& t, std::false_type, std::false_type)
			{
				com_decode(in,t);
			}
			static void decode_inner(string_tokenizer& in, T& t, std::false_type, std::true_type)
			{
				t->decode_com_state(in);
			}
			static void decode_inner(string_tokenizer& in, T& t, std::true_type, std::false_type)
			{
				t.decode_com_state(in);
			}
			static void decode_inner(string_tokenizer& in, T& t, std::true_type, std::true_type)
			{
				t->decode_com_state(in);
			}
		public:
			static void encode(std::stringstream& out, const T& t)
			{
				encode_inner(out, t, has_com_state{}, p_has_com_state{});
			}
			static void decode(string_tokenizer& in, T& t)
			{
				decode_inner(in, t, has_com_state{}, p_has_com_state{});
			}
		};
	}
}

template<typename T>
inline static void COM_ENCODE_ENTRIES(std::stringstream& out, T head)
{
	::alpaca::commsg::com_forwarder<T>::encode(out, head);
}

static bool issplit = false;
template<typename T, typename ...Arg>
inline static void COM_ENCODE_ENTRIES(std::stringstream& out, T head, Arg ...arg)
{
	if (!::alpaca::commsg::string_tokenizer::is_com_splitter(head))
	{
		if (issplit)
			issplit = false;
		else
			out << ",";
	}	
	else
		issplit = true;
	COM_ENCODE_ENTRIES(out, head);
	COM_ENCODE_ENTRIES(out, arg...);
}

#define COM_DECODE_ENTRY(in, prefix, T) do {\
    ::alpaca::commsg::com_forwarder<std::decay<decltype((prefix).T)>::type>::decode(in, (prefix).T); \
} while (0)
#define COM_DECODE_ENTRIES2(in, prefix, T1, T2) COM_DECODE_ENTRY(in, prefix, T1); in.expect_token(','); COM_DECODE_ENTRY(in, prefix, T2)
#define COM_DECODE_ENTRIES3(in, prefix, T1, T2, T3) COM_DECODE_ENTRIES2(in, prefix, T1, T2); in.expect_token(','); COM_DECODE_ENTRY(in, prefix, T3)
#define COM_DECODE_ENTRIES4(in, prefix, T1, T2, T3, T4) COM_DECODE_ENTRIES3(in, prefix, T1, T2, T3); in.expect_token(','); COM_DECODE_ENTRY(in, prefix, T4)
#define COM_DECODE_ENTRIES5(in, prefix, T1, T2, T3, T4, T5) COM_DECODE_ENTRIES4(in, prefix, T1, T2, T3, T4); in.expect_token(','); COM_DECODE_ENTRY(in, prefix, T5)
#define COM_DECODE_ENTRIES6(in, prefix, T1, T2, T3, T4, T5, T6) COM_DECODE_ENTRIES5(in, prefix, T1, T2, T3, T4, T5); in.expect_token(','); COM_DECODE_ENTRY(in, prefix, T6)
#define COM_DECODE_ENTRIES7(in, prefix, T1, T2, T3, T4, T5, T6, T7) COM_DECODE_ENTRIES6(in, prefix, T1, T2, T3, T4, T5, T6); in.expect_token(','); COM_DECODE_ENTRY(in, prefix, T7)
#define COM_DECODE_ENTRIES8(in, prefix, T1, T2, T3, T4, T5, T6, T7, T8) COM_DECODE_ENTRIES7(in, prefix, T1, T2, T3, T4, T5, T6, T7); in.expect_token(','); COM_DECODE_ENTRY(in, prefix, T8)
#define COM_DECODE_ENTRIES9(in, prefix, T1, T2, T3, T4, T5, T6, T7, T8, T9) COM_DECODE_ENTRIES8(in, prefix, T1, T2, T3, T4, T5, T6, T7, T8); in.expect_token(','); COM_DECODE_ENTRY(in, prefix, T9)
#define COM_DECODE_ENTRIES10(in, prefix, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10) COM_DECODE_ENTRIES9(in, prefix, T1, T2, T3, T4, T5, T6, T7, T8, T9); in.expect_token(','); COM_DECODE_ENTRY(in, prefix, T10)
#define COM_DECODE_ENTRIES11(in, prefix, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11) COM_DECODE_ENTRIES10(in, prefix, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10); in.expect_token(','); COM_DECODE_ENTRY(in, prefix, T11)
#define COM_DECODE_ENTRIES12(in, prefix, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12) COM_DECODE_ENTRIES11(in, prefix, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11); in.expect_token(','); COM_DECODE_ENTRY(in, prefix, T12)
#define COM_DECODE_ENTRIES13(in, prefix, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13) COM_DECODE_ENTRIES12(in, prefix, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12); in.expect_token(','); COM_DECODE_ENTRY(in, prefix, T13)
#define COM_DECODE_ENTRIES14(in, prefix, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14) COM_DECODE_ENTRIES13(in, prefix, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13); in.expect_token(','); COM_DECODE_ENTRY(in, prefix, T14)
#define COM_DECODE_ENTRIES15(in, prefix, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13,T14,T15) COM_DECODE_ENTRIES14(in, prefix, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14); in.expect_token(','); COM_DECODE_ENTRY(in, prefix, T15)
#define COM_DECODE_ENTRIES16(in, prefix, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13,T14,T15,T16) COM_DECODE_ENTRIES15(in, prefix, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14，T15); in.expect_token(','); COM_DECODE_ENTRY(in, prefix, T16)
#define COM_DECODE_ENTRIES17(in, prefix, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13,T14, T15,T16,T17) COM_DECODE_ENTRIES16(in, prefix, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14，T15，T16); in.expect_token(','); COM_DECODE_ENTRY(in, prefix, T17)
#define COM_DECODE_ENTRIES18(in, prefix, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13,T14,T15,T16,T17,T18) COM_DECODE_ENTRIES17(in, prefix, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14，T15，T16，T17); in.expect_token(','); COM_DECODE_ENTRY(in, prefix, T18)
#define COM_DECODE_ENTRIES19(in, prefix, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13,T14, T15,T16,T17,T18,T19) COM_DECODE_ENTRIES18(in, prefix, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14，T15，T16，T17，T18); in.expect_token(','); COM_DECODE_ENTRY(in, prefix, T19)
#define COM_DECODE_ENTRIES20(in, prefix, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13,T14, T15,T16,T17,T18,T19,T20) COM_DECODE_ENTRIES19(in, prefix, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14，T15，T16，T17，T18，T19); in.expect_token(','); COM_DECODE_ENTRY(in, prefix, T20)

#define COM_ENTRIES_GET_MACRO(ph1,ph2,ph3,ph4,ph5,ph6,ph7,ph8,ph9,ph10,ph11,ph12,ph13,ph14,ph15,ph16,ph17,ph18,ph19,ph20, NAME, ...) NAME
#define COM_ENTRIES_GET_MACRO_(tuple) COM_ENTRIES_GET_MACRO tuple

#define COM_DECODE_ENTRIES(in, prefix, ...)\
    COM_ENTRIES_GET_MACRO_((__VA_ARGS__,\
        COM_DECODE_ENTRIES20, \
        COM_DECODE_ENTRIES19, \
        COM_DECODE_ENTRIES18, \
        COM_DECODE_ENTRIES17, \
        COM_DECODE_ENTRIES16, \
		COM_DECODE_ENTRIES15, \
        COM_DECODE_ENTRIES14, \
        COM_DECODE_ENTRIES13, \
        COM_DECODE_ENTRIES12, \
        COM_DECODE_ENTRIES11, \
        COM_DECODE_ENTRIES10, \
        COM_DECODE_ENTRIES9, \
        COM_DECODE_ENTRIES8, \
        COM_DECODE_ENTRIES7, \
        COM_DECODE_ENTRIES6, \
        COM_DECODE_ENTRIES5, \
        COM_DECODE_ENTRIES4, \
        COM_DECODE_ENTRIES3, \
        COM_DECODE_ENTRIES2, \
        COM_DECODE_ENTRY)) (in, prefix, __VA_ARGS__);

#define DEFINE_COM_SERIALIZATION(...) \
void encode_com_state(std::stringstream& out) const \
{\
    COM_ENCODE_ENTRIES(out, __VA_ARGS__);\
}\
void decode_com_state(alpaca::commsg::string_tokenizer& in) \
{\
    COM_DECODE_ENTRIES(in ,*this ,__VA_ARGS__);\
}
