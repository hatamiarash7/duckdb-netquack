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

# Check Private IP

The `is_private_ip` function checks whether an IP address belongs to a private or reserved range. Supports both IPv4 and IPv6. Returns `NULL` for invalid addresses.

```sql
D SELECT is_private_ip('192.168.1.1');
┌──────────────────────────────┐
│ is_private_ip('192.168.1.1') │
│           boolean            │
├──────────────────────────────┤
│ true                         │
└──────────────────────────────┘

D SELECT is_private_ip('8.8.8.8');
┌──────────────────────────┐
│ is_private_ip('8.8.8.8') │
│         boolean          │
├──────────────────────────┤
│ false                    │
└──────────────────────────┘

D SELECT is_private_ip('fe80::1');
┌──────────────────────────┐
│ is_private_ip('fe80::1') │
│         boolean          │
├──────────────────────────┤
│ true                     │
└──────────────────────────┘
```

## IPv4 Private/Reserved Ranges

| Range                | Description                | RFC      |
| -------------------- | -------------------------- | -------- |
| `10.0.0.0/8`         | Private network            | RFC 1918 |
| `172.16.0.0/12`      | Private network            | RFC 1918 |
| `192.168.0.0/16`     | Private network            | RFC 1918 |
| `127.0.0.0/8`        | Loopback                   | RFC 1122 |
| `169.254.0.0/16`     | Link-local (APIPA)         | RFC 3927 |
| `0.0.0.0/8`          | Current network            | RFC 1122 |
| `100.64.0.0/10`      | Carrier-grade NAT          | RFC 6598 |
| `192.0.0.0/24`       | IETF Protocol Assignments  | RFC 6890 |
| `192.0.2.0/24`       | Documentation (TEST-NET-1) | RFC 5737 |
| `198.51.100.0/24`    | Documentation (TEST-NET-2) | RFC 5737 |
| `203.0.113.0/24`     | Documentation (TEST-NET-3) | RFC 5737 |
| `198.18.0.0/15`      | Benchmarking               | RFC 2544 |
| `224.0.0.0/4`        | Multicast                  | RFC 5771 |
| `240.0.0.0/4`        | Reserved for future use    | RFC 1112 |
| `255.255.255.255/32` | Broadcast                  | RFC 919  |

## IPv6 Private/Reserved Ranges

| Range           | Description                | RFC      |
| --------------- | -------------------------- | -------- |
| `::1/128`       | Loopback                   | RFC 4291 |
| `::/128`        | Unspecified                | RFC 4291 |
| `fe80::/10`     | Link-local                 | RFC 4291 |
| `fc00::/7`      | Unique local address (ULA) | RFC 4193 |
| `ff00::/8`      | Multicast                  | RFC 4291 |
| `2001:db8::/32` | Documentation              | RFC 3849 |
| `100::/64`      | Discard prefix             | RFC 6666 |

For IPv4-mapped IPv6 addresses (`::ffff:x.x.x.x`), the function checks the embedded IPv4 address against the IPv4 private ranges listed above.
