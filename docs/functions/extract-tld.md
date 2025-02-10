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

# Extract TLD

This function extracts the top-level domain from a URL. This function will use the public suffix list to extract the TLD. Check the [Extracting The Main Domain](extract-domain.md) section for more information about the public suffix list.

```sql
D SELECT extract_tld('https://example.com.ac/path/path') as tld;
┌─────────┐
│   tld   │
│ varchar │
├─────────┤
│ com.ac  │
└─────────┘

D SELECT extract_tld('a.example.com') as tld;
┌─────────┐
│   tld   │
│ varchar │
├─────────┤
│ com     │
└─────────┘
```
