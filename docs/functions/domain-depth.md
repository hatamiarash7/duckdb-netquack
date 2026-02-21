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

# Domain Depth

This function returns the number of dot-separated levels in a domain. It extracts the host from a URL and counts the labels. Returns `0` for IP addresses, empty strings, and invalid input. Returns `NULL` for `NULL` input.

```sql
D SELECT domain_depth('example.com') AS depth;
┌───────┐
│ depth │
│ int32 │
├───────┤
│   2   │
└───────┘

D SELECT domain_depth('https://www.example.com/page') AS depth;
┌───────┐
│ depth │
│ int32 │
├───────┤
│   3   │
└───────┘

D SELECT domain_depth('http://a.b.c.example.co.uk/page') AS depth;
┌───────┐
│ depth │
│ int32 │
├───────┤
│   6   │
└───────┘
```

## Special Cases

- **IP addresses** return `0` (both IPv4 and IPv6 — they are not domains)
- **Single-label names** like `localhost` return `1`
- **Trailing dots** are stripped before counting (DNS canonical form)

```sql
D SELECT domain_depth('192.168.1.1') AS depth;
┌───────┐
│ depth │
│ int32 │
├───────┤
│   0   │
└───────┘

D SELECT domain_depth('localhost') AS depth;
┌───────┐
│ depth │
│ int32 │
├───────┤
│   1   │
└───────┘
```
