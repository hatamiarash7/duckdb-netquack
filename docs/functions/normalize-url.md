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

# Normalize URL

This function canonicalizes a URL by applying the following normalizations per [RFC 3986](https://www.rfc-editor.org/rfc/rfc3986):

- **Scheme lowercasing** — `HTTP` → `http`
- **Host lowercasing** — `EXAMPLE.COM` → `example.com`
- **Trailing dot removal** — `example.com.` → `example.com`
- **Default port removal** — `:80` for HTTP, `:443` for HTTPS, `:21` for FTP
- **Trailing slash removal** — `/path/` → `/path`
- **Dot segment removal** — `/a/b/../c` → `/a/c`
- **Query parameter sorting** — `?z=1&a=2` → `?a=2&z=1`
- **Fragment removal** — `#section` is dropped
- **Percent-encoding normalization** — Unreserved characters are decoded, reserved characters use uppercase hex

```sql
D SELECT normalize_url('HTTP://WWW.EXAMPLE.COM:80/a/b/../c/?z=1&a=2#frag') AS url;
┌────────────────────────────────────┐
│                url                 │
│              varchar               │
├────────────────────────────────────┤
│ http://www.example.com/a/c?a=2&z=1 │
└────────────────────────────────────┘

D SELECT normalize_url('HTTPS://Example.Com:443/path/./to/../page?b=2&a=1#section') AS url;
┌───────────────────────────────────────┐
│                  url                  │
│                varchar                │
├───────────────────────────────────────┤
│ https://example.com/path/page?a=1&b=2 │
└───────────────────────────────────────┘

D SELECT normalize_url('http://example.com/%7Euser') AS url;
┌──────────────────────────┐
│           url            │
│         varchar          │
├──────────────────────────┤
│ http://example.com/~user │
└──────────────────────────┘
```

## Deduplication Use Case

`normalize_url` is especially useful for finding duplicate URLs that differ only in formatting:

```sql
D CREATE TABLE urls AS
    SELECT 'HTTP://Example.COM:80/Path?z=1&a=2#frag' AS url UNION ALL
    SELECT 'http://example.com/Path?a=2&z=1' UNION ALL
    SELECT 'https://TEST.org/page';

D SELECT normalize_url(url) AS normalized, count(*) AS cnt
  FROM urls
  GROUP BY normalized;
┌────────────────────────────────────┬───────┐
│            normalized              │  cnt  │
│              varchar               │ int64 │
├────────────────────────────────────┼───────┤
│ http://example.com/Path?a=2&z=1   │     2 │
│ https://test.org/page              │     1 │
└────────────────────────────────────┴───────┘
```
