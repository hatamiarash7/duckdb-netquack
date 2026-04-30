// Copyright 2025 Arash Hatami

#include "get_version.hpp"

namespace duckdb::netquack {
struct VersionLocalState : public LocalTableFunctionState {
	std::atomic_bool done {false};
};

unique_ptr<FunctionData> VersionFunc::Bind(ClientContext &, TableFunctionBindInput &, vector<LogicalType> &return_types,
                                           vector<string> &names) {
	// 0. version: version of the extension.
	return_types.emplace_back(LogicalTypeId::VARCHAR);
	names.emplace_back("version");
	return make_uniq<TableFunctionData>();
}

unique_ptr<LocalTableFunctionState> VersionFunc::InitLocal(ExecutionContext &, TableFunctionInitInput &,
                                                           GlobalTableFunctionState *) {
	return make_uniq<VersionLocalState>();
}

unique_ptr<GlobalTableFunctionState> VersionFunc::InitGlobal(ClientContext &, TableFunctionInitInput &) {
	return nullptr;
}

void VersionFunc::Scan(ClientContext &, TableFunctionInput &data_p, DataChunk &output) {
	// Check done
	auto &local_state = dynamic_cast<VersionLocalState &>(*data_p.local_state);
	if (local_state.done) {
		return;
	}

	output.SetCardinality(1);
	// Set version
	output.data[0].SetValue(0, "v1.12.0");
	// Set done
	local_state.done = true;
}
} // namespace duckdb::netquack
