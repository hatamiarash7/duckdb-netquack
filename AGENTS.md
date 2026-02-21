# DuckDB Netquack

## Project Overview

This extension is designed to simplify working with domains, URIs, and web paths directly within your database queries. Whether you're extracting top-level domains (TLDs), parsing URI components, or analyzing web paths, Netquack provides a suite of intuitive functions to handle all your network tasks efficiently. Built for data engineers, analysts, and developers.

## Main Commands

```bash
GEN=ninja make         # Build the entire project (reconfigures CMake + compiles).
GEN=ninja make test    # Run all tests.
```

### Build Troubleshooting

When adding new `.cpp` files, `GEN=ninja make` must be run first to re-glob sources via CMake. If a direct `ninja -C build/release` fails with undefined references, it means CMake hasn't picked up the new file — run `GEN=ninja make` to reconfigure.

For incremental rebuilds of existing files: `touch <file> && ninja -C build/release`.

## Project Structure

```text
duckdb-netquack/
├── src/
│   ├── functions/               # All extension functions (one .hpp + .cpp per function)
│   ├── include/                 # Shared headers
│   ├── utils/                   # Utility modules (url_helpers.hpp, etc.)
│   └── netquack_extension.cpp   # Main extension entry point — registers all functions
├── test/
│   ├── data/                    # Predefined test data files (e.g., CSVs)
│   └── sql/                     # SQL test scripts using sqllogictest format
├── docs/                        # GitBook documentation
│   ├── SUMMARY.md               # Navigation/table of contents (must be updated for new pages)
│   └── functions/               # One .md per function
└── README.md                    # Project overview, usage examples, and roadmap
```

## Token Efficiency

- Never re-read files you just wrote or edited. You know the contents.
- Never re-run commands to "verify" unless the outcome was uncertain.
- Don't echo back large blocks of code or file contents unless asked.
- Batch related edits into single operations. Don't make 5 edits when 1 handles it.
- Skip confirmations like "I'll continue..."  Just do it.
- If a task needs 1 tool call, don't use 3. Plan before acting.
- Do not summarize what you just did unless the result is ambiguous or you need additional input.

## Architecture Patterns

### Adding a New Scalar Function

Each function follows a consistent pattern. For a function named `my_function`:

1. **Header** — `src/functions/my_function.hpp`

```cpp
// Copyright 2026 Arash Hatami

#pragma once

#include "duckdb.hpp"

namespace duckdb {
void MyFunctionFunction(DataChunk &args, ExpressionState &state, Vector &result);

namespace netquack {
std::string MyFunction(const std::string &input);
} // namespace netquack
} // namespace duckdb
```

1. **Implementation** — `src/functions/my_function.cpp`
   - The DuckDB wrapper (`MyFunctionFunction`) iterates over the input chunk, handles NULLs, and delegates to the pure logic function (`netquack::MyFunction`).
   - Uses `url_helpers.hpp` utilities like `find_first_symbols<'#'>()` for fast character scanning.
   - On parse failure, return an error string or the original input (never throw back to DuckDB).

2. **Registration** — `src/netquack_extension.cpp`
   - Add `#include "functions/my_function.hpp"` (includes are alphabetically ordered).
   - Register as `ScalarFunction("my_function", {LogicalType::VARCHAR}, LogicalType::VARCHAR, MyFunctionFunction)`.
   - Place registration before `netquack_version` (which is always last).

3. **Tests** — `test/sql/my_function.test`
   - Uses DuckDB's sqllogictest format.
   - Must start with the 3-line header and `require netquack`.
   - Also add a NULL test case to `test/sql/null_handling.test`.

4. **Documentation**
   - Add usage examples to `README.md` (ToC entry + new section + roadmap checkbox).
   - Create `docs/functions/my-function.md` with GitBook frontmatter.
   - Add entry to `docs/SUMMARY.md`.

### Adding a Table Function

Table functions (e.g., `ipcalc`, `extract_query_parameters`, `netquack_version`) use a different pattern with Bind/InitLocal/Function callbacks and `in_out_function`. See `extract_query.hpp` or `ipcalc.hpp` for reference.

## Function Types & Signatures

| Type                       | Example                              | Return                |
| -------------------------- | ------------------------------------ | --------------------- |
| Scalar `VARCHAR → VARCHAR` | `extract_domain`, `extract_fragment` | String result         |
| Scalar `VARCHAR → BOOLEAN` | `is_valid_ip`, `is_private_ip`       | Boolean result        |
| Scalar `VARCHAR → UBIGINT` | `ip_to_int`                          | Integer result        |
| Scalar `VARCHAR → TINYINT` | `ip_version`                         | Small integer result  |
| Table (in_out)             | `extract_query_parameters`, `ipcalc` | Multiple rows/columns |

## Test Format (sqllogictest)

```text
# name: test/sql/my_function.test
# description: test netquack my_function
# group: [sql]

require netquack

# Section header comment
query I
SELECT my_function('input');
----
expected_output

# NULL returns NULL
query I
SELECT my_function(NULL);
----
NULL

# Empty string
query I
SELECT my_function('');
----
(empty)

# Table-level test
statement ok
CREATE OR REPLACE TABLE test_data AS SELECT 'value' AS col;

query II
SELECT col, my_function(col) FROM test_data;
----
value expected

statement ok
DROP TABLE test_data;
```

Key test conventions:

- Use `(empty)` for empty string results.
- Use `NULL` for NULL results.
- Use tab characters (`\t`) to separate columns in multi-column results.
- NULL sorts **last** in DuckDB `ORDER BY` (not first).
- Group tests by category with `# ===` comment separators.
- Cover: basic usage, edge cases (empty, NULL, no scheme), special characters, table usage with GROUP BY.

## Documentation Format (GitBook)

Every function doc in `docs/functions/` uses this frontmatter:

```markdown
---
layout:
  title:
    visible: true
  description:
    visible: false
  tableOfContents:
    visible: true
  outline:
    visible: true
  pagination:
    visible: true
---

# Function Name

Description of what the function does.

\```sql
D SELECT function_name('input') AS result;
┌─────────┐
│ result  │
│ varchar │
├─────────┤
│ output  │
└─────────┘
\```
```

## Key Utilities

- **`src/utils/url_helpers.hpp`** — Fast character scanning (`find_first_symbols<'?', '#'>()`), scheme detection (`getURLScheme()`), host validation (`checkAndReturnHost()`).
- **`src/include/`** — Shared headers across functions.

## Coding Conventions

- C++17 standard.
- Namespace: all code lives under `duckdb` namespace, with pure logic in `duckdb::netquack`.
- NULL handling: always check `value.IsNull()` and call `result_validity.SetInvalid(i)`.
- Use `StringVector::AddString(result, str)` to return string results.
- Headers use `#pragma once`.

## License Header (Required)

Every `.cpp` and `.hpp` file must start with:

```cpp
// Copyright 2026 Arash Hatami
```

## Roadmap

Check `README.md` for the full roadmap. Items marked `- [ ]` are open, `- [x]` are completed. When implementing a roadmap item, remove it from the README.
