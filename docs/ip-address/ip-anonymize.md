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

# IP Anonymize

The `ip_anonymize` function truncates an IP address for privacy by zeroing every bit after a prefix length. By default it keeps the first 24 bits of an IPv4 address (dropping the last octet) and the first 48 bits of an IPv6 address.

```sql
D SELECT ip_anonymize('192.168.1.123');
┌───────────────────────────────┐
│ ip_anonymize('192.168.1.123') │
│            varchar            │
├───────────────────────────────┤
│ 192.168.1.0                   │
└───────────────────────────────┘

D SELECT ip_anonymize('2001:db8:abcd:1234::1');
┌───────────────────────────────────────┐
│ ip_anonymize('2001:db8:abcd:1234::1') │
│                varchar                │
├───────────────────────────────────────┤
│ 2001:db8:abcd::                       │
└───────────────────────────────────────┘
```

## Custom prefixes

Use `ip_anonymize(ip, ipv4_prefix, ipv6_prefix)` to choose how many leading bits to keep. The prefix that applies depends on the address family, so the same call works on mixed IPv4/IPv6 columns.

```sql
D SELECT ip_anonymize('192.168.1.123', 16, 32);
┌───────────────────────────────────────┐
│ ip_anonymize('192.168.1.123', 16, 32) │
│                varchar                │
├───────────────────────────────────────┤
│ 192.168.0.0                           │
└───────────────────────────────────────┘

D SELECT ip_anonymize('2001:db8:abcd:1234::1', 16, 32);
┌───────────────────────────────────────────────┐
│ ip_anonymize('2001:db8:abcd:1234::1', 16, 32) │
│                    varchar                    │
├───────────────────────────────────────────────┤
│ 2001:db8::                                    │
└───────────────────────────────────────────────┘
```

Aggregate traffic without storing full client addresses:

```sql
D SELECT ip_anonymize(client_ip) AS client_net, count(*) FROM logs GROUP BY client_net;
```

## Behavior

- Prefixes do not need to be byte-aligned: `ip_anonymize('192.168.1.255', 25, 48)` returns `192.168.1.128`.
- IPv6 output is in RFC 5952 canonical form (lowercase, longest zero run compressed), so `ip_anonymize('2001:DB8:ABCD:1::1')` returns `2001:db8:abcd::`.
- IPv4-mapped IPv6 addresses (`::ffff:a.b.c.d`) use the IPv4 prefix: `ip_anonymize('::ffff:192.168.1.123')` returns `::ffff:192.168.1.0`.
- Bracketed IPv6 input (e.g. `[2001:db8::1]`) is accepted.
- Valid prefixes are `0`–`32` for IPv4 and `0`–`128` for IPv6. Out-of-range prefixes return `NULL`.
- Returns `NULL` for `NULL` arguments, empty strings, invalid addresses, and CIDR notation.
