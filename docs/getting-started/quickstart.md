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

First, you need to install DuckDB, it is pretty easy, you can check this: [duckdb](https://duckdb.org/docs/installation/?version=stable\&environment=cli\&platform=macos\&download_method=package_manager).

DuckDB is an in-process database system and offers client APIs for several languages: [clients api overview](https://duckdb.org/docs/api/overview)

The compatibility between Netquack and DuckDB varies across versions. Refer to the list below to identify the correct BlockDB version compatible with your chosen Netquack.

| Version of Netquack | Version of DuckDB |
| ------------------- | ----------------- |
| v1.0.0              | v1.2.0            |

### Install Netquack

BlockDuck is one of the available DuckDB community [extensions](https://duckdb.org/community_extensions/list_of_extensions), so we can use `INSTALL`and `LOAD` to install it easily.

```
INSTALL netquack FROM community;
LOAD netquack;
```

If you previously installed the extension, upgrade using the FORCE command

```
FORCE INSTALL netquack FROM community;
LOAD netquack;
```
