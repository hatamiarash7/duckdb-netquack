---
icon: champagne-glasses
cover: >-
  https://images.unsplash.com/photo-1582213782179-e0d53f98f2ca?crop=entropy&cs=srgb&fm=jpg&ixid=M3wxOTcwMjR8MHwxfHNlYXJjaHw2fHxoZWxwfGVufDB8fHx8MTczOTE5NTg1M3ww&ixlib=rb-4.0.3&q=85
coverY: 0
layout:
  cover:
    visible: true
    size: full
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

# Contributing

Thanks for wanting to contribute. Bug reports, new functions, tests, and documentation are all welcome.

The full development guide lives in [`CONTRIBUTING.md`](https://github.com/hatamiarash7/duckdb-netquack/blob/main/CONTRIBUTING.md). Architecture, function checklists, and test conventions are in [`AGENTS.md`](https://github.com/hatamiarash7/duckdb-netquack/blob/main/AGENTS.md). By participating you agree to the [Code of Conduct](https://github.com/hatamiarash7/duckdb-netquack/blob/main/CODE_OF_CONDUCT.md).

## Workflow

1. [Fork](https://github.com/hatamiarash7/duckdb-netquack/fork) the repository.
2. Clone with submodules and create a branch from `main`.
3. Build and test locally (`make help` lists targets):

```bash
git clone --recurse-submodules git@github.com:hatamiarash7/duckdb-netquack.git
cd duckdb-netquack
GEN=ninja make
GEN=ninja make test
```

4. Add tests under `test/sql/` for any behavior change.
5. Run `make format` before opening a pull request.

## Adding a function

New functions must follow the checklist in [`AGENTS.md`](https://github.com/hatamiarash7/duckdb-netquack/blob/main/AGENTS.md):

- Header + implementation in `src/functions/`
- Register with `Register()` (descriptions, examples, and categories) so the function appears in `duckdb_functions()`
- SQL tests, including a NULL case in `null_handling.test`
- README example, GitBook page, and `docs/SUMMARY.md` entry

## Pull requests

Keep changes focused. Fill in the pull-request template and link related issues. CI (`format` + extension distribution) must pass before merge.
