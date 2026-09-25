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

# Tranco List

The `tranco_list` table function exposes the cached Tranco list as `rank`, `domain`, and `category` columns. Use it to join the ranking against your own data instead of calling `get_tranco_rank` row by row.

Run `SELECT update_tranco(true);` first to download and cache the list.

```sql
D SELECT * FROM tranco_list() LIMIT 3;
┌───────┬───────────────┬──────────┐
│ rank  │    domain     │ category │
│ int32 │    varchar    │ varchar  │
├───────┼───────────────┼──────────┤
│     1 │ google.com    │ top1k    │
│     2 │ microsoft.com │ top1k    │
│     3 │ mail.ru       │ top1k    │
└───────┴───────────────┴──────────┘
```

Join it with your own data:

```sql
D SELECT l.url, t.rank, t.category
  FROM logs l
  LEFT JOIN tranco_list() t ON t.domain = extract_domain(l.url);
```

The `category` value uses the same log-scale buckets as `get_tranco_rank_category` (top1k, top5k, top10k, …).
