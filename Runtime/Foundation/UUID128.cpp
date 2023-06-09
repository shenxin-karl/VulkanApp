#include "UUID128.h"

auto UUID128::New() -> UUID128 {
    return GetRandomGenerator()();
}

auto UUID128::New(std::string_view name) -> UUID128 {
    return GetNameGenerator()(name);
}

auto UUID128::New(std::wstring_view name) -> UUID128 {
    return GetNameGenerator()(name);
}

UUID128::UUID128(const uuids::uuid &id) : uuid(id) {
}

auto UUID128::GetNameGenerator() -> uuids::uuid_name_generator & {
    static uuids::uuid_name_generator generator(from_string(sClassUUID).value());
    return generator;
}

auto UUID128::GetRandomGenerator() -> uuids::uuid_random_generator & {
    static uuids::uuid_random_generator generator = []() {
        auto currentTime = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(currentTime.time_since_epoch()).count();
        std::mt19937 seed(timestamp);
        return uuids::uuid_random_generator(seed);
    }();
    return generator;
}
