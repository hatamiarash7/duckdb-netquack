# DuckDB Netquack Extension

This extension designed to simplify working with domains, URIs, and web paths directly within your database queries. Whether you're extracting top-level domains (TLDs), parsing URI components, or analyzing web paths, Netquack provides a suite of intuitive functions to handle all your network tasks efficiently. Built for data engineers, analysts, and developers.
With Netquack, you can unlock deeper insights from your web-related datasets without the need for external tools or complex workflows."

## Installation

**netquack** is distributed as a [DuckDB Community Extension](https://github.com/duckdb/community-extensions) and can be installed using SQL:

```sql
INSTALL netquack FROM community;
LOAD netquack;
```

If you previously installed the `netquack` extension, upgrade using the FORCE command

```sql
FORCE INSTALL netquack FROM community;
LOAD netquack;
```

## Usage Examples

Once installed, the [macro functions](https://community-extensions.duckdb.org/extensions/netquack.html#added-functions) provided by the extension can be used just like built-in functions.

## Extracting The Main Domain

This function extracts the main domain from a URL. For this purpose, the extension will get all public suffixes from the [publicsuffix.org](https://publicsuffix.org/) list and extract the main domain from the URL.

The download process of the public suffix list is done automatically when the function called for the first time. After that, the list is stored in the `public_suffix_list` table to avoid downloading it again.

```sql
D SELECT extract_domain('a.domain.com') as domain;
┌────────────┐
│   domain   │
│  varchar   │
├────────────┤
│ domain.com │
└────────────┘

D SELECT extract_domain('https://b.a.domain.com/path') as domain;
┌────────────┐
│   domain   │
│  varchar   │
├────────────┤
│ domain.com │
└────────────┘
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

## Extracting The Path

This function extracts the path from a URL.

```sql
D SELECT extract_path('https://b.a.domain.com/path/path') as path;
┌────────────┐
│    path    │
│  varchar   │
├────────────┤
│ /path/path │
└────────────┘

D SELECT extract_path('domain.com/path/path/image.png') as path;
┌──────────────────────┐
│         path         │
│       varchar        │
├──────────────────────┤
│ /path/path/image.png │
└──────────────────────┘
```

## Extracting The TLD (Top-Level Domain)

This function extracts the top-level domain from a URL. This function will use the public suffix list to extract the TLD. Check the [Extracting The Main Domain](#extracting-the-main-domain) section for more information about the public suffix list.

```sql
D SELECT extract_tld('https://domain.com.ac/path/path') as tld;
┌─────────┐
│   tld   │
│ varchar │
├─────────┤
│ com.ac  │
└─────────┘

D SELECT extract_tld('a.domain.com') as tld;
┌─────────┐
│   tld   │
│ varchar │
├─────────┤
│ com     │
└─────────┘
```

## Contributing 🤝

Don't be shy and reach out to us if you want to contribute 😉

1. Fork it!
2. Create your feature branch: `git checkout -b my-new-feature`
3. Commit your changes: `git commit -am 'Add some feature'`
4. Push to the branch: `git push origin my-new-feature`
5. Submit a pull request

## Issues

Each project may have many problems. Contributing to the better development of this project by reporting them. 👍
