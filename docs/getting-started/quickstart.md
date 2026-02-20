---
icon: power-off
cover: >-
  https://images.unsplash.com/photo-1461896836934-ffe607ba8211?crop=entropy&cs=srgb&fm=jpg&ixid=M3wxOTcwMjR8MHwxfHNlYXJjaHwxfHxzdGFydHxlbnwwfHx8fDE3MzkxOTYwNDh8MA&ixlib=rb-4.0.3&q=85
coverY: 0
layout:
  cover:
    visible: true
    size: full
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

# Quickstart

### Install DuckDB

First, you need to install DuckDB, it is pretty easy, you can check [this page](https://duckdb.org/docs/installation/?version=stable\&environment=cli\&platform=linux\&download_method=direct\&architecture=x86_64). DuckDB is an in-process database system that offers [client APIs](https://duckdb.org/docs/api/overview) for several languages.

The compatibility between Netquack and DuckDB varies across versions.

| Version of Netquack | Version of DuckDB |
| ------------------- | ----------------- |
| v1.9.0              | v1.4.4            |
| v1.8.1              | v1.4.4            |
| v1.8.0              | v1.4.4            |
| v1.4.0              | v1.4.0            |

### Install Netquack

BlockDuck is one of the available DuckDB community [extensions](https://duckdb.org/community_extensions/list_of_extensions), so we can use `INSTALL`and `LOAD` to install it easily.

```sql
SET allow_community_extensions = true;
INSTALL netquack FROM community;
LOAD netquack;
```

If you previously installed the extension, upgrade using the FORCE command

```sql
FORCE INSTALL netquack FROM community;
LOAD netquack;
```

Also, you can check for any available updates for the extension using this command:

```sql
UPDATE EXTENSIONS (netquack);
```
