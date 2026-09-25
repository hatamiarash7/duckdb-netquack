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

# Extract Indicators From Text

The `extract_urls`, `extract_domains`, and `extract_ips` functions pull indicators out of free text such as logs, emails, or tickets. Each one returns a `VARCHAR[]` list:

* Matches appear in the order they occur in the text, and duplicates are kept (wrap the result in `list_distinct` to remove them).
* Text with no matches returns an empty list `[]`.
* `NULL` input returns `NULL`.

## Extract URLs

`extract_urls` finds every `scheme://...` URL, with any scheme (`http`, `https`, `ftp`, `file`, `ssh+git`, ...). A URL ends at whitespace, a quote, a backtick, or an angle bracket. Trailing sentence punctuation (`.,;:!?`) and closing brackets with no matching opening bracket inside the URL are dropped, so `(see http://en.wikipedia.org/wiki/Foo_(bar))` yields `http://en.wikipedia.org/wiki/Foo_(bar)`.

```sql
D SELECT extract_urls('Visit https://example.com/login, or ftp://files.example.org.') AS urls;
┌──────────────────────────────────────────────────────────┐
│                           urls                           │
│                        varchar[]                         │
├──────────────────────────────────────────────────────────┤
│ ['https://example.com/login', 'ftp://files.example.org'] │
└──────────────────────────────────────────────────────────┘
```

Scheme-less links such as `www.example.com/path` are not URLs here. Use `extract_domains` to catch their hosts.

## Extract Domains

`extract_domains` finds every domain name that passes [`is_valid_domain`](is-valid-domain.md) and whose last label is a TLD in the Public Suffix List. Results are lowercased. It also finds hosts inside URLs and emails, but skips:

* Email local parts: `john.dev@corp.io` yields only `corp.io`.
* Tokens glued to `_` or non-ASCII characters, such as `_dmarc.example.com` or `müller.de`.
* IP addresses, version strings (`v1.2.3`), and names with unknown TLDs (`config.json`, `host.localdomain`).

```sql
D SELECT extract_domains('Mail from alerts@Example.COM about login.bad-site.net') AS domains;
┌───────────────────────────────────┐
│              domains              │
│             varchar[]             │
├───────────────────────────────────┤
│ [example.com, login.bad-site.net] │
└───────────────────────────────────┘
```

{% hint style="warning" %}
File names whose extension is also a TLD (for example `README.md`, `run.sh`, or `archive.zip`) are reported as domains. Filter them out afterwards if that matters for your data.
{% endhint %}

## Extract IPs

`extract_ips` finds every valid IPv4 and IPv6 address, using the same rules as `is_valid_ip`. It handles addresses in brackets (`[2001:db8::1]:8080`), with ports (`10.0.0.1:443`), with CIDR suffixes (`192.168.0.0/16`), with IPv6 zone IDs (`fe80::1%eth0`), and IPv4-mapped IPv6 addresses (`::ffff:192.0.2.1`).

It skips invalid or embedded candidates such as `999.1.1.1`, `1.2.3.4.5`, `v1.2.3.4`, and octets with leading zeros like `01.2.3.4`. Timestamps like `12:34:56` and C++ scopes like `std::vector` are not treated as IPv6.

```sql
D SELECT extract_ips('Blocked 203.0.113.5:443 and [2001:db8::1]:8080') AS ips;
┌──────────────────────────────┐
│             ips              │
│          varchar[]           │
├──────────────────────────────┤
│ [203.0.113.5, '2001:db8::1'] │
└──────────────────────────────┘
```

## Working With Tables

Use `unnest` to turn the lists into one row per indicator:

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

Threat reports often share defanged indicators such as `hxxps[://]evil[.]example[.]com`. Run the text through [`refang`](defang-functions.md) first to extract them:

```sql
D SELECT extract_urls(refang('beacon to hxxps[://]evil[.]example[.]com/c2')) AS urls;
┌─────────────────────────────────┐
│              urls               │
│            varchar[]            │
├─────────────────────────────────┤
│ ['https://evil.example.com/c2'] │
└─────────────────────────────────┘
```
