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
- [ ] Save Tranco data as Parquet
- [ ] Implement GeoIP functionality
- [ ] Return default value for `get_tranco_rank`
- [ ] Support internationalized domain names (IDNs)
- [ ] Implement `punycode_encode` / `punycode_decode` functions - Convert internationalized domain names to/from ASCII-compatible encoding
- [ ] Implement `extract_query_value` function - Return a single query parameter value as a scalar
- [ ] Implement `strip_tracking_params` / `remove_query_params` functions - Remove `utm_*`, `fbclid`, `gclid` and user-specified parameters
- [ ] Implement `resolve_url` function - Resolve a relative reference against a base URL (RFC 3986)
- [ ] Implement `extract_origin` / `is_same_origin` / `is_same_site` functions
- [ ] Implement `url_hierarchy` / `url_path_hierarchy` functions - Return the list of URL prefixes
- [ ] Implement `url_to_surt` / `reverse_domain` functions - Web-archive (SURT) sort keys
- [ ] Implement `extract_urls` / `extract_domains` / `extract_ips` functions - Extract indicators from free text
- [ ] Implement `defang` / `refang` functions - Convert URLs/IPs to and from their defanged form (`hxxps://example[.]com`)
- [ ] Implement `mime_type` function - Map a URL's file extension to its MIME type
- [ ] Support IPv6 in `ip_to_int` / `int_to_ip` (`UHUGEINT`)
- [ ] Implement CIDR functions - `cidr_contains`, `cidr_overlaps`, `cidr_range`, `range_to_cidrs`
- [ ] Implement `cidr_merge` aggregate function - Collapse a set of CIDRs into the minimal covering set
- [ ] Implement `ip_anonymize` function - Truncate IPv4/IPv6 addresses for privacy
- [ ] Implement ASN lookup - `ip_to_asn` / `ip_to_as_org`
- [ ] Implement `extract_sld` function - Return the label before the public suffix
- [ ] Implement `is_public_suffix` / `is_known_tld` functions
- [ ] Implement `domain_entropy` / `is_likely_dga` functions - Detect algorithmically generated domains
- [ ] Implement `generate_typosquats` table function - Generate typosquatting variants of a domain
- [ ] Implement `domain_skeleton` / `is_homograph` functions - Detect Unicode confusable domains
- [ ] Implement `tranco_list` table function - Expose the cached Tranco list for joins
- [ ] Support historical Tranco lists by date or list ID
- [ ] Support other ranking lists (Cloudflare Radar, Cisco Umbrella, Majestic)
- [ ] Implement email functions - `extract_email_domain`, `is_valid_email`, `normalize_email`, `is_disposable_email_domain`
- [ ] Implement `read_access_log` table function - Parse Apache/Nginx access logs
- [ ] Implement `port_service` / `default_port` functions - Map ports to IANA service names and schemes to default ports
- [ ] Implement MAC address functions - `is_valid_mac`, `normalize_mac`, `mac_vendor`
- [ ] Support interop with DuckDB's `INET` type
- [ ] Add `LIST` overloads (e.g. `ip_in_range(ip, ['10.0.0.0/8', ...])`)
- [ ] Add a benchmarks page to the documentation
