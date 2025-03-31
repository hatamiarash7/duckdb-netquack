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

# Extract File Extension

This function extracts the file extension from a URL. It will return the file extension without the dot.

```sql
D SELECT extract_extension('http://example.com/image.jpg') AS ext;
┌─────────┐
│   ext   │
│ varchar │
├─────────┤
│ jpg     │
└─────────┘

D SELECT extract_extension('https://site.com/download.exe?source=official') AS ext;
┌─────────┐
│   ext   │
│ varchar │
├─────────┤
│ exe     │
└─────────┘
```
