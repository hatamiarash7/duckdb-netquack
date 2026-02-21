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

# Base64 Encode / Decode

The `base64_encode` function encodes a string into Base64 format. The `base64_decode` function decodes a Base64-encoded string back to its original form.

## base64\_encode

```sql
D SELECT base64_encode('Hello World') AS encoded;
┌──────────────────┐
│     encoded      │
│     varchar      │
├──────────────────┤
│ SGVsbG8gV29ybGQ= │
└──────────────────┘
```

```sql
D SELECT base64_encode('https://example.com/path?q=1') AS encoded;
┌──────────────────────────────────────────┐
│                 encoded                  │
│                 varchar                  │
├──────────────────────────────────────────┤
│ aHR0cHM6Ly9leGFtcGxlLmNvbS9wYXRoP3E9MQ== │
└──────────────────────────────────────────┘
```

## base64\_decode

```sql
D SELECT base64_decode('SGVsbG8gV29ybGQ=') AS decoded;
┌─────────────┐
│   decoded   │
│   varchar   │
├─────────────┤
│ Hello World │
└─────────────┘
```

```sql
D SELECT base64_decode('INVALID!') AS decoded;
┌────────────────┐
│    decoded     │
│    varchar     │
├────────────────┤
│ INVALID_BASE64 │
└────────────────┘
```

## Round-trip

You can combine both functions to verify encoding and decoding:

```sql
D SELECT base64_decode(base64_encode('café ☕')) AS roundtrip;
┌───────────┐
│ roundtrip │
│  varchar  │
├───────────┤
│ café ☕    │
└───────────┘
```
