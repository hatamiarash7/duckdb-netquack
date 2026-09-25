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

# IP to PTR

The `ip_to_ptr` function builds the reverse DNS (PTR) name for an IP address. Supports both IPv4 (`in-addr.arpa`) and IPv6 (`ip6.arpa`).

```sql
D SELECT ip_to_ptr('192.168.1.1');
┌──────────────────────────┐
│ ip_to_ptr('192.168.1.1') │
│         varchar          │
├──────────────────────────┤
│ 1.1.168.192.in-addr.arpa │
└──────────────────────────┘

D SELECT ip_to_ptr('2001:db8::1');
┌──────────────────────────────────────────────────────────────────────────┐
│                         ip_to_ptr('2001:db8::1')                         │
│                                 varchar                                  │
├──────────────────────────────────────────────────────────────────────────┤
│ 1.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.8.b.d.0.1.0.0.2.ip6.arpa │
└──────────────────────────────────────────────────────────────────────────┘
```

Build PTR names for a table of addresses:

```sql
D SELECT ip, ip_to_ptr(ip) AS ptr FROM ips;
┌─────────┬──────────────────────┐
│   ip    │         ptr          │
│ varchar │       varchar        │
├─────────┼──────────────────────┤
│ 8.8.8.8 │ 8.8.8.8.in-addr.arpa │
│ 1.1.1.1 │ 1.1.1.1.in-addr.arpa │
└─────────┴──────────────────────┘
```

## Behavior

- IPv4 octets are reversed and suffixed with `in-addr.arpa`.
- IPv6 addresses are fully expanded, then every hex nibble is reversed and suffixed with `ip6.arpa`. Hex digits are always lowercase.
- Bracketed IPv6 addresses (e.g. `[2001:db8::1]`) are accepted.
- IPv4-mapped IPv6 addresses (`::ffff:x.x.x.x`) produce an `ip6.arpa` name.
- The result has no trailing dot.
- Returns `NULL` for `NULL`, invalid addresses, or CIDR notation.
