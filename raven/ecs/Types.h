#pragma once

#include "helpers/Murmur3.h"

namespace Ecs
{
	//We will need to register components

	using IndexType = uint32_t;
	using IdType = uint64_t;

	//These structs are only really used to get some information, hence they are called 'GetXXX'

	template <typename Type>
	struct GetTypeIndex
	{
		//Returns the sequential instance identifier of a given component.
		static IndexType value()
		{
			static IndexType value {};
			return value++;
		}
	};


	template <typename Type>
	struct GetTypeInfo
	{
		static constexpr IdType id()
		{
			//https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros?view=vs-2019
			constexpr auto value = Murmur3::Hash( __FUNCSIG__ );
			return static_cast<IdType>( value );
		}
	};


	template <typename, typename = void>
	struct has_type_index : std::false_type
	{
	};


	template <typename Type>
	struct has_type_index<Type, std::void_t<decltype( GetTypeIndex<Type>::value() )>> : std::true_type
	{
	};


	template <typename Type>
	inline constexpr bool has_type_index_v = has_type_index<Type>::value;


}
