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

# Defang / Refang

Defanging rewrites a URL, domain, or IP address so it can be shared in reports, tickets, or chat without being clickable or auto-linked. It is commonly used when handling indicators of compromise (IOCs).

The `defang` function:

- Replaces the `http` / `https` scheme with `hxxp` / `hxxps` and `ftp` with `fxp` (only when followed by `://`, letter case preserved).
- Replaces every `.` with `[.]` and every `@` with `[@]`.
- Replaces `:` with `[:]` inside IPv6 addresses (scheme separators and ports are left alone).
- Is idempotent — already defanged input is left unchanged.

The `refang` function reverses the process and accepts the common defanging variants:

| Defanged form                              | Refanged |
| ------------------------------------------ | -------- |
| `hxxp://`, `hxxps://`, `hXXps://`, `fxp://` | `http://`, `https://`, `hTTps://`, `ftp://` |
| `[.]`, `(.)`, `{.}`, `[dot]`, `(dot)`       | `.`      |
| `[:]`                                      | `:`      |
| `[://]`                                    | `://`    |
| `[@]`, `[at]`, `(at)`                       | `@`      |

Bracket keywords are case-insensitive. Mismatched brackets (e.g. `[.)`) and IPv6 literals (e.g. `[2001:db8::1]`) are left untouched.

## Defang

```sql
D SELECT defang('https://example.com/path') AS defanged;
┌────────────────────────────┐
│          defanged          │
│          varchar           │
├────────────────────────────┤
│ hxxps://example[.]com/path │
└────────────────────────────┘
```

```sql
D SELECT defang('192.168.1.1') AS defanged;
┌───────────────────┐
│     defanged      │
│      varchar      │
├───────────────────┤
│ 192[.]168[.]1[.]1 │
└───────────────────┘
```

```sql
D SELECT defang('http://user@[2001:db8::1]:8080/') AS defanged;
┌─────────────────────────────────────────┐
│                defanged                 │
│                 varchar                 │
├─────────────────────────────────────────┤
│ hxxp://user[@][2001[:]db8[:][:]1]:8080/ │
└─────────────────────────────────────────┘
```

## Refang

```sql
D SELECT refang('hxxps://example[.]com/path') AS refanged;
┌──────────────────────────┐
│         refanged         │
│         varchar          │
├──────────────────────────┤
│ https://example.com/path │
└──────────────────────────┘
```

```sql
D SELECT refang('user[at]example(dot)com') AS refanged;
┌──────────────────┐
│     refanged     │
│     varchar      │
├──────────────────┤
│ user@example.com │
└──────────────────┘
```

Refang IOCs before feeding them to other Netquack functions:

```sql
D SELECT extract_domain(refang('hxxps://evil[.]example[.]com/payload')) AS domain;
┌─────────────┐
│   domain    │
│   varchar   │
├─────────────┤
│ example.com │
└─────────────┘
```

Returns an empty string for empty input and `NULL` for `NULL` input.
