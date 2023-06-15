#pragma once
#include "TransferBase.hpp"
#include "Foundation/NamespeceAlias.h"
#include "Foundation/TypeTraits.hpp"

template<typename T>
struct TransferHelper;

template<>
struct TransferHelper<stdfs::path> {
public:
    template<TransferContextConcept T>
    static void Transfer(T &transfer, std::string_view name, stdfs::path &data) {
	    if constexpr (transfer.IsReading()) {
		    std::string string;
	        transfer.Transfer(name, string);
	        data = string;
	    } else {
		    std::string string = data.string();
			transfer.Transfer(name, string);    
	    }
    }
};

template<Enumerable T>
struct TransferHelper<T> {
	template<TransferContextConcept Transfer>
    static void Transfer(Transfer &transfer, std::string_view name, T &data) {
		using NativeType = typename std::underlying_type<T>::type;
		transfer.transfer(name, static_cast<NativeType &>(data));
    }
};