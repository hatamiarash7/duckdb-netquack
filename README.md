# DuckDB Netquack Extension

[![DuckDB Badge](https://img.shields.io/badge/Built_With-DuckDB-fff100)](https://duckdb.org/community_extensions/extensions/netquack.html) [![GitHub License](https://img.shields.io/github/license/hatamiarash7/duckdb-netquack)](https://github.com/hatamiarash7/duckdb-netquack/blob/main/LICENSE) [![GitHub Release](https://img.shields.io/github/v/release/hatamiarash7/duckdb-netquack)](https://github.com/hatamiarash7/duckdb-netquack/releases/latest)

![logo](./.github/logo.jpg)

This extension is designed to simplify working with domains, URIs, and web paths directly within your database queries. Whether you're extracting top-level domains (TLDs), parsing URI components, or analyzing web paths, Netquack provides a suite of intuitive functions to handle all your network tasks efficiently. Built for data engineers, analysts, and developers.

With Netquack, you can unlock deeper insights from your web-related datasets without the need for external tools or complex workflows.

NetQuack uses ClickHouse-inspired character-by-character parsing and gperf-generated perfect hash functions for optimal performance.

Table of Contents

- [DuckDB Netquack Extension](#duckdb-netquack-extension)
  - [Installation 🚀](#installation-)
  - [Usage Examples 📚](#usage-examples-)
    - [Extracting The Main Domain](#extracting-the-main-domain)
    - [Extracting The Path](#extracting-the-path)
    - [Extracting The Host](#extracting-the-host)
    - [Extracting The Schema](#extracting-the-schema)
    - [Extracting The Query](#extracting-the-query)
      - [Query String](#query-string)
      - [Query Parameters](#query-parameters)
    - [Extracting The Port](#extracting-the-port)
    - [Extracting The File Extension](#extracting-the-file-extension)
    - [Extracting The TLD (Top-Level Domain)](#extracting-the-tld-top-level-domain)
    - [Extracting The Sub Domain](#extracting-the-sub-domain)
    - [Extracting The SLD (Second-Level Domain)](#extracting-the-sld-second-level-domain)
    - [Extracting The Fragment](#extracting-the-fragment)
    - [Get Tranco Rank](#get-tranco-rank)
      - [Update Tranco List](#update-tranco-list)
      - [Get Tranco Ranking](#get-tranco-ranking)
      - [Tranco List](#tranco-list)
    - [IP Address Functions](#ip-address-functions)
      - [IP Calculator](#ip-calculator)
      - [Validate IP Address](#validate-ip-address)
      - [Check Private IP](#check-private-ip)
      - [IP Version](#ip-version)
      - [IP to Integer / Integer to IP](#ip-to-integer--integer-to-ip)
      - [IP in Range](#ip-in-range)
      - [IP to PTR](#ip-to-ptr)
      - [IPv6 Compress / Expand](#ipv6-compress--expand)
      - [IP Type / Bogon](#ip-type--bogon)
      - [IP Anonymize](#ip-anonymize)
    - [Normalize URL](#normalize-url)
    - [Domain Depth](#domain-depth)
    - [Base64 Encode / Decode](#base64-encode--decode)
    - [Validate URL](#validate-url)
    - [Validate Domain](#validate-domain)
    - [Public Suffix / Known TLD](#public-suffix--known-tld)
    - [Extract Path Segments](#extract-path-segments)
    - [Parse URI](#parse-uri)
    - [URL Encode / Decode](#url-encode--decode)
    - [Defang / Refang](#defang--refang)
    - [URL to SURT / SURT to URL](#url-to-surt--surt-to-url)
    - [Extract Indicators From Text](#extract-indicators-from-text)
    - [Get Extension Version](#get-extension-version)
  - [Build Requirements](#build-requirements)
  - [Debugging](#debugging)
  - [Roadmap 🗺️](#roadmap-️)
  - [Contributing 🤝](#contributing-)
  - [Issues 🐛](#issues-)

## Installation 🚀

**netquack** is distributed as a [DuckDB Community Extension](https://duckdb.org/community_extensions/) and can be installed using SQL:

```sql
SET allow_community_extensions = true;
INSTALL netquack FROM community;
LOAD netquack;
```

If you previously installed the `netquack` extension, upgrade using the FORCE command

```sql
FORCE INSTALL netquack FROM community;
LOAD netquack;
```

Also, you can check for any available updates for the extension using this command:

```sql
UPDATE EXTENSIONS (netquack);
```

The compatibility between Netquack and DuckDB varies across versions.

| Version of Netquack | Version of DuckDB |
| ------------------- | ----------------- |
| v1.15.0             | v1.5.5            |
| v1.14.0             | v1.5.5            |
| v1.13.0             | v1.5.5            |
| v1.12.1             | v1.5.3            |
| v1.12.0             | v1.5.2            |
| v1.11.2             | v1.5.2            |

## Usage Examples 📚

Once installed, the [macro functions](https://duckdb.org/community_extensions/extensions/netquack.html#added-functions) provided by the extension can be used just like built-in functions.

Function names, parameter names, descriptions, examples, and categories are also available in-catalog:

```sql
D SELECT function_name, parameters, description, categories
  FROM duckdb_functions()
  WHERE function_name = 'extract_domain';
┌────────────────┬────────────┬──────────────────────────────────────────────────────────────────────────┬────────────┐
│ function_name  │ parameters │                               description                                │ categories │
│    varchar     │ varchar[]  │                                 varchar                                  │ varchar[]  │
├────────────────┼────────────┼──────────────────────────────────────────────────────────────────────────┼────────────┤
│ extract_domain │ [url]      │ Extracts the registrable domain from a URL using the Public Suffix List. │ [url]      │
└────────────────┴────────────┴──────────────────────────────────────────────────────────────────────────┴────────────┘
```

### Extracting The Main Domain

This function extracts the main domain from a URL using an optimized static TLD lookup system. The extension uses Mozilla's Public Suffix List compiled into a gperf-generated perfect hash function for O(1) TLD lookups with zero collisions.

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

The TLD lookup is built into the extension at compile time using the latest Mozilla Public Suffix List. No runtime downloads or database operations are required.

### Extracting The Path

This function extracts the path from a URL.

```sql
D SELECT extract_path('https://b.a.example.com/path/path') AS path;
┌────────────┐
│    path    │
│  varchar   │
├────────────┤
│ /path/path │
└────────────┘

D SELECT extract_path('example.com/path/path/image.png') AS path;
┌──────────────────────┐
│         path         │
│       varchar        │
├──────────────────────┤
│ /path/path/image.png │
└──────────────────────┘
```

### Extracting The Host

This function extracts the host from a URL.

```sql
D SELECT extract_host('https://b.a.example.com/path/path') AS host;
┌─────────────────┐
│      host       │
│     varchar     │
├─────────────────┤
│ b.a.example.com │
└─────────────────┘

D SELECT extract_host('example.com:443/path/image.png') AS host;
┌─────────────┐
│    host     │
│   varchar   │
├─────────────┤
│ example.com │
└─────────────┘
```

### Extracting The Schema

This function extracts the schema from a URL. Supported schemas for now:

- `http` | `https`
- `ftp`
- `mailto`
- `tel` | `sms`

```sql
D SELECT extract_schema('https://b.a.example.com/path/path') AS schema;
┌─────────┐
│ schema  │
│ varchar │
├─────────┤
│ https   │
└─────────┘

D SELECT extract_schema('mailto:someone@example.com') AS schema;
┌─────────┐
│ schema  │
│ varchar │
├─────────┤
│ mailto  │
└─────────┘

D SELECT extract_schema('tel:+123456789') AS schema;
┌─────────┐
│ schema  │
│ varchar │
├─────────┤
│ tel     │
└─────────┘
```

### Extracting The Query

#### Query String

The `extract_query_string` function extracts the query string from a URL as a single string.

```sql
D SELECT extract_query_string('example.com?key=value') AS query;
┌───────────┐
│   query   │
│  varchar  │
├───────────┤
│ key=value │
└───────────┘

D SELECT extract_query_string('http://example.com.ac/path/?a=1&b=2') AS query;
┌─────────┐
│  query  │
│ varchar │
├─────────┤
│ a=1&b=2 │
└─────────┘
```

#### Query Parameters

The `extract_query_parameters` table function parses the query string and returns each key-value pair as a separate row. This is useful for analyzing URL parameters in a structured way.

```sql
D SELECT * FROM extract_query_parameters('http://example.com/path/?a=1&b=2');
┌─────────┬─────────┐
│   key   │  value  │
│ varchar │ varchar │
├─────────┼─────────┤
│ a       │ 1       │
│ b       │ 2       │
└─────────┴─────────┘

D SELECT * FROM extract_query_parameters('https://example.com/search?q=duckdb&hl=en&num=10');
┌─────────┬─────────┐
│   key   │  value  │
│ varchar │ varchar │
├─────────┼─────────┤
│ q       │ duckdb  │
│ hl      │ en      │
│ num     │ 10      │
└─────────┴─────────┘

D SELECT m.media_url,
  e.key,
  e.value
FROM instagram_posts m,
  LATERAL extract_query_parameters(m.media_url) e
ORDER BY m.id;

┌───────────────────────────────────────────────────────────────────────────────────────────┬────────────┬───────────┐
│                                         media_url                                         │    key     │   value   │
│                                          varchar                                          │  varchar   │  varchar  │
├───────────────────────────────────────────────────────────────────────────────────────────┼────────────┼───────────┤
│ https://cdn.instagram.com/media/abc123.jpg?utm_source=instagram&utm_medium=social&id=1001 │ id         │ 1001      │
│ https://cdn.instagram.com/media/abc123.jpg?utm_source=instagram&utm_medium=social&id=1001 │ utm_medium │ social    │
│ https://cdn.instagram.com/media/abc123.jpg?utm_source=instagram&utm_medium=social&id=1001 │ utm_source │ instagram │
│ https://cdn.instagram.com/media/def456.jpg?quality=hd&format=webp&user=arash              │ user       │ arash     │
│ https://cdn.instagram.com/media/def456.jpg?quality=hd&format=webp&user=arash              │ format     │ webp      │
│ https://cdn.instagram.com/media/def456.jpg?quality=hd&format=webp&user=arash              │ quality    │ hd        │
│ https://cdn.instagram.com/media/ghi789.mp4?autoplay=true&loop=false&session_id=xyz987     │ session_id │ xyz987    │
│ https://cdn.instagram.com/media/ghi789.mp4?autoplay=true&loop=false&session_id=xyz987     │ loop       │ false     │
│ https://cdn.instagram.com/media/ghi789.mp4?autoplay=true&loop=false&session_id=xyz987     │ autoplay   │ true      │
└───────────────────────────────────────────────────────────────────────────────────────────┴────────────┴───────────┘
```

### Extracting The Port

This function extracts the port from a URL.

```sql
D SELECT extract_port('https://example.com:8443/') AS port;
┌─────────┐
│  port   │
│ varchar │
├─────────┤
│ 8443    │
└─────────┘

D SELECT extract_port('[::1]:6379') AS port;
┌─────────┐
│  port   │
│ varchar │
├─────────┤
│ 6379    │
└─────────┘
```

### Extracting The File Extension

This function extracts the file extension from a URL. It will return the file extension without the dot.

```sql
D SELECT extract_extension('http://example.com/image.jpg') AS ext;
┌─────────┐
│   ext   │
│ varchar │
├─────────┤
│ jpg     │
└─────────┘
```

### Extracting The TLD (Top-Level Domain)

This function extracts the top-level domain from a URL using the optimized gperf-based public suffix lookup system. The function correctly handles multi-part TLDs (like `com.au`) using the longest-match algorithm from Mozilla's Public Suffix List.

```sql
D SELECT extract_tld('https://example.com.ac/path/path') AS tld;
┌─────────┐
│   tld   │
│ varchar │
├─────────┤
│ com.ac  │
└─────────┘

D SELECT extract_tld('a.example.com') AS tld;
┌─────────┐
│   tld   │
│ varchar │
├─────────┤
│ com     │
└─────────┘
```

### Extracting The Sub Domain

This function extracts the sub-domain from a URL using the optimized public suffix lookup system to correctly identify the domain boundary and extract everything before it.

```sql
D SELECT extract_subdomain('http://a.b.example.com/path') AS dns_record;
┌────────────┐
│ dns_record │
│  varchar   │
├────────────┤
│ a.b        │
└────────────┘

D SELECT extract_subdomain('test.example.com.ac') AS dns_record;
┌────────────┐
│ dns_record │
│  varchar   │
├────────────┤
│ test       │
└────────────┘
```

### Extracting The SLD (Second-Level Domain)

This function extracts the label just before the public suffix, e.g. `google` from `mail.google.co.uk`. It's handy for matching a brand across TLDs. An empty string is returned when there is no such label (e.g. `co.uk`, `localhost`, or an IP address).

```sql
D SELECT extract_sld('https://mail.google.co.uk/inbox') AS sld;
┌─────────┐
│   sld   │
│ varchar │
├─────────┤
│ google  │
└─────────┘

D SELECT extract_sld('www.example.com.au') AS sld;
┌─────────┐
│   sld   │
│ varchar │
├─────────┤
│ example │
└─────────┘
```

### Extracting The Fragment

The `extract_fragment` function extracts the fragment (the part after `#`) from a URL. Fragments are commonly used for page anchors, SPA routing, and deep linking.

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

Returns an empty string when no fragment is present, and `NULL` for `NULL` input.

### Get Tranco Rank

#### Update Tranco List

This function returns the [Tranco](https://tranco-list.eu/) rank of a domain. You have an `update_tranco` function to update the Tranco list manually.

```sql
D SELECT update_tranco(true);
┌─────────────────────────────────────┐
│ update_tranco(CAST('f' AS BOOLEAN)) │
│               varchar               │
├─────────────────────────────────────┤
│ Tranco list updated                 │
└─────────────────────────────────────┘
```

This function will get the latest Tranco list and save it into the `tranco_list` table. There will be a `tranco_list_%Y-%m-%d.csv` file in the current directory after the function is called. The extension will use this file to prevent downloading the list again.

You can ignore the file and force the extension to download the list again by calling the function with `true` as a parameter. If you don't want to download the list again, you can call the function with `false` as a parameter.

```sql
D SELECT update_tranco(false);
```

As the latest Tranco list is for the last day, you can download your list manually and rename it to `tranco_list_%Y-%m-%d.csv` to use it with the extension too.

#### Get Tranco Ranking

You can use this function to get the ranking of a domain:

```sql
D SELECT get_tranco_rank('microsoft.com') AS rank;
┌─────────┐
│  rank   │
│ varchar │
├─────────┤
│ 2       │
└─────────┘

D SELECT get_tranco_rank('cloudflare.com') AS rank;
┌─────────┐
│  rank   │
│ varchar │
├─────────┤
│ 13      │
└─────────┘
```

You can use the `get_tranco_rank_category` function to retrieve the category utility column that gives you the domain's rank category. The `category` value is on a log10 scale with half steps (e.g., top 1k, top 5k, top 10k, top 50k, top 100k, top 500k, top 1M, top 5m, etc.), with each rank excluding the previous (e.g., top 5k is actually 4k domains, excluding top 1k).

```sql
D SELECT get_tranco_rank_category('microsoft.com') AS category;
┌──────────┐
│ category │
│ varchar  │
├──────────┤
│ top1k    │
└──────────┘
```

#### Tranco List

The `tranco_list` table function exposes the cached Tranco list as `rank`, `domain`, and `category` columns, so you can join it against your own data instead of calling `get_tranco_rank` row by row. Run `update_tranco` first to populate the cache.

```sql
D SELECT * FROM tranco_list() LIMIT 3;
┌───────┬───────────────┬──────────┐
│ rank  │    domain     │ category │
│ int32 │    varchar    │ varchar  │
├───────┼───────────────┼──────────┤
│     1 │ google.com    │ top1k    │
│     2 │ microsoft.com │ top1k    │
│     3 │ mail.ru       │ top1k    │
└───────┴───────────────┴──────────┘

D SELECT l.url, t.rank, t.category
  FROM logs l
  LEFT JOIN tranco_list() t ON t.domain = extract_domain(l.url);
```

### IP Address Functions

This extension provides various functions for manipulating and analyzing IP addresses, including calculating networks, hosts, and subnet masks.

#### IP Calculator

> [!WARNING]
> It's an experimental function.

The `ipcalc` function takes an IP address and netmask and calculates the resulting broadcast, network, wildcard mask, and host range.

![ipcalc-sc](./.github/ipcalc-sc.png)

```sql
SELECT * FROM ipcalc('192.168.1.0/24');
```

It's a table function that provides various details about IP addresses, including:

- Address
- Netmask
- Wildcard
- Network / Hostroute
- HostMin
- HostMax
- Broadcast
- Hosts count

You can use this table function with your data easily:

```sql
D CREATE OR REPLACE TABLE ips AS SELECT '127.0.0.1' AS ip UNION ALL SELECT '192.168.1.0/22';

D SELECT i.IP,
      (
          SELECT hostsPerNet
          FROM ipcalc(i.IP)
      ) AS hosts
  FROM ips AS i;
┌────────────────┬───────┐
│       ip       │ hosts │
│    varchar     │ int64 │
├────────────────┼───────┤
│ 127.0.0.1      │   254 │
│ 192.168.1.0/22 │  1022 │
└────────────────┴───────┘
```

#### Validate IP Address

The `is_valid_ip` function checks whether a string is a valid IPv4 or IPv6 address. Returns a `BOOLEAN`.

```sql
D SELECT is_valid_ip('192.168.1.1');
┌────────────────────────────┐
│ is_valid_ip('192.168.1.1') │
│          boolean           │
├────────────────────────────┤
│ true                       │
└────────────────────────────┘

D SELECT is_valid_ip('2001:db8::1');
┌────────────────────────────┐
│ is_valid_ip('2001:db8::1') │
│          boolean           │
├────────────────────────────┤
│ true                       │
└────────────────────────────┘

D SELECT is_valid_ip('not-an-ip');
┌──────────────────────────┐
│ is_valid_ip('not-an-ip') │
│         boolean          │
├──────────────────────────┤
│ false                    │
└──────────────────────────┘
```

#### Check Private IP

The `is_private_ip` function checks whether an IP address belongs to a private or reserved range. Supports both IPv4 and IPv6. Returns `NULL` for invalid addresses.

IPv4 ranges covered:

- RFC 1918 (10/8, 172.16/12, 192.168/16)
- loopback (127/8)
- link-local (169.254/16)
- carrier-grade NAT (100.64/10)
- documentation (TEST-NET)
- benchmarking (198.18/15)
- multicast (224/4)
- reserved (240/4)

IPv6 ranges covered:

- loopback (::1)
- unspecified (::)
- link-local (fe80::/10)
- ULA (fc00::/7)
- multicast (ff00::/8)
- documentation (2001:db8::/32)
- discard (100::/64)

```sql
D SELECT is_private_ip('192.168.1.1');
┌──────────────────────────────┐
│ is_private_ip('192.168.1.1') │
│           boolean            │
├──────────────────────────────┤
│ true                         │
└──────────────────────────────┘

D SELECT is_private_ip('8.8.8.8');
┌──────────────────────────┐
│ is_private_ip('8.8.8.8') │
│         boolean          │
├──────────────────────────┤
│ false                    │
└──────────────────────────┘

D SELECT is_private_ip('fe80::c028:8eff:fe34:6e5f');
┌────────────────────────────────────────────┐
│ is_private_ip('fe80::c028:8eff:fe34:6e5f') │
│                  boolean                   │
├────────────────────────────────────────────┤
│ true                                       │
└────────────────────────────────────────────┘
```

#### IP Version

The `ip_version` function returns `4` for IPv4, `6` for IPv6, or `NULL` for invalid addresses.

```sql
D SELECT ip_version('192.168.1.1');
┌───────────────────────────┐
│ ip_version('192.168.1.1') │
│           int8            │
├───────────────────────────┤
│             4             │
└───────────────────────────┘

D SELECT ip_version('::1');
┌───────────────────┐
│ ip_version('::1') │
│       int8        │
├───────────────────┤
│         6         │
└───────────────────┘
```

#### IP to Integer / Integer to IP

The `ip_to_int` function converts an IPv4 address to its 32-bit unsigned integer representation. The `int_to_ip` function converts back. Returns `NULL` for invalid or IPv6 input (IPv6 requires 128-bit support).

```sql
D SELECT ip_to_int('192.168.1.1');
┌──────────────────────────┐
│ ip_to_int('192.168.1.1') │
│          uint64          │
├──────────────────────────┤
│        3232235777        │
│      (3.23 billion)      │
└──────────────────────────┘

D SELECT int_to_ip(3232235777::UBIGINT);
┌────────────────────────────────────────┐
│ int_to_ip(CAST(3232235777 AS UBIGINT)) │
│                varchar                 │
├────────────────────────────────────────┤
│ 192.168.1.1                            │
└────────────────────────────────────────┘

D SELECT int_to_ip('3232235777');
┌─────────────────────────┐
│ int_to_ip('3232235777') │
│         varchar         │
├─────────────────────────┤
│ 192.168.1.1             │
└─────────────────────────┘
```

These functions are useful for sorting IPs numerically or performing range comparisons.

**Sort IPs numerically** instead of lexicographically:

```sql
D SELECT ip FROM my_ips ORDER BY ip_to_int(ip);
┌─────────────┐
│     ip      │
│   varchar   │
├─────────────┤
│ 8.8.8.8     │
│ 10.0.0.1    │
│ 192.168.1.1 │
└─────────────┘
```

**Range queries** using integer comparison:

```sql
D SELECT ip
  FROM my_ips
  WHERE ip_to_int(ip) BETWEEN ip_to_int('10.0.0.0') AND ip_to_int('10.255.255.255');
┌──────────┐
│    ip    │
│ varchar  │
├──────────┤
│ 10.0.0.1 │
└──────────┘
```

#### IP in Range

The `ip_in_range` function checks whether an IP address falls within a given CIDR block. Supports both IPv4 and IPv6. A CIDR without a prefix length is treated as a single host. An IPv4 address never matches an IPv6 block (and vice versa). Returns `NULL` for invalid IPs or malformed CIDRs.

```sql
D SELECT ip_in_range('192.168.1.100', '192.168.1.0/24');
┌────────────────────────────────────────────────┐
│ ip_in_range('192.168.1.100', '192.168.1.0/24') │
│                    boolean                     │
├────────────────────────────────────────────────┤
│ true                                           │
└────────────────────────────────────────────────┘

D SELECT ip_in_range('10.0.0.1', '192.168.1.0/24');
┌───────────────────────────────────────────┐
│ ip_in_range('10.0.0.1', '192.168.1.0/24') │
│                  boolean                  │
├───────────────────────────────────────────┤
│ false                                     │
└───────────────────────────────────────────┘

D SELECT ip_in_range('2001:db8::1', '2001:db8::/32');
┌─────────────────────────────────────────────┐
│ ip_in_range('2001:db8::1', '2001:db8::/32') │
│                   boolean                   │
├─────────────────────────────────────────────┤
│ true                                        │
└─────────────────────────────────────────────┘
```

#### IP to PTR

The `ip_to_ptr` function builds the reverse DNS name for an IP address: `in-addr.arpa` for IPv4 and nibble-reversed `ip6.arpa` for IPv6. The name has no trailing dot. Returns `NULL` for invalid input.

```sql
D SELECT ip_to_ptr('192.168.1.1');
┌──────────────────────────┐
│ ip_to_ptr('192.168.1.1') │
│         varchar          │
├──────────────────────────┤
│ 1.1.168.192.in-addr.arpa │
└──────────────────────────┘

D SELECT ip_to_ptr('2001:db8::1');
┌──────────────────────────────────────────────────────────────────────────┐
│                         ip_to_ptr('2001:db8::1')                         │
│                                 varchar                                  │
├──────────────────────────────────────────────────────────────────────────┤
│ 1.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.8.b.d.0.1.0.0.2.ip6.arpa │
└──────────────────────────────────────────────────────────────────────────┘
```

#### IPv6 Compress / Expand

The `ipv6_compress` function formats an IPv6 address in its shortest RFC 5952 canonical form (lowercase, leading zeros stripped, longest zero run replaced with `::`). IPv4-mapped addresses are written as `::ffff:a.b.c.d`. The `ipv6_expand` function returns the full form with eight zero-padded groups. Both return `NULL` for invalid or IPv4 input.

The `is_ipv4_mapped` function returns `true` if the address is an IPv4-mapped IPv6 address (`::ffff:0:0/96`), `false` for other IPv6 or IPv4 addresses, and `NULL` for invalid input.

```sql
D SELECT ipv6_compress('2001:0db8:0000:0000:0000:0000:0000:0001');
┌──────────────────────────────────────────────────────────┐
│ ipv6_compress('2001:0db8:0000:0000:0000:0000:0000:0001') │
│                         varchar                          │
├──────────────────────────────────────────────────────────┤
│ 2001:db8::1                                              │
└──────────────────────────────────────────────────────────┘

D SELECT ipv6_expand('2001:db8::1');
┌─────────────────────────────────────────┐
│       ipv6_expand('2001:db8::1')        │
│                 varchar                 │
├─────────────────────────────────────────┤
│ 2001:0db8:0000:0000:0000:0000:0000:0001 │
└─────────────────────────────────────────┘

D SELECT is_ipv4_mapped('::ffff:192.168.1.1');
┌──────────────────────────────────────┐
│ is_ipv4_mapped('::ffff:192.168.1.1') │
│               boolean                │
├──────────────────────────────────────┤
│ true                                 │
└──────────────────────────────────────┘
```

#### IP Type / Bogon

The `ip_type` function classifies an IPv4 or IPv6 address as one of `public`, `private`, `loopback`, `link_local`, `multicast`, `cgnat`, `documentation`, or `reserved`. IPv4-mapped IPv6 addresses are classified by their embedded IPv4 address. Returns `NULL` for invalid input.

The `is_bogon` function returns `true` if the address is not globally routable (any type other than `public`), and `NULL` for invalid input.

```sql
D SELECT ip_type('100.64.0.1');
┌───────────────────────┐
│ ip_type('100.64.0.1') │
│        varchar        │
├───────────────────────┤
│ cgnat                 │
└───────────────────────┘

D SELECT is_bogon('203.0.113.1');
┌─────────────────────────┐
│ is_bogon('203.0.113.1') │
│         boolean         │
├─────────────────────────┤
│ true                    │
└─────────────────────────┘
```

#### IP Anonymize

The `ip_anonymize` function truncates an IP address for privacy by zeroing every bit after a prefix length: `/24` for IPv4 (the last octet) and `/48` for IPv6 by default. Pass `ip_anonymize(ip, ipv4_prefix, ipv6_prefix)` to choose how many bits to keep. IPv6 output is in RFC 5952 canonical form, and IPv4-mapped IPv6 addresses use the IPv4 prefix. Returns `NULL` for invalid input or out-of-range prefixes.

```sql
D SELECT ip_anonymize('192.168.1.123');
┌───────────────────────────────┐
│ ip_anonymize('192.168.1.123') │
│            varchar            │
├───────────────────────────────┤
│ 192.168.1.0                   │
└───────────────────────────────┘

D SELECT ip_anonymize('2001:db8:abcd:1234::1', 16, 32);
┌───────────────────────────────────────────────┐
│ ip_anonymize('2001:db8:abcd:1234::1', 16, 32) │
│                    varchar                    │
├───────────────────────────────────────────────┤
│ 2001:db8::                                    │
└───────────────────────────────────────────────┘
```

### Normalize URL

The `normalize_url` function canonicalizes a URL by applying RFC 3986 normalizations: scheme/host lowercasing, default port removal (80/443/21), trailing slash removal, dot segment resolution, query parameter sorting, fragment removal, and percent-encoding normalization.

```sql
D SELECT normalize_url('HTTP://WWW.EXAMPLE.COM:80/a/b/../c/?z=1&a=2#frag') AS url;
┌────────────────────────────────────┐
│                url                 │
│              varchar               │
├────────────────────────────────────┤
│ http://www.example.com/a/c?a=2&z=1 │
└────────────────────────────────────┘

D SELECT normalize_url('HTTPS://Example.Com:443/path/./to/../page?b=2&a=1#section') AS url;
┌───────────────────────────────────────┐
│                  url                  │
│                varchar                │
├───────────────────────────────────────┤
│ https://example.com/path/page?a=1&b=2 │
└───────────────────────────────────────┘

D SELECT normalize_url('http://example.com/%7Euser') AS url;
┌──────────────────────────┐
│           url            │
│         varchar          │
├──────────────────────────┤
│ http://example.com/~user │
└──────────────────────────┘
```

This is especially useful for deduplicating URLs that differ only in formatting:

```sql
D SELECT normalize_url(url) AS normalized,
  count(*) AS cnt
FROM urls
GROUP BY normalized
HAVING cnt > 1;
```

### Domain Depth

The `domain_depth` function returns the number of dot-separated levels in a domain. It extracts the host from a URL and counts the labels. Returns `0` for IP addresses and invalid input, `NULL` for `NULL`.

```sql
D SELECT domain_depth('example.com') AS depth;
┌───────┐
│ depth │
│ int32 │
├───────┤
│   2   │
└───────┘

D SELECT domain_depth('https://www.example.com/page') AS depth;
┌───────┐
│ depth │
│ int32 │
├───────┤
│   3   │
└───────┘

D SELECT domain_depth('http://a.b.c.example.co.uk/page') AS depth;
┌───────┐
│ depth │
│ int32 │
├───────┤
│   6   │
└───────┘
```

### Base64 Encode / Decode

The `base64_encode` function encodes a string into Base64 format. The `base64_decode` function decodes a Base64-encoded string back to its original form.

```sql
D SELECT base64_encode('Hello World') AS encoded;
┌──────────────────┐
│     encoded      │
│     varchar      │
├──────────────────┤
│ SGVsbG8gV29ybGQ= │
└──────────────────┘

D SELECT base64_decode('SGVsbG8gV29ybGQ=') AS decoded;
┌─────────────┐
│   decoded   │
│   varchar   │
├─────────────┤
│ Hello World │
└─────────────┘

D SELECT base64_decode(base64_encode('https://example.com')) AS roundtrip;
┌─────────────────────┐
│      roundtrip      │
│       varchar       │
├─────────────────────┤
│ https://example.com │
└─────────────────────┘
```

### Validate URL

The `is_valid_url` function checks whether a string is a well-formed URL. A valid URL must have a scheme (e.g., `http`, `https`, `ftp`), the `://` separator, and a non-empty host. Returns a `BOOLEAN`, `NULL` for `NULL` input.

```sql
D SELECT is_valid_url('https://example.com') AS valid;
┌─────────┐
│  valid  │
│ boolean │
├─────────┤
│ true    │
└─────────┘

D SELECT is_valid_url('example.com') AS valid;
┌─────────┐
│  valid  │
│ boolean │
├─────────┤
│ false   │
└─────────┘

D SELECT is_valid_url('https://[::1]:8080/path') AS valid;
┌─────────┐
│  valid  │
│ boolean │
├─────────┤
│ true    │
└─────────┘
```

### Validate Domain

The `is_valid_domain` function validates a domain name against RFC 1035 / RFC 1123 rules. Requires at least two labels, alphanumeric and hyphens only (no start/end with hyphen), max 63 chars per label, max 253 chars total, and a non-numeric TLD. Returns a `BOOLEAN`, `NULL` for `NULL` input.

```sql
D SELECT is_valid_domain('example.com') AS valid;
┌─────────┐
│  valid  │
│ boolean │
├─────────┤
│ true    │
└─────────┘

D SELECT is_valid_domain('sub.example.co.uk') AS valid;
┌─────────┐
│  valid  │
│ boolean │
├─────────┤
│ true    │
└─────────┘

D SELECT is_valid_domain('localhost') AS valid;
┌─────────┐
│  valid  │
│ boolean │
├─────────┤
│ false   │
└─────────┘
```

### Public Suffix / Known TLD

The `is_public_suffix` function returns `true` if the input is exactly a public suffix from the [Public Suffix List](https://publicsuffix.org/) — ICANN suffixes like `com` or `co.uk`, private suffixes like `github.io`, and wildcard / exception rules (`*.ck`, `!www.ck`). Registrable domains such as `example.co.uk` return `false`.

The `is_known_tld` function returns `true` if the input is a single label that is a top-level domain in the list (e.g. `com`, `uk`, `ck`).

Both functions are case-insensitive, accept a leading or trailing dot (`.com`, `co.uk.`), return a `BOOLEAN`, and return `NULL` for `NULL` input.

```sql
D SELECT is_public_suffix('co.uk') AS suffix, is_public_suffix('example.co.uk') AS registrable;
┌─────────┬─────────────┐
│ suffix  │ registrable │
│ boolean │   boolean   │
├─────────┼─────────────┤
│ true    │ false       │
└─────────┴─────────────┘

D SELECT is_public_suffix('github.io') AS private_suffix;
┌────────────────┐
│ private_suffix │
│    boolean     │
├────────────────┤
│ true           │
└────────────────┘

D SELECT is_known_tld('com') AS tld, is_known_tld('co.uk') AS multi_label, is_known_tld('notarealtld') AS unknown;
┌─────────┬─────────────┬─────────┐
│   tld   │ multi_label │ unknown │
│ boolean │   boolean   │ boolean │
├─────────┼─────────────┼─────────┤
│ true    │ false       │ false   │
└─────────┴─────────────┴─────────┘
```

### Extract Path Segments

The `extract_path_segments` table function splits a URL path into individual segment rows. Each row contains a 1-based `segment_index` and the `segment` string. Returns 0 rows for `NULL`, empty, or root-only paths.

```sql
D SELECT * FROM extract_path_segments('https://example.com/path/to/page?q=1');
┌───────────────┬─────────┐
│ segment_index │ segment │
│     int32     │ varchar │
├───────────────┼─────────┤
│             1 │ path    │
│             2 │ to      │
│             3 │ page    │
└───────────────┴─────────┘
```

Use with `LATERAL` to expand segments per row in a table:

```sql
D SELECT u.url,
      s.segment_index,
      s.segment
  FROM urls u,
      LATERAL extract_path_segments(u.url) s
  ORDER BY u.url,
      s.segment_index;
┌───────────────────────────┬───────────────┬─────────┐
│            url            │ segment_index │ segment │
│          varchar          │     int32     │ varchar │
├───────────────────────────┼───────────────┼─────────┤
│ https://example.com/a/b/c │             1 │ a       │
│ https://example.com/a/b/c │             2 │ b       │
│ https://example.com/a/b/c │             3 │ c       │
│ https://test.org/x/y      │             1 │ x       │
│ https://test.org/x/y      │             2 │ y       │
└───────────────────────────┴───────────────┴─────────┘
```

### Parse URI

The `parse_uri` function returns every URI component in a single `STRUCT` call: `scheme`, `host`, `port`, `path`, `query`, and `fragment`. Missing components are empty strings. `NULL` input returns `NULL`. Scheme and host are lowercased; path, query, and fragment keep their original case.

```sql
D SELECT parse_uri('https://example.com:8080/path?q=1#section') AS uri;
┌──────────────────────────────────────────────────────────────────────────────────────────────────────────┐
│                                                   uri                                                    │
│    struct(scheme varchar, host varchar, port varchar, path varchar, query varchar, fragment varchar)     │
├──────────────────────────────────────────────────────────────────────────────────────────────────────────┤
│ {'scheme': https, 'host': example.com, 'port': 8080, 'path': /path, 'query': 'q=1', 'fragment': section} │
└──────────────────────────────────────────────────────────────────────────────────────────────────────────┘
```

Access individual fields with dot notation:

```sql
D SELECT parse_uri('https://example.com:8080/path?q=1#section').scheme AS scheme;
┌─────────┐
│ scheme  │
│ varchar │
├─────────┤
│ https   │
└─────────┘

D SELECT uri.host, uri.port, uri.path, uri.query, uri.fragment
  FROM (SELECT parse_uri('https://User:Pass@WWW.Example.COM:8080/Path?Q=1#Frag') AS uri);
┌─────────────────┬─────────┬─────────┬─────────┬──────────┐
│      host       │  port   │  path   │  query  │ fragment │
│     varchar     │ varchar │ varchar │ varchar │ varchar  │
├─────────────────┼─────────┼─────────┼─────────┼──────────┤
│ www.example.com │ 8080    │ /Path   │ Q=1     │ Frag     │
└─────────────────┴─────────┴─────────┴─────────┴──────────┘
```

Opaque URIs such as `mailto:` and `tel:` put the remainder in `path`. Scheme-less values like `example.com/path` are still split into host and path.

```sql
D SELECT parse_uri('mailto:someone@example.com').path AS path;
┌─────────────────────┐
│        path         │
│       varchar       │
├─────────────────────┤
│ someone@example.com │
└─────────────────────┘
```

### URL Encode / Decode

The `url_encode` function percent-encodes a string per RFC 3986. Only unreserved characters (`A-Z`, `a-z`, `0-9`, `-`, `_`, `.`, `~`) are left as-is — everything else is encoded as `%XX` with uppercase hex digits.

The `url_decode` function decodes percent-encoded strings back to their original form. It also decodes `+` as a space (for `application/x-www-form-urlencoded` compatibility). Invalid percent sequences are passed through literally.

```sql
D SELECT url_encode('hello world') AS encoded;
┌───────────────┐
│    encoded    │
│    varchar    │
├───────────────┤
│ hello%20world │
└───────────────┘

D SELECT url_decode('hello%20world') AS decoded;
┌─────────────┐
│   decoded   │
│   varchar   │
├─────────────┤
│ hello world │
└─────────────┘

D SELECT url_encode('https://www.google.com/search?client=firefox-b-d&q=url+encode') AS encoded;
┌─────────────────────────────────────────────────────────────────────────────────┐
│                                     encoded                                     │
│                                     varchar                                     │
├─────────────────────────────────────────────────────────────────────────────────┤
│ https%3A%2F%2Fwww.google.com%2Fsearch%3Fclient%3Dfirefox-b-d%26q%3Durl%2Bencode │
└─────────────────────────────────────────────────────────────────────────────────┘

D SELECT url_decode(url_encode('café 🦆')) AS roundtrip;
┌───────────┐
│ roundtrip │
│  varchar  │
├───────────┤
│ café 🦆   │
└───────────┘
```

`url_decode` also decodes `+` as space:

```sql
D SELECT url_decode('hello+world') AS decoded;
┌─────────────┐
│   decoded   │
│   varchar   │
├─────────────┤
│ hello world │
└─────────────┘
```

### Defang / Refang

The `defang` function makes a URL, domain, email, or IP safe to share using the CyberChef "Defang URL" convention. It rewrites `http`/`https`/`ftp` schemes to `hxxp`/`hxxps`/`fxp` and brackets `://`, dots, `@`, port colons, and IPv6 colons. It is idempotent.

The `refang` function reverses this and also accepts common variants such as `(.)`, `{.}`, `[dot]`, `[at]`, and `hXXps://`.

```sql
D SELECT defang('https://malware.example.com/beacon') AS defanged;
┌──────────────────────────────────────────┐
│                 defanged                 │
│                 varchar                  │
├──────────────────────────────────────────┤
│ hxxps[://]malware[.]example[.]com/beacon │
└──────────────────────────────────────────┘

D SELECT defang('invoice@spam-domain.com') AS defanged;
┌─────────────────────────────┐
│          defanged           │
│           varchar           │
├─────────────────────────────┤
│ invoice[@]spam-domain[.]com │
└─────────────────────────────┘

D SELECT defang('http://192.168.1.100:8080/beacon') AS defanged;
┌────────────────────────────────────────────┐
│                  defanged                  │
│                  varchar                   │
├────────────────────────────────────────────┤
│ hxxp[://]192[.]168[.]1[.]100[:]8080/beacon │
└────────────────────────────────────────────┘

D SELECT refang('hxxps[://]malware[.]example[.]com/beacon') AS refanged;
┌────────────────────────────────────┐
│              refanged              │
│              varchar               │
├────────────────────────────────────┤
│ https://malware.example.com/beacon │
└────────────────────────────────────┘
```

### URL to SURT / SURT to URL

The `url_to_surt` function converts a URL into a SURT (Sort-friendly URI Reordering Transform) key, the canonical form used by web archives in CDX indexes. It drops the scheme, userinfo, fragment, `www.` prefix, and default ports, reverses the host labels with commas, lowercases the path, and sorts query parameters.

The `surt_to_url` function converts a SURT key back into a URL. Since SURT keys don't store the scheme, `http://` is used unless the SURT is in the Heritrix form with a scheme (`https://(com,example,)/`).

```sql
D SELECT url_to_surt('https://www.Example.com/Path/?b=2&a=1#frag') AS surt;
┌───────────────────────────┐
│           surt            │
│          varchar          │
├───────────────────────────┤
│ com,example)/path?a=1&b=2 │
└───────────────────────────┘

D SELECT surt_to_url('com,example)/path?a=1&b=2') AS url;
┌─────────────────────────────────┐
│               url               │
│             varchar             │
├─────────────────────────────────┤
│ http://example.com/path?a=1&b=2 │
└─────────────────────────────────┘
```

### Extract Indicators From Text

The `extract_urls`, `extract_domains`, and `extract_ips` functions pull indicators out of free text such as logs, emails, or tickets. Each returns a `VARCHAR[]` list in order of appearance (duplicates included), an empty list when nothing is found, and `NULL` for `NULL` input.

- `extract_urls` finds `scheme://...` URLs and drops trailing punctuation and unbalanced closing brackets.
- `extract_domains` finds lowercased domain names whose TLD is in the Public Suffix List. It skips email local parts and tokens glued to `_` or non-ASCII characters.
- `extract_ips` finds valid IPv4 and IPv6 addresses, including bracketed, port-suffixed, and CIDR-suffixed forms.

```sql
D SELECT extract_urls('Visit https://example.com/login, or ftp://files.example.org.') AS urls;
┌──────────────────────────────────────────────────────────┐
│                           urls                           │
│                        varchar[]                         │
├──────────────────────────────────────────────────────────┤
│ ['https://example.com/login', 'ftp://files.example.org'] │
└──────────────────────────────────────────────────────────┘

D SELECT extract_domains('Mail from alerts@Example.COM about login.bad-site.net') AS domains;
┌───────────────────────────────────┐
│              domains              │
│             varchar[]             │
├───────────────────────────────────┤
│ [example.com, login.bad-site.net] │
└───────────────────────────────────┘

D SELECT extract_ips('Blocked 203.0.113.5:443 and [2001:db8::1]:8080') AS ips;
┌──────────────────────────────┐
│             ips              │
│          varchar[]           │
├──────────────────────────────┤
│ [203.0.113.5, '2001:db8::1'] │
└──────────────────────────────┘
```

Use `unnest` to get one row per indicator, `list_distinct` to deduplicate, and `refang` first to catch defanged indicators:

```sql
D SELECT ip, count(*) AS hits
  FROM (SELECT unnest(extract_ips(line)) AS ip FROM logs)
  GROUP BY ip ORDER BY hits DESC;
┌──────────────┬───────┐
│      ip      │ hits  │
│   varchar    │ int64 │
├──────────────┼───────┤
│ 198.51.100.7 │     2 │
│ 2001:db8::42 │     1 │
└──────────────┴───────┘
```

### Get Extension Version

You can use the `netquack_version` function to get the extension version.

```sql
D SELECT * FROM netquack_version();
┌─────────┐
│ version │
│ varchar │
├─────────┤
│ v1.15.0 │
└─────────┘
```

## Build Requirements

- **C++ compiler**: Needs C++17 or later (e.g., `g++`, `clang++`).
- **gperf**: Perfect hash generation requires `gperf`.
- **CMake** and **GNU Make**
- **Ninja** (recommended) and **ccache** (optional, speeds rebuilds)
- **vcpkg** for `libcurl` (see [`CONTRIBUTING.md`](CONTRIBUTING.md))

```bash
# On Debian-based systems
sudo apt-get install gperf cmake make ninja-build g++

# On macOS using Homebrew
brew install gperf cmake make ninja
```

See [`CONTRIBUTING.md`](CONTRIBUTING.md) for vcpkg setup, Make targets, and the development workflow. `make help` lists the common commands.

## Debugging

The debugging process for DuckDB extensions is not an easy job. For Netquack, we have created a log file in the current directory. The log file is named `netquack.log` and contains all the logs for the extension. You can use this file to debug your code.

Also, there will be stdout errors for background tasks like CURL.

## Roadmap 🗺️

- [ ] Implement `extract_custom_format` function
- [ ] Save Tranco data as Parquet
- [ ] Implement GeoIP functionality
- [ ] Return default value for `get_tranco_rank`
- [ ] Support internationalized domain names (IDNs)
- [ ] Implement `punycode_encode` / `punycode_decode` functions - Convert internationalized domain names to/from ASCII-compatible encoding
- [ ] Implement `extract_query_value` function - Return a single query parameter value as a scalar
- [ ] Implement `strip_tracking_params` / `remove_query_params` functions - Remove `utm_*`, `fbclid`, `gclid` and user-specified parameters
- [ ] Implement `resolve_url` function - Resolve a relative reference against a base URL (RFC 3986)
- [ ] Implement `extract_origin` / `is_same_origin` / `is_same_site` functions
- [ ] Implement `url_hierarchy` / `url_path_hierarchy` functions - Return the list of URL prefixes
- [ ] Implement `mime_type` function - Map a URL's file extension to its MIME type
- [ ] Support IPv6 in `ip_to_int` / `int_to_ip` (`UHUGEINT`)
- [ ] Implement CIDR functions - `cidr_contains`, `cidr_overlaps`, `cidr_range`, `range_to_cidrs`
- [ ] Implement `cidr_merge` aggregate function - Collapse a set of CIDRs into the minimal covering set
- [ ] Implement ASN lookup - `ip_to_asn` / `ip_to_as_org`
- [ ] Implement `domain_entropy` / `is_likely_dga` functions - Detect algorithmically generated domains
- [ ] Implement `generate_typosquats` table function - Generate typosquatting variants of a domain
- [ ] Implement `domain_skeleton` / `is_homograph` functions - Detect Unicode confusable domains
- [ ] Support other ranking lists (Cloudflare Radar, Cisco Umbrella, Majestic)
- [ ] Implement email functions - `extract_email_domain`, `is_valid_email`, `normalize_email`, `is_disposable_email_domain`
- [ ] Implement `read_access_log` table function - Parse Apache/Nginx access logs
- [ ] Implement `port_service` / `default_port` functions - Map ports to IANA service names and schemes to default ports
- [ ] Implement MAC address functions - `is_valid_mac`, `normalize_mac`, `mac_vendor`
- [ ] Support interop with DuckDB's `INET` type
- [ ] Add `LIST` overloads (e.g. `ip_in_range(ip, ['10.0.0.0/8', ...])`)
- [ ] Add a benchmarks page to the documentation

## Contributing 🤝

Contributions are welcome. The full workflow (build, tests, new functions, PR checklist) is in [`CONTRIBUTING.md`](CONTRIBUTING.md). Architecture and coding conventions live in [`AGENTS.md`](AGENTS.md).

```bash
git clone --recurse-submodules git@github.com:hatamiarash7/duckdb-netquack.git
cd duckdb-netquack
make help
GEN=ninja make
GEN=ninja make test
```

Please follow the [Code of Conduct](CODE_OF_CONDUCT.md). Security issues go through [SECURITY.md](SECURITY.md), not public issues.

## Issues 🐛

Use the [issue templates](https://github.com/hatamiarash7/duckdb-netquack/issues/new/choose) and include Netquack and DuckDB versions.
