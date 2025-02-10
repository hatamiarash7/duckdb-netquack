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

This function extracts the query string from a URL.

```sql
D SELECT extract_query_string('example.com?key=value') as query;
┌───────────┐
│   query   │
│  varchar  │
├───────────┤
│ key=value │
└───────────┘

D SELECT extract_query_string('http://example.com.ac/path/?a=1&b=2&') as query;
┌──────────┐
│  query   │
│ varchar  │
├──────────┤
│ a=1&b=2& │
└──────────┘
```
