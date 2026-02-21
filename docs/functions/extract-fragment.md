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

# Extract Fragment

This function extracts the fragment (the part after `#`) from a URL. Fragments are commonly used for page anchors, single-page application routing, and deep linking.

```sql
D SELECT extract_fragment('http://example.com/page#section') AS fragment;
┌──────────┐
│ fragment │
│ varchar  │
├──────────┤
│ section  │
└──────────┘

D SELECT extract_fragment('http://example.com/path?q=1#results') AS fragment;
┌──────────┐
│ fragment │
│ varchar  │
├──────────┤
│ results  │
└──────────┘

D SELECT extract_fragment('http://example.com/#/users/123/profile') AS fragment;
┌────────────────────┐
│      fragment      │
│      varchar       │
├────────────────────┤
│ /users/123/profile │
└────────────────────┘
```

Returns an empty string if no fragment is present, and `NULL` for `NULL` input.

```sql
D SELECT extract_fragment('http://example.com/path') AS fragment;
┌──────────┐
│ fragment │
│ varchar  │
├──────────┤
│          │
└──────────┘
```
