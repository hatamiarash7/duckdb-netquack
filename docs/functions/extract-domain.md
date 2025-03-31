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

# Extract Domain

This function extracts the main domain from a URL. For this purpose, the extension will get all public suffixes from the [publicsuffix.org](https://publicsuffix.org/) list and extract the main domain from the URL.

The download process of the public suffix list is done automatically when the function is called for the first time. After that, the list is stored in the `public_suffix_list` table to avoid downloading it again.

```sql
D SELECT extract_domain('a.example.com') AS domain;
┌─────────────┐
│   domain    │
│   varchar   │
├─────────────┤
│ example.com │
└─────────────┘

D SELECT extract_domain('https://b.a.example.com/path') AS domain;
┌─────────────┐
│   domain    │
│   varchar   │
├─────────────┤
│ example.com │
└─────────────┘
```
