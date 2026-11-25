# Changelog

Each version links to its published GitHub release, with the newest first.

## [1.16.2](https://github.com/hatamiarash7/duckdb-netquack/releases/tag/1.16.2) — 2026-11-25

- Added support for DuckDB 2.0.1
- Create `CHANGELOG.md` file.

## [1.16.1](https://github.com/hatamiarash7/duckdb-netquack/releases/tag/1.16.1) — 2026-11-06

- `url_decode` and `base64_decode` now return `NULL` when the decoded bytes are not valid UTF-8.

## [1.16.0](https://github.com/hatamiarash7/duckdb-netquack/releases/tag/1.16.0) — 2026-10-24

- Added support for DuckDB 2.0.0

## [1.15.1](https://github.com/hatamiarash7/duckdb-netquack/releases/tag/1.15.1) — 2026-10-01

- Added support for DuckDB 1.5.6

## [1.15.0](https://github.com/hatamiarash7/duckdb-netquack/releases/tag/1.15.0) — 2026-09-25

### New Features

- Format IPv6 addresses with `ipv6_compress` and `ipv6_expand`. Identify IPv4-mapped addresses with `is_ipv4_mapped`.
- Classify addresses with `ip_type` and `is_bogon`, anonymize them with `ip_anonymize`, check range membership with `ip_in_range`, and generate reverse DNS names with `ip_to_ptr`.
- Check public suffixes and known top-level domains with `is_public_suffix` and `is_known_tld`. Extract the second-level domain with `extract_sld`.
- Extract URLs, domains, and IP addresses from text with `extract_urls`, `extract_domains`, and `extract_ips`. Defang and restore indicators with `defang` and `refang`.
- Convert between URLs and SURT notation with `url_to_surt` and `surt_to_url`.
- Query the Tranco ranking list with `tranco_list`.

## [1.14.0](https://github.com/hatamiarash7/duckdb-netquack/releases/tag/1.14.0) — 2026-09-21

- Added `parse_uri`, which returns `scheme`, `host`, `port`, `path`, `query`, and `fragment` in a single `STRUCT`.
- Completed function metadata so SQL tools and AI agents can discover how to use Netquack functions.

## [1.13.0](https://github.com/hatamiarash7/duckdb-netquack/releases/tag/1.13.0) — 2026-07-25

- Added support for DuckDB 1.5.5
- Fixed time calculations on Windows.

## [1.12.1](https://github.com/hatamiarash7/duckdb-netquack/releases/tag/1.12.1) — 2026-05-22

- Added support for DuckDB 1.5.3

## [1.12.0](https://github.com/hatamiarash7/duckdb-netquack/releases/tag/1.12.0) — 2026-04-30

- Refreshed the lookup table used to recognize top-level domains.

## [1.11.2](https://github.com/hatamiarash7/duckdb-netquack/releases/tag/1.11.2) — 2026-04-14

- Added support for DuckDB 1.5.2

## [1.11.1](https://github.com/hatamiarash7/duckdb-netquack/releases/tag/1.11.1) — 2026-04-02

- Added support for DuckDB 1.5.1

## [1.11.0](https://github.com/hatamiarash7/duckdb-netquack/releases/tag/1.11.0) — 2026-02-24

- Added `extract_path_segments` to split a URL path into segments, and `url_encode` and `url_decode` for percent-encoding and decoding.

## [1.10.0](https://github.com/hatamiarash7/duckdb-netquack/releases/tag/1.10.0) — 2026-02-21

- Added `normalize_url` to normalize URLs and `extract_fragment` to read the part after `#`.
- Added `is_valid_url` and `is_valid_domain` for validation, and `domain_depth` to count domain labels.
- Added `base64_encode` and `base64_decode`.

## [1.9.0](https://github.com/hatamiarash7/duckdb-netquack/releases/tag/1.9.0) — 2026-02-20

- Added `is_valid_ip`, `is_private_ip`, and `ip_version` for IPv4 and IPv6. `is_private_ip` recognizes reserved ranges as well as private networks, including loopback, link-local, and documentation ranges.
- Added `ip_to_int` and `int_to_ip` to convert between dotted IPv4 addresses and their integer representation.

## [1.8.1](https://github.com/hatamiarash7/duckdb-netquack/releases/tag/1.8.1) — 2026-02-20

- `extract_query_parameters` now supports lateral joins, so it can extract parameters from each URL in a table.

## [1.8.0](https://github.com/hatamiarash7/duckdb-netquack/releases/tag/1.8.0) — 2026-02-15

- Added `extract_query_parameters` to return URL query parameters as rows.
- Updated DuckDB to 1.4.4

## [1.7.0](https://github.com/hatamiarash7/duckdb-netquack/releases/tag/1.7.0) — 2025-12-06

### Improvements

- Added support for RFC 3021 `/31` IPv4 point-to-point networks, sped up IP parsing, and guarded against overflow.
- Updated DuckDB to 1.4.2 and improved handling of `NULL` inputs across functions.

### Bug Fixes

- Fixed a SQL injection vulnerability by using parameterized queries.
- Fixed `extract_host` for URLs containing `user:pass@host`. `extract_query_string` now returns an empty string when `?` occurs only inside the fragment.

### Breaking Changes and Upgrade Notes

- `get_tranco_rank` and `get_tranco_rank_category` now return `NULL` instead of an empty string when a domain is absent from the list. Use `IS NULL` to match missing results.

## [1.6.0](https://github.com/hatamiarash7/duckdb-netquack/releases/tag/1.6.0) — 2025-10-02

- Accelerated URL parsing and top-level domain lookups. In the release benchmarks, `extract_domain` fell from 37.503 s to 0.012 s and `extract_tld` from 36.003 s to 0.009 s.

## [1.5.1](https://github.com/hatamiarash7/duckdb-netquack/releases/tag/1.5.1) — 2025-09-28

- Fixed a crash on Windows in the cURL callback that handles downloaded data.

## [1.5.0](https://github.com/hatamiarash7/duckdb-netquack/releases/tag/1.5.0) — 2025-09-24

- Added support for DuckDB 1.4.0
- Downloaded data is now validated before processing, and more errors are reported in the console.

## [1.4.0](https://github.com/hatamiarash7/duckdb-netquack/releases/tag/1.4.0) — 2025-05-19

- Updated DuckDB to 1.2.2 and fixed certificate authority handling in cURL.

## [1.3.0](https://github.com/hatamiarash7/duckdb-netquack/releases/tag/1.3.0) — 2025-05-15

- Added `extract_port` and `extract_extension` to read a URL's port and file extension.
- Added cURL error logging and a debugging section to the documentation.

## [1.2.0](https://github.com/hatamiarash7/duckdb-netquack/releases/tag/1.2.0) — 2025-03-19

- Added the `ipcalc` IP calculator and fixed reported issues in extraction functions.

## [1.1.0](https://github.com/hatamiarash7/duckdb-netquack/releases/tag/1.1.0) — 2025-03-13

- Added support for more URI schemes and array input. Input values are now converted to lowercase.

## [1.0.0](https://github.com/hatamiarash7/duckdb-netquack/releases/tag/1.0.0) — 2025-02-09

- Initial release of Netquack.
