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

# IP Address

This extension provides various functions for manipulating and analyzing IP addresses, including calculating networks, hosts, and subnet masks.

* [**IP Calculator**](ip-calculator.md) — Calculate network, broadcast, host range, and subnet masks from an IP/CIDR
* [**Validate IP Address**](is-valid-ip.md) — Check if a string is a valid IPv4 or IPv6 address
* [**Check Private IP**](is-private-ip.md) — Determine if an IP belongs to a private or reserved range
* [**IP Version**](ip-version.md) — Detect whether an address is IPv4 or IPv6
* [**IP to Integer / Integer to IP**](ip-to-int.md) — Convert between dotted-quad notation and integer representation
* [**IP in Range**](ip-in-range.md) — Check if an IP falls within a given CIDR block
* [**IP to PTR**](ip-to-ptr.md) — Build the reverse DNS (`in-addr.arpa` / `ip6.arpa`) name for an IP
* [**IPv6 Compress / Expand**](ipv6-format.md) — Canonicalize or fully expand IPv6 addresses and detect IPv4-mapped addresses
* [**IP Type / Bogon**](ip-type.md) — Classify an IP (public, private, loopback, CGNAT, ...) and detect bogons
