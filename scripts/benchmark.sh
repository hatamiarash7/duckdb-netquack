#!/usr/bin/env bash

echo "=== NetQuack Performance Comparison ==="
echo ""

# Change to the project root directory (parent of script location)
CWD="$(cd "$(dirname "$0")/.." && pwd)"
cd "$CWD"

# Create results directory in build (gitignored)
mkdir -p build/benchmark_results

# Function to create benchmark SQL with specified load command
create_benchmark_sql() {
    local load_command="$1"
    local version_label="$2"
    local output_file="$3"
    
    cat > "$output_file" << EOF
${load_command}

-- Create test data
CREATE TABLE benchmark_urls AS SELECT * FROM (VALUES
    ('https://www.example.com/path?query=value#fragment'),
    ('http://subdomain.example.co.uk:8080/long/path/to/file.html?param1=value1&param2=value2'),
    ('ftp://user:password@ftp.example.com.au:21/directory/file.zip'),
    ('https://blog.example.org/2023/12/article-title.html'),
    ('http://api.service.example.net:3000/v1/users/123?format=json'),
    ('https://cdn.assets.example.com/images/logo.png'),
    ('mailto:contact@example.com'),
    ('http://localhost:8080/debug'),
    ('https://secure.payment.example.gov:8443/transaction?id=abc123'),
    ('https://example.com')
) AS t(url);

-- Expand dataset
CREATE TABLE large_benchmark_urls AS 
WITH RECURSIVE series(i) AS (
    SELECT 1
    UNION ALL
    SELECT i + 1 FROM series WHERE i <= 3000
)
SELECT url FROM benchmark_urls CROSS JOIN series;

.timer on
.print '=== ${version_label} BENCHMARKS ==='

.print 'extract_schema:'
SELECT extract_schema(url) FROM large_benchmark_urls;

.print 'extract_host:'
SELECT extract_host(url) FROM large_benchmark_urls;

.print 'extract_port:'
SELECT extract_port(url) FROM large_benchmark_urls;

.print 'extract_path:'
SELECT extract_path(url) FROM large_benchmark_urls;

.print 'extract_query_string:'
SELECT extract_query_string(url) FROM large_benchmark_urls;

.print 'extract_domain:'
SELECT extract_domain(url) FROM large_benchmark_urls;

.print 'extract_subdomain:'
SELECT extract_subdomain(url) FROM large_benchmark_urls;

.print 'extract_tld:'
SELECT extract_tld(url) FROM large_benchmark_urls;

.print 'extract_extension:'
SELECT extract_extension(url) FROM large_benchmark_urls;
EOF
}

# Create temporary directory
mkdir -p build/tmp

# Create benchmark SQL files using the DRY function
create_benchmark_sql "FORCE INSTALL netquack FROM community; LOAD netquack;" "PUBLISHED VERSION" "build/tmp/published_benchmark.sql"
create_benchmark_sql "LOAD './build/release/extension/netquack/netquack.duckdb_extension';" "local VERSION" "build/tmp/local_benchmark.sql"

echo "Step 1: Installing and benchmarking PUBLISHED NetQuack extension..."
duckdb < build/tmp/published_benchmark.sql > build/benchmark_results/published_full_output.txt 2>&1

echo "Step 2: Running benchmarks on LOCAL implementation..."
./build/release/duckdb < build/tmp/local_benchmark.sql > build/benchmark_results/local_full_output.txt 2>&1

echo "Step 3: Generating comparison analysis..."

# Extract times and calculate improvements
echo "📊 RESULTS SUMMARY:" > build/benchmark_results/analysis.txt
echo "" >> build/benchmark_results/analysis.txt

echo "| Function | Published Time |   Local Time   | Improvement |" >> build/benchmark_results/analysis.txt
echo "|----------|----------------|----------------|-------------|" >> build/benchmark_results/analysis.txt

# Extract timing data using a more robust approach
functions=("extract_schema" "extract_host" "extract_port" "extract_path" "extract_query_string" "extract_domain" "extract_subdomain" "extract_tld" "extract_extension")

# Extract all times into temporary files
grep "Run Time" build/benchmark_results/published_full_output.txt | grep -o "real [0-9.]*" | cut -d' ' -f2 > build/tmp/published_times.txt
grep "Run Time" build/benchmark_results/local_full_output.txt | grep -o "real [0-9.]*" | cut -d' ' -f2 > build/tmp/local_times.txt

# Process each function
for i in {0..9}; do
    func=${functions[$i]}
    line_num=$((i + 1))
    
    published_time=$(sed -n "${line_num}p" build/tmp/published_times.txt)
    local_time=$(sed -n "${line_num}p" build/tmp/local_times.txt)
    
    if [ ! -z "$published_time" ] && [ ! -z "$local_time" ]; then
        improvement=$(echo "scale=1; $published_time / $local_time" | bc -l)
        echo "| ${func} | ${published_time}s | ${local_time}s | ${improvement}x faster |" >> build/benchmark_results/analysis.txt
    fi
done

# Clean up timing files
rm -f build/tmp/published_times.txt build/tmp/local_times.txt

echo ""
echo "✅ Benchmark comparison complete!"
echo ""
cat build/benchmark_results/analysis.txt
