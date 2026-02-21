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

# Validate URL

The `is_valid_url` function checks whether a string is a well-formed URL with a scheme, authority (host), and optional port/path/query/fragment. Returns a `BOOLEAN`.

A valid URL must have:

- A scheme (e.g., `http`, `https`, `ftp`, or any valid scheme)
- The `://` separator
- A non-empty host (domain name, IPv4, or bracketed IPv6)

```sql
D SELECT is_valid_url('https://example.com') AS valid;
┌─────────┐
│  valid  │
│ boolean │
├─────────┤
│ true    │
└─────────┘
```

```sql
D SELECT is_valid_url('https://example.com:8080/path?q=1#frag') AS valid;
┌─────────┐
│  valid  │
│ boolean │
├─────────┤
│ true    │
└─────────┘
```

```sql
D SELECT is_valid_url('not a url') AS valid;
┌─────────┐
│  valid  │
│ boolean │
├─────────┤
│ false   │
└─────────┘
```

```sql
D SELECT is_valid_url('example.com') AS valid;
┌─────────┐
│  valid  │
│ boolean │
├─────────┤
│ false   │
└─────────┘
```

Returns `NULL` for `NULL` input.
