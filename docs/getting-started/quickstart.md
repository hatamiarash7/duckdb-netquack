---
icon: power-off
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

# Quickstart

### Install DuckDB

First, you need to install DuckDB, it is pretty easy, you can check [this page](https://duckdb.org/docs/installation/?version=stable\&environment=cli\&platform=linux\&download_method=direct\&architecture=x86_64). DuckDB is an in-process database system that offers [client APIs](https://duckdb.org/docs/api/overview) for several languages.

The compatibility between Netquack and DuckDB varies across versions.

| Version of Netquack | Version of DuckDB |
| ------------------- | ----------------- |
| v1.0.0              | v1.2.0            |

### Install Netquack

BlockDuck is one of the available DuckDB community [extensions](https://duckdb.org/community_extensions/list_of_extensions), so we can use `INSTALL`and `LOAD` to install it easily.

```sql
INSTALL netquack FROM community;
LOAD netquack;
```

If you previously installed the extension, upgrade using the FORCE command

```sql
FORCE INSTALL netquack FROM community;
LOAD netquack;
```
