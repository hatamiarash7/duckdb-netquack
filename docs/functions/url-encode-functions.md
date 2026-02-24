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

# URL Encode / Decode

The `url_encode` function percent-encodes a string per RFC 3986. Only unreserved characters (`A-Z`, `a-z`, `0-9`, `-`, `_`, `.`, `~`) are left as-is — everything else is encoded as `%XX` with uppercase hex digits.

The `url_decode` function decodes percent-encoded strings back to their original form. It also decodes `+` as a space (for `application/x-www-form-urlencoded` compatibility). Invalid percent sequences (e.g., `%ZZ`, trailing `%`) are passed through literally.

## Encode

```sql
D SELECT url_encode('hello world') AS encoded;
┌───────────────┐
│    encoded    │
│    varchar    │
├───────────────┤
│ hello%20world │
└───────────────┘
```

```sql
D SELECT url_encode('café') AS encoded;
┌───────────┐
│  encoded  │
│  varchar  │
├───────────┤
│ caf%C3%A9 │
└───────────┘
```

```sql
D SELECT url_encode('key=value&lang=en') AS encoded;
┌───────────────────────────┐
│          encoded          │
│          varchar          │
├───────────────────────────┤
│ key%3Dvalue%26lang%3Den   │
└───────────────────────────┘
```

## Decode

```sql
D SELECT url_decode('hello%20world') AS decoded;
┌─────────────┐
│   decoded   │
│   varchar   │
├─────────────┤
│ hello world │
└─────────────┘
```

```sql
D SELECT url_decode('hello+world') AS decoded;
┌─────────────┐
│   decoded   │
│   varchar   │
├─────────────┤
│ hello world │
└─────────────┘
```

```sql
D SELECT url_decode('caf%C3%A9') AS decoded;
┌─────────┐
│ decoded │
│ varchar │
├─────────┤
│ café    │
└─────────┘
```

## Round-trip

```sql
D SELECT url_decode(url_encode('https://example.com/path?q=hello world')) AS roundtrip;
┌──────────────────────────────────────────┐
│                roundtrip                 │
│                 varchar                  │
├──────────────────────────────────────────┤
│ https://example.com/path?q=hello world   │
└──────────────────────────────────────────┘
```

Returns an empty string for empty input and `NULL` for `NULL` input.
