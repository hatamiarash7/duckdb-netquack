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

# Extract SLD

This function extracts the second-level domain (SLD) from a URL: the label just before the public suffix. It uses the public suffix list, so multi-part suffixes such as `co.uk` or `com.au` are handled correctly. Check the [Extracting The Main Domain](extract-domain.md) section for more information about the public suffix list.

It returns an empty string when there is no label before the suffix (e.g. `co.uk`, `localhost`, or an IP address).

```sql
D SELECT extract_sld('https://mail.google.co.uk/inbox') AS sld;
┌─────────┐
│   sld   │
│ varchar │
├─────────┤
│ google  │
└─────────┘
```

This is handy for matching a brand across TLDs:

```sql
D SELECT extract_sld(url) AS brand, count(*) AS hits
  FROM (VALUES ('https://www.google.com'), ('https://mail.google.co.uk'), ('https://google.de')) t(url)
  GROUP BY brand;
┌─────────┬───────┐
│  brand  │ hits  │
│ varchar │ int64 │
├─────────┼───────┤
│ google  │     3 │
└─────────┴───────┘
```
