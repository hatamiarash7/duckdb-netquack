---
icon: list-check
cover: >-
  https://images.unsplash.com/photo-1476973422084-e0fa66ff9456?crop=entropy&cs=srgb&fm=jpg&ixid=M3wxOTcwMjR8MHwxfHNlYXJjaHw1fHxyb2FkbWFwfGVufDB8fHx8MTczOTEyOTkwNnww&ixlib=rb-4.0.3&q=85
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

# Roadmap

- [ ] Implement `extract_custom_format` function
- [ ] Implement `parse_uri` function - Return a STRUCT with all components (scheme, host, port, path, query, fragment) in a single call
- [ ] Save Tranco data as Parquet
- [ ] Implement GeoIP functionality
- [ ] Return default value for `get_tranco_rank`
- [ ] Implement `extract_fragment` function - Extract the fragment (`#section`) from a URL
- [ ] Implement `normalize_url` function - Canonicalize URLs (lowercase scheme/host, remove default ports, sort query params, remove trailing slashes)
- [ ] Implement `is_valid_url` function - Return whether a string is a well-formed URL
- [ ] Implement `url_encode` / `url_decode` functions - Standalone percent-encoding and decoding
- [ ] Implement `is_valid_ip` function - Return whether a string is a valid IPv4 or IPv6 address
- [ ] Implement `is_private_ip` function - Check if an IP is in a private/reserved range (RFC 1918, loopback, link-local)
- [ ] Implement `ip_to_int` / `int_to_ip` functions - Convert between dotted-quad notation and integer representation
- [ ] Implement `ip_in_range` function - Check if an IP falls within a given CIDR block
- [ ] Implement `ip_version` function - Return `4` or `6` for the IP version of a given address
- [ ] Support internationalized domain names (IDNs)
- [ ] Implement `punycode_encode` / `punycode_decode` functions - Convert internationalized domain names to/from ASCII-compatible encoding
- [ ] Implement `is_valid_domain` function - Validate a domain name against RFC rules
- [ ] Implement `domain_depth` function - Return the number of levels in a domain
- [ ] Implement `base64_encode` / `base64_decode` functions - Encode and decode Base64 strings
- [ ] Implement `extract_path_segments` table function - Split a URL path into individual segment rows
