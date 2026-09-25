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

# Public Suffix / Known TLD

These functions check a name against the [Public Suffix List](https://publicsuffix.org/) bundled with the extension. Both are case-insensitive, accept a leading or trailing dot (`.com`, `co.uk.`), return a `BOOLEAN`, and return `NULL` for `NULL` input.

## is\_public\_suffix

Returns `true` if the input is exactly a public suffix: an ICANN suffix like `com` or `co.uk`, or a private suffix like `github.io`. Registrable domains and hosts such as `example.co.uk` return `false`.

```sql
D SELECT is_public_suffix('co.uk') AS suffix, is_public_suffix('example.co.uk') AS registrable;
┌─────────┬─────────────┐
│ suffix  │ registrable │
│ boolean │   boolean   │
├─────────┼─────────────┤
│ true    │ false       │
└─────────┴─────────────┘

D SELECT is_public_suffix('github.io') AS private_suffix;
┌────────────────┐
│ private_suffix │
│    boolean     │
├────────────────┤
│ true           │
└────────────────┘
```

Wildcard and exception rules are honored. For example, `*.ck` makes every `<label>.ck` a public suffix, except `www.ck`, which is excluded by `!www.ck`:

```sql
D SELECT is_public_suffix('foo.ck') AS wildcard, is_public_suffix('www.ck') AS exception;
┌──────────┬───────────┐
│ wildcard │ exception │
│ boolean  │  boolean  │
├──────────┼───────────┤
│ true     │ false     │
└──────────┴───────────┘
```

The input is treated as a bare domain, not a URL. Use `extract_host` first if you have full URLs:

```sql
D SELECT is_public_suffix(extract_host('https://co.uk/path')) AS suffix;
```

## is\_known\_tld

Returns `true` if the input is a single label that is a top-level domain in the list (e.g. `com`, `uk`, `ck`, `рф`). Multi-label suffixes like `co.uk` return `false`. Internationalized TLDs must be given in Unicode form (`рф`), not Punycode (`xn--p1ai`).

```sql
D SELECT is_known_tld('com') AS tld, is_known_tld('co.uk') AS multi_label, is_known_tld('notarealtld') AS unknown;
┌─────────┬─────────────┬─────────┐
│   tld   │ multi_label │ unknown │
│ boolean │   boolean   │ boolean │
├─────────┼─────────────┼─────────┤
│ true    │ false       │ false   │
└─────────┴─────────────┴─────────┘
```
