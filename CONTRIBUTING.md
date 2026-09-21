# Contributing to DuckDB Netquack

Thanks for wanting to contribute. Netquack is a [DuckDB community extension](https://duckdb.org/community_extensions/extensions/netquack.html) for domains, URIs, IP addresses, and related network helpers. Bug reports, new functions, tests, and documentation are all welcome.

This guide covers the day-to-day workflow. Architecture, function checklists, and test conventions live in [`AGENTS.md`](AGENTS.md).

## Code of Conduct

By participating you agree to follow the [Code of Conduct](CODE_OF_CONDUCT.md).

## Getting started

### Prerequisites

- **C++17** compiler (`g++` or `clang++`)
- **CMake**, **GNU Make**, **Ninja** (recommended), and **ccache** (optional, speeds rebuilds)
- **gperf** (Public Suffix List perfect-hash generation)
- **Python 3** with `clang-format==11.0.1` (for `make format`)
- **[vcpkg](https://vcpkg.io/en/getting-started)** for `libcurl`

```bash
# Debian / Ubuntu
sudo apt-get install gperf cmake make ninja-build ccache g++ python3

# macOS (Homebrew)
brew install gperf cmake make ninja ccache python
```

Install the formatter DuckDB CI expects:

```bash
python3 -m pip install clang-format==11.0.1
```

### Clone with submodules

Netquack vendors DuckDB and `extension-ci-tools` as git submodules. Clone them together:

```bash
git clone --recurse-submodules git@github.com:hatamiarash7/duckdb-netquack.git
cd duckdb-netquack
```

If you already cloned without `--recurse-submodules`:

```bash
git submodule update --init --recursive
```

### Configure vcpkg

```bash
git clone https://github.com/Microsoft/vcpkg.git
./vcpkg/scripts/bootstrap.sh -disableMetrics
export VCPKG_TOOLCHAIN_PATH="$(pwd)/vcpkg/scripts/buildsystems/vcpkg.cmake"
```

`VCPKG_TOOLCHAIN_PATH` must be set in the shell that runs `make`.

### Build and test

```bash
make help                 # list common targets
GEN=ninja make            # release build (reconfigures CMake + compiles)
GEN=ninja make test       # run sqllogictest suite
make run                  # interactive DuckDB CLI with the local extension
```

Ninja is strongly recommended. The first build compiles DuckDB itself and is slow; later incremental builds are much faster.

When you add a new `.cpp` file, run `GEN=ninja make` so CMake re-globs sources. A direct `ninja -C build/release` will fail with undefined references until CMake has picked the file up.

### Load the local extension

```sql
LOAD './build/release/extension/netquack/netquack.duckdb_extension';
SELECT * FROM netquack_version();
```

`make run` starts `./build/release/duckdb -unsigned` and prints the `LOAD` path for you.

## Development workflow

1. [Fork](https://github.com/hatamiarash7/duckdb-netquack/fork) the repository and create a branch from `main`.
2. Make a focused change (one function, one bug, or one docs fix).
3. Add or update tests under `test/sql/`.
4. Format and run tests:

   ```bash
   make format
   GEN=ninja make test
   ```

5. Open a pull request. Describe **why** the change exists and link any related issue.

Run a single test file while iterating:

```bash
make test-one TEST=test/sql/extract_domain.test
```

## Adding a function

New scalar and table functions follow a fixed pattern. The full checklist is in [`AGENTS.md`](AGENTS.md). In short:

| Step | Where |
| ---- | ----- |
| Header + implementation | `src/functions/<name>.hpp` / `.cpp` |
| Register with `Register()` | `src/netquack_extension.cpp` (includes alphabetically; `netquack_version` last) |
| SQL tests | `test/sql/<name>.test` |
| NULL case | `test/sql/null_handling.test` |
| Catalog metadata assertion | `test/sql/function_descriptions.test` |
| README example | `README.md` |
| GitBook page | `docs/functions/<name>.md` + `docs/SUMMARY.md` |

Every function must ship a `FunctionDescription` (parameter names, one-sentence description, copy-pasteable example, category) so it appears in `duckdb_functions()`. Use the local `Register()` helpers — do not call `loader.RegisterFunction` directly.

On parse failure, return an error string or the original input. Do not throw back into DuckDB.

## Tests

Tests use DuckDB's [sqllogictest](https://duckdb.org/dev/sqllogictest/intro) format. Each file should start with:

```text
# name: test/sql/my_function.test
# description: test netquack my_function
# group: [sql]

require netquack
```

Conventions:

- `(empty)` for empty-string results, `NULL` for SQL NULL.
- Tab-separate columns in multi-column expected output.
- NULL sorts **last** in DuckDB `ORDER BY`.
- Cover the happy path, empty input, NULL, missing scheme, special characters, and table usage.

## Documentation

- User-facing examples belong in `README.md` and `docs/functions/`.
- GitBook pages need the standard frontmatter (see [`AGENTS.md`](AGENTS.md)).
- Add new pages to `docs/SUMMARY.md` or they will not show up in the docs site.
- Keep the in-catalog description aligned with README / GitBook.

## Code style

- C++17. Tabs for indentation, 120-column line limit (see [`.editorconfig`](.editorconfig) and [`.clang-format`](.clang-format)).
- All code lives under `namespace duckdb`, with pure logic in `duckdb::netquack`.
- Headers use `#pragma once`.
- Every `.cpp` / `.hpp` file starts with `// Copyright 2026 Arash Hatami`.
- Handle SQL NULL with `value.IsNull()` and `result_validity.SetInvalid(i)`.
- Return strings with `StringVector::AddString(result, str)`.

```bash
make format          # apply clang-format
make format-check    # CI-style check, no writes
```

## Commits and pull requests

- Prefer conventional commits: `feat:`, `fix:`, `docs:`, `test:`, `chore:`, `refactor:`.
- Keep pull requests small and reviewable. Split unrelated changes.
- Fill in the pull-request template (summary, type of change, checklist).
- Do not commit secrets, local DuckDB files, or `public_suffix_list.dat`.
- CI must pass (`format` + extension distribution) before merge.

## Updating the Public Suffix List

TLD lookup is compiled in at build time from Mozilla's list via gperf:

```bash
make update-tld
```

That regenerates `src/utils/tld_lookup.gperf` and `src/utils/tld_lookup_generated.hpp`. Commit both files together.

## Reporting issues

Use the [issue templates](https://github.com/hatamiarash7/duckdb-netquack/issues/new/choose). Include Netquack and DuckDB versions:

```sql
SELECT * FROM netquack_version();
```

```bash
duckdb --version
```

Security issues should be reported privately — see [SECURITY.md](SECURITY.md).

## Useful Make targets

| Target | Purpose |
| ------ | ------- |
| `make` / `make release` | Release build |
| `GEN=ninja make` | Faster Ninja-backed build |
| `make debug` | Debug build |
| `make test` | Run all tests (release) |
| `make test-one TEST=...` | Run one `.test` file |
| `make run` | Interactive local DuckDB shell |
| `make format` | Format `src/` and `test/` |
| `make clean` | Remove build artifacts |
| `make update-tld` | Refresh the Public Suffix List hash |
| `make help` | Print this list |

See [`docs/getting-started/how-to-build.md`](docs/getting-started/how-to-build.md) for vcpkg and platform notes.
