#pragma once
#include <string_view>
#include <unordered_map>
#include "Foundation/NonCopyable.h"
#include "Foundation/TypeAlias.h"
#include "Foundation/NamespeceAlias.h"

template<typename T>
struct TransferHelper;

class TransferBase : public NonCopyable {
public:
    TransferBase(const stdfs::path &path) : _sourceFilePath(path) {
    }
    virtual ~TransferBase() {
    }

    virtual void TransferVersion(std::string_view name, int version) {
        _currentVersion = version;
    }

    virtual void Transfer(std::string_view name, bool &data) = 0;

    virtual void Transfer(std::string_view name, uint8 &data) = 0;
    virtual void Transfer(std::string_view name, uint16 &data) = 0;
    virtual void Transfer(std::string_view name, uint32 &data) = 0;
    virtual void Transfer(std::string_view name, uint64 &data) = 0;

    virtual void Transfer(std::string_view name, int8 &data) = 0;
    virtual void Transfer(std::string_view name, int16 &data) = 0;
    virtual void Transfer(std::string_view name, int32 &data) = 0;
    virtual void Transfer(std::string_view name, int64 &data) = 0;

    virtual void Transfer(std::string_view name, float &data) = 0;
    virtual void Transfer(std::string_view name, double &data) = 0;

    virtual void Transfer(std::string_view name, std::string &data) = 0;

    virtual bool BeginTransfer() = 0;
    virtual bool EndTransfer() {
	    _currentVersion = 0;
        _versionMap.clear();
        return true;
    }

    virtual bool IsRead() const = 0;

    void SetVersion(std::string_view name, int version) {
        _versionMap[std::string(name)] = version;
    }
    auto GetVersion(std::string_view name) -> int {
        auto iter = _versionMap.find(name.data());
        if (iter != _versionMap.end()) {
            return iter->second;
        }
        return 0;
    }
protected:
    size_t _currentVersion = 0;
    stdfs::path _sourceFilePath;
    std::unordered_map<std::string, int> _versionMap;
};

template<typename T>
concept TransferContextConcept = std::is_base_of_v<TransferBase, T>;

class TransferJsonReader;
class TransferJsonWriter;

#define DECLARE_SERIALIZER(Type)                                                                                       \
public:                                                                                                                \
    template<TransferContextConcept T>                                                                                 \
    void Transfer(T &transfer);

#define IMPLEMENT_SERIALIZER(Type)                                                                                     \
    template void Type::Transfer<TransferJsonWriter>(TransferJsonWriter & transfer);                                   \
    template void Type::Transfer<TransferJsonReader>(TransferJsonReader & transfer);

#define TRANSFER(obj) transfer.Transfer(#obj, this->obj);

template<TransferContextConcept Transfer, typename T>
bool Serialize(Transfer &transfer, T &object) {
    if (!transfer.BeginTransfer()) {
	    return false;
    }
    object.Transfer(transfer);
    return transfer.EndTransfer();
}