PROJ_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

# Configuration of extension
EXT_NAME=netquack
EXT_CONFIG=${PROJ_DIR}extension_config.cmake

# Include the Makefile from extension-ci-tools
include extension-ci-tools/makefiles/duckdb_extension.Makefile

# ---------------------------------------------------------------------------
# Local convenience targets
#
# Inherited from extension-ci-tools (see that Makefile for full details):
#   all/release, debug, reldebug, relassert, test, test_debug, test_reldebug,
#   format, format-check, tidy-check, clangd, clean, update, pull, wasm_*
# ---------------------------------------------------------------------------

EXT_RELEASE_BIN := $(PROJ_DIR)build/release/extension/netquack/netquack.duckdb_extension
EXT_DEBUG_BIN := $(PROJ_DIR)build/debug/extension/netquack/netquack.duckdb_extension
DUCKDB_RELEASE := $(PROJ_DIR)build/release/duckdb
DUCKDB_DEBUG := $(PROJ_DIR)build/debug/duckdb

.PHONY: help build run shell test-one debug-run update-tld benchmark

help:
	@echo "DuckDB Netquack — development targets"
	@echo ""
	@echo "Build"
	@echo "  make / make release     Release build (default)"
	@echo "  make debug              Debug build"
	@echo "  make reldebug           RelWithDebInfo build"
	@echo "  GEN=ninja make          Faster Ninja-backed build (recommended)"
	@echo "  make clangd             Generate compile_commands for clangd"
	@echo ""
	@echo "Test"
	@echo "  make test               Run all tests (release)"
	@echo "  make test_debug         Run all tests (debug)"
	@echo "  make test-one TEST=...  Run a single sqllogictest file"
	@echo ""
	@echo "Quality"
	@echo "  make format             Auto-format src/ and test/"
	@echo "  make format-check       Check formatting without writing"
	@echo "  make tidy-check         Run clang-tidy"
	@echo ""
	@echo "Run"
	@echo "  make run / make shell   Interactive DuckDB CLI (unsigned, release)"
	@echo "  make debug-run          Interactive DuckDB CLI (unsigned, debug)"
	@echo ""
	@echo "Maintenance"
	@echo "  make clean              Remove build artifacts"
	@echo "  make update             Update git submodules"
	@echo "  make pull               Init and update submodules"
	@echo "  make update-tld         Refresh Public Suffix List (gperf)"
	@echo "  make benchmark          Compare local vs published extension"
	@echo ""
	@echo "Examples"
	@echo "  GEN=ninja make"
	@echo "  GEN=ninja make test"
	@echo "  make test-one TEST=test/sql/extract_domain.test"
	@echo ""
	@echo "See CONTRIBUTING.md for setup (vcpkg, gperf, clang-format)."

build: release

run:
	@test -x "$(DUCKDB_RELEASE)" || { echo "error: build first with 'GEN=ninja make'"; exit 1; }
	@echo "Unsigned local shell. Load the extension with:"
	@echo "  LOAD '$(EXT_RELEASE_BIN)';"
	@$(DUCKDB_RELEASE) -unsigned

shell: run

debug-run:
	@test -x "$(DUCKDB_DEBUG)" || { echo "error: build first with 'GEN=ninja make debug'"; exit 1; }
	@echo "Unsigned local shell. Load the extension with:"
	@echo "  LOAD '$(EXT_DEBUG_BIN)';"
	@$(DUCKDB_DEBUG) -unsigned

test-one:
	@test -n "$(TEST)" || { echo "Usage: make test-one TEST=test/sql/extract_domain.test"; exit 1; }
	@test -x "$(PROJ_DIR)build/release$(TEST_PATH)" || { echo "error: build first with 'GEN=ninja make'"; exit 1; }
	"$(PROJ_DIR)build/release$(TEST_PATH)" "$(TEST)"

update-tld:
	rm -f public_suffix_list.dat
	bash scripts/generate_tld_lookup.sh

benchmark:
	bash scripts/benchmark.sh
