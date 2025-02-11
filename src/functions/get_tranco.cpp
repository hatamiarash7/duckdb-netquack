#include "get_tranco.hpp"
#include "../utils/utils.hpp"

#include <regex>

namespace duckdb
{
	namespace netquack
	{
		// Function to update the Tranco list table
		void UpdateTrancoListFunction(DataChunk &args, ExpressionState &state, Vector &result)
		{
			// Extract the force_download argument
			auto &force_download_vector = args.data[0];
			bool force_download = force_download_vector.GetValue(0).GetValue<bool>();

			// Load the Tranco list into the database
			auto &db = *state.GetContext().db;
			netquack::LoadTrancoList(db, force_download);

			result.SetValue(0, Value("Tranco list updated"));
		}

		// Function to get the Tranco rank of a domain
		void GetTrancoRankFunction(DataChunk &args, ExpressionState &state, Vector &result)
		{
			auto &db = *state.GetContext().db;
			netquack::LoadPublicSuffixList(db, false);
			Connection con(db);

			auto table_exists = con.Query("SELECT 1 FROM information_schema.tables WHERE table_name = 'tranco_list'");

			if (table_exists->RowCount() == 0)
			{
				throw std::runtime_error("Tranco table not found. Download it first using `SELECT update_tranco(true);`");
			}

			auto &domain_vector = args.data[0];
			auto domain = domain_vector.GetValue(0).ToString();

			auto query = "SELECT rank FROM tranco_list WHERE domain = '" + domain + "'";
			auto query_result = con.Query(query);

			result.SetValue(0, query_result->RowCount() > 0 ? query_result->GetValue(0, 0) : Value());
		}
	}
}
