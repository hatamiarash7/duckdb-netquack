---
icon: trowel
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

# How to build

#### Clone repo <a href="#clone-repo" id="clone-repo"></a>

```
git clone --recurse-submodules git@github.com:hatamiarash7/duckdb-netquack.git
```

{% hint style="info" %}
`--recurse-submodules` is needed because Netquack has two submodules for DuckDB and its extension CI tools.
{% endhint %}

#### Managing dependencies <a href="#managing-dependencies" id="managing-dependencies"></a>

Currently, Netquack has only one dependency which is `curl`. And we use [VCPKG](https://vcpkg.io/en/getting-started) to manage this dependency. You can run the following commands to install and enable `VCPKG`.

```bash
cd <your-working-dir-not-the-plugin-repo>
git clone https://github.com/Microsoft/vcpkg.git
sh ./vcpkg/scripts/bootstrap.sh -disableMetrics
export VCPKG_TOOLCHAIN_PATH=`pwd`/vcpkg/scripts/buildsystems/vcpkg.cmake
```

#### Compile <a href="#compile" id="compile"></a>

Then we can use `make` to build the extension:

```bash
make
```

The BlockDuck extension binary will be:

```bash
./build/release/extension/netquack/netquack.duckdb_extension
```

{% hint style="warning" %}
The build process is long and resource-intensive. Use tools like [Ninja](https://ninja-build.org/) and [Ccache](https://ccache.dev/) and consider adequate swap space if you run out of RAM.

```bash
GEN=ninja make
```
{% endhint %}

#### Tests <a href="#tests" id="tests"></a>

In Netquack, there are some `sqllogictest`, scenarios to make sure all functions work as expected:

```bash
make test
```
