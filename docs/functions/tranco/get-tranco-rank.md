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
D select get_tranco_rank('microsoft.com') as rank;
┌───────┐
│ rank  │
│ int32 │
├───────┤
│     2 │
└───────┘

D select get_tranco_rank('cloudflare.com') as rank;
┌───────┐
│ rank  │
│ int32 │
├───────┤
│    13 │
└───────┘
```
