#pragma once
#include "TransferBase.hpp"
#include "Foundation/NamespeceAlias.h"
#include "Foundation/TypeTraits.hpp"

template<typename T>
struct TransferHelper;

template<>
struct TransferHelper<stdfs::path> {
    static void Read(TransferBase &transfer, std::string_view name, stdfs::path &data) {
        std::string string;
        transfer.Transfer(name, string);
        data = string;
    }
    static void Write(TransferBase &transfer, std::string_view name, stdfs::path &data) {
        std::string string = data.string();
        transfer.Transfer(name, string);
    }
};

template<typename T>
concept InvokeTransferHelperReadConcept = requires(
    T a) { TransferHelper<T>::Read(std::declval<TransferBase &>(), std::string_view{}, a); };

template<typename T>
concept InvokeTransferHelperWriteConcept = requires(
    T a) { TransferHelper<T>::Write(std::declval<TransferBase &>(), std::string_view{}, a); };
