// Copyright 2026 Arash Hatami

#pragma once

// Source compatibility between DuckDB 1.5 and DuckDB 2.0.
//
// DuckDB 2.0 moved the vector helpers (FlatVector, StringVector, ...) into their own headers, made
// FlatVector::GetData and FlatVector::Validity read-only (GetDataMutable and ValidityMutable write), returns the
// children of a struct vector as vectors instead of pointers, removed the ExecuteWithNulls variants of the executors
// (lambdas return an optional instead), made table function column names Identifiers and moved a function's argument
// types into its signature. The helpers below compile against both.

#include "duckdb.hpp"
#include "duckdb/common/vector_operations/binary_executor.hpp"
#include "duckdb/common/vector_operations/ternary_executor.hpp"
#include "duckdb/common/vector_operations/unary_executor.hpp"

#if __has_include("duckdb/common/vector/flat_vector.hpp")
#define NETQUACK_DUCKDB_V2 1
#include "duckdb/common/vector/constant_vector.hpp"
#include "duckdb/common/vector/flat_vector.hpp"
#include "duckdb/common/vector/list_vector.hpp"
#include "duckdb/common/vector/string_vector.hpp"
#include "duckdb/common/vector/struct_vector.hpp"
#endif

namespace duckdb {
namespace netquack_compat {

//! Writable data of a flat vector
template <class T>
inline T *MutableData(Vector &vector) {
#ifdef NETQUACK_DUCKDB_V2
	return FlatVector::GetDataMutable<T>(vector);
#else
	return FlatVector::GetData<T>(vector);
#endif
}

//! Writable validity of a flat vector
inline ValidityMask &MutableValidity(Vector &vector) {
#ifdef NETQUACK_DUCKDB_V2
	return FlatVector::ValidityMutable(vector);
#else
	return FlatVector::Validity(vector);
#endif
}

//! The child vector of a struct vector, from StructVector::GetEntries()
template <class ENTRIES>
inline Vector &StructChild(ENTRIES &entries, idx_t index) {
#ifdef NETQUACK_DUCKDB_V2
	return entries[index];
#else
	return *entries[index];
#endif
}

//! Column names of a table function's bind
#ifdef NETQUACK_DUCKDB_V2
using NameList = vector<Identifier>;
#else
using NameList = vector<string>;
#endif

//! The argument types of a function
template <class FUNCTION>
vector<LogicalType> ArgumentTypes(const FUNCTION &function) {
#ifdef NETQUACK_DUCKDB_V2
	vector<LogicalType> types;
	for (auto &parameter : function.GetSignature().GetParameters()) {
		types.push_back(parameter.GetType());
	}
	return types;
#else
	return function.arguments;
#endif
}

// UnaryExecutor::ExecuteWithNulls and its binary and ternary versions: lambdas which get the result's validity mask
// and the row index to set NULLs. DuckDB 2.0 removed them, there these loops do the same.

template <class A, class R, class FUNC>
void UnaryExecuteWithNulls(Vector &a, Vector &result, idx_t count, FUNC fun) {
#ifdef NETQUACK_DUCKDB_V2
	UnifiedVectorFormat fa;
	a.ToUnifiedFormat(count, fa);
	auto da = UnifiedVectorFormat::GetData<A>(fa);
	result.SetVectorType(VectorType::FLAT_VECTOR);
	auto out = MutableData<R>(result);
	auto &mask = MutableValidity(result);
	for (idx_t i = 0; i < count; i++) {
		auto ia = fa.sel->get_index(i);
		if (!fa.validity.RowIsValid(ia)) {
			mask.SetInvalid(i);
			continue;
		}
		out[i] = fun(da[ia], mask, i);
	}
#else
	UnaryExecutor::ExecuteWithNulls<A, R>(a, result, count, fun);
#endif
}

template <class A, class B, class R, class FUNC>
void BinaryExecuteWithNulls(Vector &a, Vector &b, Vector &result, idx_t count, FUNC fun) {
#ifdef NETQUACK_DUCKDB_V2
	UnifiedVectorFormat fa, fb;
	a.ToUnifiedFormat(count, fa);
	b.ToUnifiedFormat(count, fb);
	auto da = UnifiedVectorFormat::GetData<A>(fa);
	auto db = UnifiedVectorFormat::GetData<B>(fb);
	result.SetVectorType(VectorType::FLAT_VECTOR);
	auto out = MutableData<R>(result);
	auto &mask = MutableValidity(result);
	for (idx_t i = 0; i < count; i++) {
		auto ia = fa.sel->get_index(i);
		auto ib = fb.sel->get_index(i);
		if (!fa.validity.RowIsValid(ia) || !fb.validity.RowIsValid(ib)) {
			mask.SetInvalid(i);
			continue;
		}
		out[i] = fun(da[ia], db[ib], mask, i);
	}
#else
	BinaryExecutor::ExecuteWithNulls<A, B, R>(a, b, result, count, fun);
#endif
}

template <class A, class B, class C, class R, class FUNC>
void TernaryExecuteWithNulls(Vector &a, Vector &b, Vector &c, Vector &result, idx_t count, FUNC fun) {
#ifdef NETQUACK_DUCKDB_V2
	UnifiedVectorFormat fa, fb, fc;
	a.ToUnifiedFormat(count, fa);
	b.ToUnifiedFormat(count, fb);
	c.ToUnifiedFormat(count, fc);
	auto da = UnifiedVectorFormat::GetData<A>(fa);
	auto db = UnifiedVectorFormat::GetData<B>(fb);
	auto dc = UnifiedVectorFormat::GetData<C>(fc);
	result.SetVectorType(VectorType::FLAT_VECTOR);
	auto out = MutableData<R>(result);
	auto &mask = MutableValidity(result);
	for (idx_t i = 0; i < count; i++) {
		auto ia = fa.sel->get_index(i);
		auto ib = fb.sel->get_index(i);
		auto ic = fc.sel->get_index(i);
		if (!fa.validity.RowIsValid(ia) || !fb.validity.RowIsValid(ib) || !fc.validity.RowIsValid(ic)) {
			mask.SetInvalid(i);
			continue;
		}
		out[i] = fun(da[ia], db[ib], dc[ic], mask, i);
	}
#else
	TernaryExecutor::ExecuteWithNulls<A, B, C, R>(a, b, c, result, count, fun);
#endif
}

} // namespace netquack_compat
} // namespace duckdb
