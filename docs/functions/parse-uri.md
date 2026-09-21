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

# Parse URI

This function parses a URI and returns a `STRUCT` with `scheme`, `host`, `port`, `path`, `query`, and `fragment` in a single call. Missing components are empty strings. `NULL` input returns `NULL`.

Scheme and host are lowercased. Path, query, and fragment keep their original case. Userinfo (`user:pass@`) is skipped. Opaque URIs such as `mailto:` and `tel:` put the remainder in `path`.

```sql
D SELECT parse_uri('https://example.com:8080/path?q=1#section') AS uri;
┌──────────────────────────────────────────────────────────────────────────────────────────────────────────┐
│                                                   uri                                                    │
│    struct(scheme varchar, host varchar, port varchar, path varchar, query varchar, fragment varchar)     │
├──────────────────────────────────────────────────────────────────────────────────────────────────────────┤
│ {'scheme': https, 'host': example.com, 'port': 8080, 'path': /path, 'query': 'q=1', 'fragment': section} │
└──────────────────────────────────────────────────────────────────────────────────────────────────────────┘
```

Access individual fields with dot notation:

```sql
D SELECT parse_uri('https://example.com:8080/path?q=1#section').scheme AS scheme;
┌─────────┐
│ scheme  │
│ varchar │
├─────────┤
│ https   │
└─────────┘

D SELECT uri.host, uri.port, uri.path
  FROM (SELECT parse_uri('https://User:Pass@WWW.Example.COM:8080/Path') AS uri);
┌─────────────────┬─────────┬─────────┐
│      host       │  port   │  path   │
│     varchar     │ varchar │ varchar │
├─────────────────┼─────────┼─────────┤
│ www.example.com │ 8080    │ /Path   │
└─────────────────┴─────────┴─────────┘
```

IPv6 literals keep their brackets. Localhost and custom schemes (`ws`, `sftp`, `git+ssh`) are supported.

```sql
D SELECT parse_uri('http://[2001:db8::1]:8080/path').host AS host;
┌───────────────┐
│     host      │
│    varchar    │
├───────────────┤
│ [2001:db8::1] │
└───────────────┘

D SELECT parse_uri('mailto:someone@example.com').path AS path;
┌─────────────────────┐
│        path         │
│       varchar       │
├─────────────────────┤
│ someone@example.com │
└─────────────────────┘
```
