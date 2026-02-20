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

# IP Version

The `ip_version` function detects the version of an IP address. Returns `4` for IPv4, `6` for IPv6, or `NULL` for invalid input.

```sql
D SELECT ip_version('192.168.1.1');
┌───────────────────────────┐
│ ip_version('192.168.1.1') │
│          tinyint          │
├───────────────────────────┤
│ 4                         │
└───────────────────────────┘

D SELECT ip_version('::1');
┌────────────────────┐
│ ip_version('::1')  │
│      tinyint       │
├────────────────────┤
│ 6                  │
└────────────────────┘

D SELECT ip_version('not-an-ip');
┌───────────────────────────┐
│ ip_version('not-an-ip')   │
│          tinyint          │
├───────────────────────────┤
│ NULL                      │
└───────────────────────────┘
```

This is useful for filtering or branching logic based on IP type:

```sql
D SELECT ip, ip_version(ip) AS version
  FROM my_ips
  WHERE ip_version(ip) = 4;
```
