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

# IP to Integer / Integer to IP

Two functions for converting between IPv4 addresses and their 32-bit unsigned integer representation.

## ip\_to\_int

Converts an IPv4 address string to a `UBIGINT` (unsigned 64-bit integer, but the value will always fit in 32 bits). Returns `NULL` for invalid or IPv6 input.

```sql
D SELECT ip_to_int('192.168.1.1');
┌──────────────────────────┐
│ ip_to_int('192.168.1.1') │
│          uint64          │
├──────────────────────────┤
│        3232235777        │
│      (3.23 billion)      │
└──────────────────────────┘

D SELECT ip_to_int('10.0.0.1');
┌───────────────────────┐
│ ip_to_int('10.0.0.1') │
│        uint64         │
├───────────────────────┤
│       167772161       │
│   (167.77 million)    │
└───────────────────────┘

D SELECT ip_to_int('255.255.255.255');
┌──────────────────────────────┐
│ ip_to_int('255.255.255.255') │
│            uint64            │
├──────────────────────────────┤
│          4294967295          │
│        (4.29 billion)        │
└──────────────────────────────┘
```

## int\_to\_ip

Converts a `UBIGINT` integer back to an IPv4 dotted-quad string. Returns `NULL` if the value exceeds the IPv4 range (`> 4294967295`).

```sql
D SELECT int_to_ip(3232235777::UBIGINT);
┌────────────────────────────────────────┐
│ int_to_ip(CAST(3232235777 AS UBIGINT)) │
│                varchar                 │
├────────────────────────────────────────┤
│ 192.168.1.1                            │
└────────────────────────────────────────┘

D SELECT int_to_ip(0::UBIGINT);
┌───────────────────────────────┐
│ int_to_ip(CAST(0 AS UBIGINT)) │
│            varchar            │
├───────────────────────────────┤
│ 0.0.0.0                       │
└───────────────────────────────┘

D SELECT int_to_ip('3232235777');
┌─────────────────────────┐
│ int_to_ip('3232235777') │
│         varchar         │
├─────────────────────────┤
│ 192.168.1.1             │
└─────────────────────────┘
```

## Roundtrip

The two functions are inverses of each other:

```sql
D SELECT int_to_ip(ip_to_int('192.168.1.1')) AS result;
┌─────────────┐
│   result    │
│   varchar   │
├─────────────┤
│ 192.168.1.1 │
└─────────────┘
```

## Use Cases

**Sort IPs numerically** instead of lexicographically:

```sql
D SELECT ip FROM my_ips ORDER BY ip_to_int(ip);
┌─────────────┐
│     ip      │
│   varchar   │
├─────────────┤
│ 8.8.8.8     │
│ 10.0.0.1    │
│ 192.168.1.1 │
└─────────────┘
```

**Range queries** using integer comparison:

```sql
D SELECT ip
  FROM my_ips
  WHERE ip_to_int(ip) BETWEEN ip_to_int('10.0.0.0') AND ip_to_int('10.255.255.255');
┌──────────┐
│    ip    │
│ varchar  │
├──────────┤
│ 10.0.0.1 │
└──────────┘
```
