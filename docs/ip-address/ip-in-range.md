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

# IP in Range

The `ip_in_range` function checks whether an IP address falls within a given CIDR block. Supports both IPv4 and IPv6.

```sql
D SELECT ip_in_range('192.168.1.100', '192.168.1.0/24');
┌────────────────────────────────────────────────┐
│ ip_in_range('192.168.1.100', '192.168.1.0/24') │
│                    boolean                     │
├────────────────────────────────────────────────┤
│ true                                           │
└────────────────────────────────────────────────┘

D SELECT ip_in_range('10.0.0.1', '192.168.1.0/24');
┌───────────────────────────────────────────┐
│ ip_in_range('10.0.0.1', '192.168.1.0/24') │
│                  boolean                  │
├───────────────────────────────────────────┤
│ false                                     │
└───────────────────────────────────────────┘

D SELECT ip_in_range('2001:db8::1', '2001:db8::/32');
┌─────────────────────────────────────────────┐
│ ip_in_range('2001:db8::1', '2001:db8::/32') │
│                   boolean                   │
├─────────────────────────────────────────────┤
│ true                                        │
└─────────────────────────────────────────────┘
```

Filter rows by network:

```sql
D SELECT ip FROM logs WHERE ip_in_range(ip, '10.0.0.0/8');
┌──────────┐
│    ip    │
│ varchar  │
├──────────┤
│ 10.1.2.3 │
└──────────┘
```

## Behavior

- A CIDR without a prefix length (e.g. `8.8.8.8`) is treated as a single host (`/32` for IPv4, `/128` for IPv6).
- Host bits in the network address are ignored, so `192.168.1.200/24` is the same as `192.168.1.0/24`.
- An IPv4 address never matches an IPv6 block and vice versa; the function returns `false`.
- IPv4-mapped IPv6 addresses (`::ffff:x.x.x.x`) are compared as IPv6.
- Returns `NULL` if either argument is `NULL`, the IP is invalid, or the CIDR is malformed (bad address, or prefix outside `0-32` / `0-128`).
