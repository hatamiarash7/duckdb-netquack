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

# URL to SURT / SURT to URL

## URL to SURT

The `url_to_surt` function converts a URL into a [SURT](https://heritrix.readthedocs.io/en/latest/glossary.html#term-SURT) (Sort-friendly URI Reordering Transform) key, the canonical form web archives such as the Internet Archive use in CDX indexes. Sorting by SURT groups URLs by domain, then subdomain, then path.

The transformation matches the Internet Archive canonicalizer (the Python [`surt`](https://github.com/internetarchive/surt) library defaults):

- The scheme, userinfo, and fragment are dropped.
- The host is lowercased, a leading `www.` / `www<digits>.` label is removed, and the labels are reversed and joined with commas (`www.example.com` → `com,example`). IP addresses are kept as-is.
- Default ports (`80` for `http`, `443` for `https`, `21` for `ftp`) are dropped. Other ports are kept (`com,example:8080`).
- The host is closed with `)`, followed by the lowercased path. Trailing slashes are removed, and an empty path becomes `/`.
- Query parameters are sorted and lowercased. An empty query is dropped.

Input without a host (for example `/relative/path`) is returned unchanged.

```sql
D SELECT url_to_surt('https://www.Example.com/Path/?b=2&a=1#frag') AS surt;
┌───────────────────────────┐
│           surt            │
│          varchar          │
├───────────────────────────┤
│ com,example)/path?a=1&b=2 │
└───────────────────────────┘

D SELECT url_to_surt('http://blog.example.co.uk:8080/') AS surt;
┌───────────────────────────┐
│           surt            │
│          varchar          │
├───────────────────────────┤
│ uk,co,example,blog:8080)/ │
└───────────────────────────┘
```

## SURT to URL

The `surt_to_url` function reverses `url_to_surt`: it turns the comma-separated host back into a dotted host, keeps any port, and appends the path and query.

A SURT key does not store the scheme, so `http://` is used. The Heritrix form that includes a scheme (`https://(com,example,)/path`) is also accepted, and its scheme is kept. Anything removed by `url_to_surt` (`www.`, userinfo, default port, fragment, original letter case, and query order) cannot be restored. Input that is not a SURT key is returned unchanged.

```sql
D SELECT surt_to_url('com,example)/path?a=1&b=2') AS url;
┌─────────────────────────────────┐
│               url               │
│             varchar             │
├─────────────────────────────────┤
│ http://example.com/path?a=1&b=2 │
└─────────────────────────────────┘

D SELECT surt_to_url('https://(uk,co,example,blog,:8080)/') AS url;
┌──────────────────────────────────┐
│               url                │
│             varchar              │
├──────────────────────────────────┤
│ https://blog.example.co.uk:8080/ │
└──────────────────────────────────┘
```
