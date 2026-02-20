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

# Validate IP Address

The `is_valid_ip` function checks whether a string is a valid IPv4 or IPv6 address. Returns a `BOOLEAN`.

Supports:

* Standard IPv4 dotted-quad notation (e.g., `192.168.1.1`)
* Full and compressed IPv6 notation (e.g., `2001:db8::1`, `::1`)
* Bracketed IPv6 (e.g., `[::1]`)
* IPv4-mapped IPv6 (e.g., `::ffff:192.168.1.1`)

Returns `NULL` for `NULL` input.

```sql
D SELECT is_valid_ip('192.168.1.1');
┌───────────────────────────┐
│ is_valid_ip('192.168.1.1')│
│          boolean          │
├───────────────────────────┤
│ true                      │
└───────────────────────────┘

D SELECT is_valid_ip('2001:db8::1');
┌─────────────────────────────┐
│ is_valid_ip('2001:db8::1')  │
│          boolean            │
├─────────────────────────────┤
│ true                        │
└─────────────────────────────┘

D SELECT is_valid_ip('not-an-ip');
┌───────────────────────────┐
│ is_valid_ip('not-an-ip')  │
│          boolean          │
├───────────────────────────┤
│ false                     │
└───────────────────────────┘
```

## Validation Rules

### IPv4

* Must have exactly 4 octets separated by dots
* Each octet must be a number between 0 and 255
* No leading zeros allowed (e.g., `192.168.01.1` is invalid)
* No CIDR notation (e.g., `192.168.1.0/24` is invalid)

### IPv6

* Must have up to 8 groups of hexadecimal digits separated by colons
* Double-colon (`::`) can appear at most once to represent consecutive groups of zeros
* Each group must be 1-4 hexadecimal characters
