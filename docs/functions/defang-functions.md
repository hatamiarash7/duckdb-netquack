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

The `defang` function follows the [CyberChef](https://gchq.github.io/CyberChef/#recipe=Defang_URL()) "Defang URL" convention:

- Rewrites the `http` / `https` scheme to `hxxp` / `hxxps` and `ftp` to `fxp` (letter case preserved).
- Brackets the scheme separator: `://` becomes `[://]`.
- Brackets every `.` as `[.]` and every `@` as `[@]`.
- Brackets port colons (`example.com:443`, `203.0.113.45:443`, `[::1]:8080`) as `[:]`, so they survive round-tripping without being mistaken for the scheme separator.
- Brackets the colons inside IPv6 addresses as `[:]`.
- Is idempotent: input that is already defanged is left unchanged.

Other colons, such as those in `12:30:45`, are left as they are.

The `refang` function reverses the process and accepts the common defanging variants:

| Defanged form                                  | Refanged |
| ---------------------------------------------- | -------- |
| `hxxp`, `hxxps`, `hXXps`, `fxp` (before `://`) | `http`, `https`, `hTTps`, `ftp` |
| `[.]`, `(.)`, `{.}`, `[dot]`, `(dot)`          | `.`      |
| `[:]`                                          | `:`      |
| `[://]`                                        | `://`    |
| `[@]`, `[at]`, `(at)`                          | `@`      |

Bracket keywords are case-insensitive. Mismatched brackets (e.g. `[.)`) and IPv6 literals (e.g. `[2001:db8::1]`) are left untouched.

## Defang

```sql
D SELECT defang('https://malware.example.com/beacon') AS defanged;
┌──────────────────────────────────────────┐
│                 defanged                 │
│                 varchar                  │
├──────────────────────────────────────────┤
│ hxxps[://]malware[.]example[.]com/beacon │
└──────────────────────────────────────────┘
```

```sql
D SELECT defang('invoice@spam-domain.com') AS defanged;
┌─────────────────────────────┐
│          defanged           │
│           varchar           │
├─────────────────────────────┤
│ invoice[@]spam-domain[.]com │
└─────────────────────────────┘
```

```sql
D SELECT defang('203.0.113.45:443') AS defanged;
┌──────────────────────────┐
│         defanged         │
│         varchar          │
├──────────────────────────┤
│ 203[.]0[.]113[.]45[:]443 │
└──────────────────────────┘
```

```sql
D SELECT defang('http://192.168.1.100:8080/beacon') AS defanged;
┌────────────────────────────────────────────┐
│                  defanged                  │
│                  varchar                   │
├────────────────────────────────────────────┤
│ hxxp[://]192[.]168[.]1[.]100[:]8080/beacon │
└────────────────────────────────────────────┘
```

```sql
D SELECT defang('http://[2001:db8::1]:8080/') AS defanged;
┌──────────────────────────────────────┐
│               defanged               │
│               varchar                │
├──────────────────────────────────────┤
│ hxxp[://][2001[:]db8[:][:]1][:]8080/ │
└──────────────────────────────────────┘
```

## Refang

```sql
D SELECT refang('hxxps[://]malware[.]example[.]com/beacon') AS refanged;
┌────────────────────────────────────┐
│              refanged              │
│              varchar               │
├────────────────────────────────────┤
│ https://malware.example.com/beacon │
└────────────────────────────────────┘
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
D SELECT extract_domain(refang('hxxps[://]evil[.]example[.]com/payload')) AS domain;
┌─────────────┐
│   domain    │
│   varchar   │
├─────────────┤
│ example.com │
└─────────────┘
```

Returns an empty string for empty input and `NULL` for `NULL` input.
