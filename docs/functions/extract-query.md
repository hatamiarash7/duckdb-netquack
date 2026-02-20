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

# Extract Query

## Query Strings

The `extract_query_string` function extracts the query string from a URL as a single string.

```sql
D SELECT extract_query_string('example.com?key=value') AS query;
┌───────────┐
│   query   │
│  varchar  │
├───────────┤
│ key=value │
└───────────┘

D SELECT extract_query_string('http://example.com.ac/path/?a=1&b=2') AS query;
┌─────────┐
│  query  │
│ varchar │
├─────────┤
│ a=1&b=2 │
└─────────┘
```

## Query Parameters

The `extract_query_parameters` table function parses the query string and returns each key-value pair as a separate row. This is useful for analyzing URL parameters in a structured way.

```sql
D SELECT * FROM extract_query_parameters('http://example.com/path/?a=1&b=2');
┌─────────┬─────────┐
│   key   │  value  │
│ varchar │ varchar │
├─────────┼─────────┤
│ a       │ 1       │
│ b       │ 2       │
└─────────┴─────────┘

D SELECT * FROM extract_query_parameters('https://example.com/search?q=duckdb&hl=en&num=10');
┌─────────┬─────────┐
│   key   │  value  │
│ varchar │ varchar │
├─────────┼─────────┤
│ q       │ duckdb  │
│ hl      │ en      │
│ num     │ 10      │
└─────────┴─────────┘

D SELECT m.media_url,
  e.key,
  e.value
FROM instagram_posts m,
  LATERAL extract_query_parameters(m.media_url) e
ORDER BY m.id;

┌───────────────────────────────────────────────────────────────────────────────────────────┬────────────┬───────────┐
│                                         media_url                                         │    key     │   value   │
│                                          varchar                                          │  varchar   │  varchar  │
├───────────────────────────────────────────────────────────────────────────────────────────┼────────────┼───────────┤
│ https://cdn.instagram.com/media/abc123.jpg?utm_source=instagram&utm_medium=social&id=1001 │ id         │ 1001      │
│ https://cdn.instagram.com/media/abc123.jpg?utm_source=instagram&utm_medium=social&id=1001 │ utm_medium │ social    │
│ https://cdn.instagram.com/media/abc123.jpg?utm_source=instagram&utm_medium=social&id=1001 │ utm_source │ instagram │
│ https://cdn.instagram.com/media/def456.jpg?quality=hd&format=webp&user=arash              │ user       │ arash     │
│ https://cdn.instagram.com/media/def456.jpg?quality=hd&format=webp&user=arash              │ format     │ webp      │
│ https://cdn.instagram.com/media/def456.jpg?quality=hd&format=webp&user=arash              │ quality    │ hd        │
│ https://cdn.instagram.com/media/ghi789.mp4?autoplay=true&loop=false&session_id=xyz987     │ session_id │ xyz987    │
│ https://cdn.instagram.com/media/ghi789.mp4?autoplay=true&loop=false&session_id=xyz987     │ loop       │ false     │
│ https://cdn.instagram.com/media/ghi789.mp4?autoplay=true&loop=false&session_id=xyz987     │ autoplay   │ true      │
└───────────────────────────────────────────────────────────────────────────────────────────┴────────────┴───────────┘
```
