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

# IP Type / Bogon

## ip_type

The `ip_type` function classifies an IPv4 or IPv6 address by the special-purpose range it belongs to.

```sql
D SELECT ip_type('100.64.0.1');
┌───────────────────────┐
│ ip_type('100.64.0.1') │
│        varchar        │
├───────────────────────┤
│ cgnat                 │
└───────────────────────┘

D SELECT ip_type('fe80::1');
┌────────────────────┐
│ ip_type('fe80::1') │
│      varchar       │
├────────────────────┤
│ link_local         │
└────────────────────┘
```

| Type            | IPv4                                                                       | IPv6                                                                                   |
| --------------- | -------------------------------------------------------------------------- | -------------------------------------------------------------------------------------- |
| `loopback`      | `127.0.0.0/8`                                                              | `::1`                                                                                  |
| `private`       | `10.0.0.0/8`, `172.16.0.0/12`, `192.168.0.0/16`                            | `fc00::/7`                                                                             |
| `link_local`    | `169.254.0.0/16`                                                           | `fe80::/10`                                                                            |
| `cgnat`         | `100.64.0.0/10`                                                            | —                                                                                      |
| `multicast`     | `224.0.0.0/4`                                                              | `ff00::/8`                                                                             |
| `documentation` | `192.0.2.0/24`, `198.51.100.0/24`, `203.0.113.0/24`                        | `2001:db8::/32`, `3fff::/20`                                                           |
| `reserved`      | `0.0.0.0/8`, `192.0.0.0/24`, `198.18.0.0/15`, `240.0.0.0/4` (incl. broadcast) | `::`, everything outside `2000::/3`, `2001:2::/48`, `2001:10::/28`                   |
| `public`        | everything else                                                            | everything else in `2000::/3`                                                          |

Summarize traffic by address type:

```sql
D SELECT ip_type(client_ip) AS type, count(*) FROM logs GROUP BY type;
```

## is_bogon

The `is_bogon` function returns `true` if the address is not globally routable, i.e. its `ip_type` is anything other than `public`.

```sql
D SELECT is_bogon('203.0.113.1');
┌─────────────────────────┐
│ is_bogon('203.0.113.1') │
│         boolean         │
├─────────────────────────┤
│ true                    │
└─────────────────────────┘

D SELECT is_bogon('8.8.8.8');
┌─────────────────────┐
│ is_bogon('8.8.8.8') │
│       boolean       │
├─────────────────────┤
│ false               │
└─────────────────────┘
```

## Behavior

- IPv4-mapped IPv6 addresses (`::ffff:a.b.c.d`) are classified by their embedded IPv4 address, so `ip_type('::ffff:192.168.1.1')` is `private`.
- Addresses outside the IPv6 global unicast block `2000::/3` (e.g. `100::/64`, `fec0::/10`, `64:ff9b::/96`) are `reserved`.
- For IPv4, `is_bogon` matches `is_private_ip`: both cover the same set of ranges.
- Bracketed IPv6 input (e.g. `[::1]`) is accepted.
- Both functions return `NULL` for `NULL`, empty strings, invalid addresses, and CIDR notation.
