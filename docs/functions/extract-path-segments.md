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

# Extract Path Segments

The `extract_path_segments` table function splits a URL path into individual segment rows. Each row contains a 1-based `segment_index` and the `segment` string. This is useful for analyzing URL structures, filtering by path depth, or joining path components.

```sql
D SELECT * FROM extract_path_segments('https://example.com/path/to/page?q=1');
┌───────────────┬─────────┐
│ segment_index │ segment │
│     int32     │ varchar │
├───────────────┼─────────┤
│             1 │ path    │
│             2 │ to      │
│             3 │ page    │
└───────────────┴─────────┘
```

```sql
D SELECT * FROM extract_path_segments('https://api.example.com/v3/users/42/repos');
┌───────────────┬─────────┐
│ segment_index │ segment │
│     int32     │ varchar │
├───────────────┼─────────┤
│             1 │ v3      │
│             2 │ users   │
│             3 │ 42      │
│             4 │ repos   │
└───────────────┴─────────┘
```

## LATERAL Join

Use with `LATERAL` to expand path segments per row in a table:

```sql
D SELECT u.url,
      s.segment_index,
      s.segment
  FROM urls u,
      LATERAL extract_path_segments(u.url) s
  ORDER BY u.url,
      s.segment_index;
┌───────────────────────────┬───────────────┬─────────┐
│            url            │ segment_index │ segment │
│          varchar          │     int32     │ varchar │
├───────────────────────────┼───────────────┼─────────┤
│ https://example.com/a/b/c │             1 │ a       │
│ https://example.com/a/b/c │             2 │ b       │
│ https://example.com/a/b/c │             3 │ c       │
│ https://test.org/x/y      │             1 │ x       │
│ https://test.org/x/y      │             2 │ y       │
└───────────────────────────┴───────────────┴─────────┘
```

Returns 0 rows for URLs with no path, root path (`/`), empty strings, and `NULL` input.
