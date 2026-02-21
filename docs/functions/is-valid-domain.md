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

# Validate Domain

The `is_valid_domain` function validates a domain name against RFC 1035 / RFC 1123 rules. Returns a `BOOLEAN`.

Validation rules:

- Total length: 1–253 characters
- At least two labels separated by dots (e.g., `example.com`)
- Each label: 1–63 characters, alphanumeric and hyphens only
- Labels must not start or end with a hyphen
- TLD must not be all-numeric (rejects IP addresses)
- Optional trailing dot (FQDN notation) is allowed

```sql
D SELECT is_valid_domain('example.com') AS valid;
┌─────────┐
│  valid  │
│ boolean │
├─────────┤
│ true    │
└─────────┘
```

```sql
D SELECT is_valid_domain('sub.example.co.uk') AS valid;
┌─────────┐
│  valid  │
│ boolean │
├─────────┤
│ true    │
└─────────┘
```

```sql
D SELECT is_valid_domain('localhost') AS valid;
┌─────────┐
│  valid  │
│ boolean │
├─────────┤
│ false   │
└─────────┘
```

```sql
D SELECT is_valid_domain('-bad.com') AS valid;
┌─────────┐
│  valid  │
│ boolean │
├─────────┤
│ false   │
└─────────┘
```

Returns `NULL` for `NULL` input.
