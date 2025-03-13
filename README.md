# DuckDB Netquack Extension

![DuckDB Badge](https://img.shields.io/badge/Built_With-DuckDB-fff100) ![GitHub License](https://img.shields.io/github/license/hatamiarash7/duckdb-netquack) ![GitHub Release](https://img.shields.io/github/v/release/hatamiarash7/duckdb-netquack)

This extension is designed to simplify working with domains, URIs, and web paths directly within your database queries. Whether you're extracting top-level domains (TLDs), parsing URI components, or analyzing web paths, Netquack provides a suite of intuitive functions to handle all your network tasks efficiently. Built for data engineers, analysts, and developers.

With Netquack, you can unlock deeper insights from your web-related datasets without the need for external tools or complex workflows.

## Installation 🚀

**netquack** is distributed as a [DuckDB Community Extension](https://duckdb.org/community_extensions/) and can be installed using SQL:

```sql
INSTALL netquack FROM community;
LOAD netquack;
```

If you previously installed the `netquack` extension, upgrade using the FORCE command

```sql
FORCE INSTALL netquack FROM community;
LOAD netquack;
```

## Usage Examples 📚

Once installed, the [macro functions](https://duckdb.org/community_extensions/extensions/netquack.html#added-functions) provided by the extension can be used just like built-in functions.

### Extracting The Main Domain

This function extracts the main domain from a URL. For this purpose, the extension will get all public suffixes from the [publicsuffix.org](https://publicsuffix.org/) list and extract the main domain from the URL.

The download process of the public suffix list is done automatically when the function is called for the first time. After that, the list is stored in the `public_suffix_list` table to avoid downloading it again.

```sql
D SELECT extract_domain('a.example.com') as domain;
┌─────────────┐
│   domain    │
│   varchar   │
├─────────────┤
│ example.com │
└─────────────┘

D SELECT extract_domain('https://b.a.example.com/path') as domain;
┌─────────────┐
│   domain    │
│   varchar   │
├─────────────┤
│ example.com │
└─────────────┘
```

You can use the `update_suffixes` function to update the public suffix list manually.

```sql
D SELECT update_suffixes();
┌───────────────────┐
│ update_suffixes() │
│      varchar      │
├───────────────────┤
│ updated           │
└───────────────────┘
```

### Extracting The Path

This function extracts the path from a URL.

```sql
D SELECT extract_path('https://b.a.example.com/path/path') as path;
┌────────────┐
│    path    │
│  varchar   │
├────────────┤
│ /path/path │
└────────────┘

D SELECT extract_path('example.com/path/path/image.png') as path;
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
D SELECT extract_host('https://b.a.example.com/path/path') as host;
┌─────────────────┐
│      host       │
│     varchar     │
├─────────────────┤
│ b.a.example.com │
└─────────────────┘

D SELECT extract_host('example.com:443/path/image.png') as host;
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
D SELECT extract_schema('https://b.a.example.com/path/path') as schema;
┌─────────┐
│ schema  │
│ varchar │
├─────────┤
│ https   │
└─────────┘

D SELECT extract_schema('mailto:someone@example.com') as schema;
┌─────────┐
│ schema  │
│ varchar │
├─────────┤
│ mailto  │
└─────────┘

D SELECT extract_schema('tel:+123456789') as schema;
┌─────────┐
│ schema  │
│ varchar │
├─────────┤
│ tel     │
└─────────┘
```

### Extracting The Query

This function extracts the query string from a URL.

```sql
D SELECT extract_query_string('example.com?key=value') as query;
┌───────────┐
│   query   │
│  varchar  │
├───────────┤
│ key=value │
└───────────┘

D SELECT extract_query_string('http://example.com.ac/path/?a=1&b=2&') as query;
┌──────────┐
│  query   │
│ varchar  │
├──────────┤
│ a=1&b=2& │
└──────────┘
```

### Extracting The TLD (Top-Level Domain)

This function extracts the top-level domain from a URL. This function will use the public suffix list to extract the TLD. Check the [Extracting The Main Domain](#extracting-the-main-domain) section for more information about the public suffix list.

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

### Extracting The Sub Domain

This function extracts the sub-domain from a URL. This function will use the public suffix list to extract the TLD. Check the [Extracting The Main Domain](#extracting-the-main-domain) section for more information about the public suffix list.

```sql
D SELECT extract_subdomain('http://a.b.example.com/path') as dns_record;
┌────────────┐
│ dns_record │
│  varchar   │
├────────────┤
│ a.b        │
└────────────┘

D SELECT extract_subdomain('test.example.com.ac') as dns_record;
┌────────────┐
│ dns_record │
│  varchar   │
├────────────┤
│ test       │
└────────────┘
```

### Get Tranco Rank

#### Update Tranco List

This function returns the [Tranco](https://tranco-list.eu/) rank of a domain. You have a `update_tranco` function to update the Tranco list manually.

```sql
D SELECT update_tranco(true);
┌─────────────────────────────────────┐
│ update_tranco(CAST('f' AS BOOLEAN)) │
│               varchar               │
├─────────────────────────────────────┤
│ Tranco list updated                 │
└─────────────────────────────────────┘
```

This function will get the latest Tranco list and save it into the `tranco_list` table. There will be a `tranco_lit_%Y-%m-%d.csv` file in the current directory after the function is called. The extension will use this file to prevent downloading the list again.

You can ignore the file and force the extension to download the list again by calling the function with `true` as a parameter. If you don't want to download the list again, you can call the function with `false` as a parameter.

```sql
D SELECT update_tranco(false);
```

As the latest Tranco list is for the last day, you can download your list manually and rename it to `tranco_lit_%Y-%m-%d.csv` to use it with the extension too.

#### Get Tranco Ranking

You can use this function to get the ranking of a domain:

```sql
D SELECT get_tranco_rank('microsoft.com') as rank;
┌─────────┐
│  rank   │
│ varchar │
├─────────┤
│ 2       │
└─────────┘

D SELECT get_tranco_rank('cloudflare.com') as rank;
┌─────────┐
│  rank   │
│ varchar │
├─────────┤
│ 13      │
└─────────┘
```

### Get Extension Version

You can use the `netquack_version` function to get the version of the extension.

```sql
D select * from netquack_version();
┌─────────┐
│ version │
│ varchar │
├─────────┤
│ v1.1.0  │
└─────────┘
```

## Roadmap 🗺️

- [ ] Create a `TableFunction` for `extract_query_parameters` that return each key-value pair as a row.
- [ ] Save Tranco data as Parquet
- [ ] Create Rank category for Tranco ( `top1k` , `top5k`, `top10k`, `top100k`, `top500k`, `top1m` )
- [ ] Implement GeoIP functionality
- [ ] Add new functions to work with IPs
- [ ] Return default value for `get_tranco_rank`

## Contributing 🤝

Don't be shy and reach out to us if you want to contribute 😉

1. Fork it!
2. Create your feature branch: `git checkout -b my-new-feature`
3. Commit your changes: `git commit -am 'Add some feature'`
4. Push to the branch: `git push origin my-new-feature`
5. Submit a pull request

## Issues 🐛

Each project may have many problems. Contributing to the better development of this project by reporting them. 👍
