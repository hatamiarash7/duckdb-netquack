# Security Policy

## Supported versions

Security fixes are applied to the latest release of Netquack. Please upgrade with:

```sql
FORCE INSTALL netquack FROM community;
LOAD netquack;
```

Compatibility with DuckDB is listed in the [README](README.md#installation-).

## Reporting a vulnerability

Please **do not** open a public issue for security problems.

Report privately via [GitHub security advisories](https://github.com/hatamiarash7/duckdb-netquack/security/advisories/new), or contact [@hatamiarash7](https://github.com/hatamiarash7) directly.

Include:

- A description of the issue and its impact
- Steps to reproduce, or a proof of concept
- Netquack and DuckDB versions (`SELECT * FROM netquack_version();` / `duckdb --version`)

You will receive an acknowledgement as soon as possible. Please give us time to investigate and ship a fix before any public disclosure.
