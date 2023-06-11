#include "TransferJsonWriter.h"
#include "Foundation/Exception.h"
#include <fstream>
#include <json/writer.h>


void TransferJsonWriter::TransferVersion(std::string_view name, int version) {
	TransferBase::TransferVersion(name, version);
	SetVersion(name, version);
}

bool TransferJsonWriter::BeginTransfer() {
	_stack.emplace_back();
	return true;
}

bool TransferJsonWriter::EndTransfer() {
	std::ofstream output(_sourceFilePath);
	Exception::CondThrow(output.is_open(), "can't open the file!");
	Exception::CondThrow(_stack.size() == 1, "invalid stack state");

	Json::Value versionObject = Json::ValueType::objectValue;
	for (auto &&[key, version] : _versionMap) {
		Json::Value object;
		object["Name"] = key;
		object["Version"] = version;
		versionObject.append(object);
	}
	_stack.front()["__VersionMap"] = versionObject;

	Json::StreamWriterBuilder builder;
	std::unique_ptr<Json::StreamWriter> pWriter(builder.newStreamWriter());
	pWriter->write(_stack.front(), &output);

	TransferBase::EndTransfer();
	return true;
}

void TransferJsonWriter::PushJsonValue(Json::ValueType type) {
	_stack.emplace_back(type);
}

auto TransferJsonWriter::PopJsonValue() -> Json::Value {
	ExceptionAssert(_stack.size() >= 2);
	auto obj = std::move(_stack.back());
	_stack.pop_back();
	return obj;
}

auto TransferJsonWriter::GetCurrentJsonValue() -> Json::Value & {
	return _stack.back();
}
