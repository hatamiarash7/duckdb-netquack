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

# Get Tranco Rank

You can use this function to get the ranking of a domain:

```sql
D SELECT get_tranco_rank('microsoft.com') ASrank;
┌─────────┐
│  rank   │
│ varchar │
├─────────┤
│ 2       │
└─────────┘

D SELECT get_tranco_rank('cloudflare.com') ASrank;
┌─────────┐
│  rank   │
│ varchar │
├─────────┤
│ 13      │
└─────────┘
```

You can use the `get_tranco_rank_category` function to retrieve the category utility column that gives you the rank category of the domain. The `category` value is on a log10 scale with half steps (e.g. top 1k, top 5k, top 10k, top 50k, top 100k, top 500k, top 1M, top 5m, etc.) with each rank excluding the previous (e.g. top 5k is actually 4k domains, excluding top 1k).

```sql
D SELECT get_tranco_rank_category('microsoft.com') AScategory;
┌──────────┐
│ category │
│ varchar  │
├──────────┤
│ top1k    │
└──────────┘
```
