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

# IPv6 Compress / Expand

## ipv6_compress

The `ipv6_compress` function formats an IPv6 address in its shortest [RFC 5952](https://www.rfc-editor.org/rfc/rfc5952) canonical form.

```sql
D SELECT ipv6_compress('2001:0db8:0000:0000:0000:0000:0000:0001');
┌──────────────────────────────────────────────────────────┐
│ ipv6_compress('2001:0db8:0000:0000:0000:0000:0000:0001') │
│                         varchar                          │
├──────────────────────────────────────────────────────────┤
│ 2001:db8::1                                              │
└──────────────────────────────────────────────────────────┘

D SELECT ipv6_compress('::ffff:c0a8:101');
┌──────────────────────────────────┐
│ ipv6_compress('::ffff:c0a8:101') │
│             varchar              │
├──────────────────────────────────┤
│ ::ffff:192.168.1.1               │
└──────────────────────────────────┘
```

Use it to deduplicate addresses written in different forms:

```sql
D SELECT ipv6_compress(ip) AS canonical, count(*) FROM ips GROUP BY canonical;
```

## ipv6_expand

The `ipv6_expand` function returns the full form of an IPv6 address: eight groups of four lowercase hex digits.

```sql
D SELECT ipv6_expand('2001:db8::1');
┌─────────────────────────────────────────┐
│       ipv6_expand('2001:db8::1')        │
│                 varchar                 │
├─────────────────────────────────────────┤
│ 2001:0db8:0000:0000:0000:0000:0000:0001 │
└─────────────────────────────────────────┘
```

## is_ipv4_mapped

The `is_ipv4_mapped` function returns `true` if the address is an IPv4-mapped IPv6 address (`::ffff:0:0/96`).

```sql
D SELECT is_ipv4_mapped('::ffff:192.168.1.1');
┌──────────────────────────────────────┐
│ is_ipv4_mapped('::ffff:192.168.1.1') │
│               boolean                │
├──────────────────────────────────────┤
│ true                                 │
└──────────────────────────────────────┘
```

## Behavior

- `ipv6_compress` lowercases hex digits, strips leading zeros, and replaces the longest run of two or more zero groups with `::` (the first run wins on a tie). A single zero group is never compressed.
- `ipv6_compress` writes IPv4-mapped addresses in mixed notation (`::ffff:192.168.1.1`). Other embedded IPv4 forms (e.g. `64:ff9b::1.2.3.4`) are rendered as hex.
- `ipv6_expand` always returns 39 characters, with any embedded IPv4 tail converted to hex groups.
- Bracketed input (e.g. `[2001:db8::1]`) is accepted; output never includes brackets.
- `ipv6_compress` and `ipv6_expand` return `NULL` for `NULL`, invalid addresses, CIDR notation, and IPv4 addresses.
- `is_ipv4_mapped` returns `false` for plain IPv4 addresses and other IPv6 addresses (including the deprecated IPv4-compatible `::a.b.c.d` form), and `NULL` for `NULL` or invalid input.
