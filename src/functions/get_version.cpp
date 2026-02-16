// Copyright 2025 Arash Hatami

#include "get_version.hpp"

#include <future>
#include <iostream>
#include <queue>
#include <sstream>
#include <string>

namespace duckdb {
namespace netquack {
struct VersionLocalState : public LocalTableFunctionState {
	std::atomic_bool done {false};
};

unique_ptr<FunctionData> VersionFunc::Bind(ClientContext &context, TableFunctionBindInput &input,
                                           vector<LogicalType> &return_types, vector<string> &names) {
	// 0. version: version of the extension.
	return_types.emplace_back(LogicalType(LogicalTypeId::VARCHAR));
	names.emplace_back("version");
	return make_uniq<TableFunctionData>();
}

unique_ptr<LocalTableFunctionState> VersionFunc::InitLocal(ExecutionContext &context, TableFunctionInitInput &input,
                                                           GlobalTableFunctionState *global_state_p) {
	return make_uniq<VersionLocalState>();
}

unique_ptr<GlobalTableFunctionState> VersionFunc::InitGlobal(ClientContext &context, TableFunctionInitInput &input) {
	return nullptr;
}

void VersionFunc::Scan(ClientContext &context, TableFunctionInput &data_p, DataChunk &output) {
	// Check done
	if (((VersionLocalState &)*data_p.local_state).done) {
		return;
	}

	output.SetCardinality(1);
	// Set version
	output.data[0].SetValue(0, "v1.8.1");
	// Set done
	auto &local_state = (VersionLocalState &)*data_p.local_state;
	local_state.done = true;
}
} // namespace netquack
} // namespace duckdb
