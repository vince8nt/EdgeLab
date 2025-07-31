# PowerShell script for testing load/save functionality
# Exit on any error
$ErrorActionPreference = "Stop"

# Get the directory where this script is located
$SCRIPT_DIR = Split-Path -Parent $MyInvocation.MyCommand.Path
$PROJECT_ROOT = Split-Path -Parent $SCRIPT_DIR
$BUILD_DIR = Join-Path $PROJECT_ROOT "build"
$TEMP_DIR = Join-Path $SCRIPT_DIR "load_save_temp"

Write-Host "Running load/save tests from: $SCRIPT_DIR"
Write-Host "Build directory: $BUILD_DIR"
Write-Host "Temp directory: $TEMP_DIR"

# Check if build directory exists
if (-not (Test-Path $BUILD_DIR)) {
    Write-Error "Build directory not found: $BUILD_DIR"
    Write-Host "Please build the project first"
    exit 1
}

# Check if test_load_save executable exists
$EXECUTABLE_PATH = Join-Path $BUILD_DIR "test_load_save.exe"
if (-not (Test-Path $EXECUTABLE_PATH)) {
    Write-Error "test_load_save.exe not found in build directory"
    Write-Host "Please build the project first"
    exit 1
}

# Check if temp directory already exists - if so, fail
if (Test-Path $TEMP_DIR) {
    Write-Error "Temp directory already exists: $TEMP_DIR"
    Write-Host "Please remove it manually to run the test"
    exit 1
}

# Create temp directory
Write-Host "Creating temp directory: $TEMP_DIR"
New-Item -ItemType Directory -Path $TEMP_DIR | Out-Null

# Define test configurations
$graph_types = @('undirected', 'directed')
$vertex_types = @('unweighted', 'weighted', 'unweighted', 'weighted')
$edge_types = @('unweighted', 'unweighted', 'weighted', 'weighted')
$el_files = @('temp.el', 'temp.vel', 'temp.wel', 'temp.vwel')
$elab_file = 'temp.elab'

# Counter for tests
$test_count = 0
$passed_count = 0

Write-Host "Starting load/save tests..."

try {
    # Test all combinations of graph type, vertex type, edge type, and save file
    foreach ($graph_type in $graph_types) {
        for ($i = 0; $i -lt $vertex_types.Length; $i++) {
            $vertex_type = $vertex_types[$i]
            $edge_type = $edge_types[$i]
            $el_file = $el_files[$i]
            
            $test_count++
            Write-Host "Test $test_count`: Graph=$graph_type, Vertex=$vertex_type, Edge=$edge_type"
            
            # Test with EL format
            Write-Host "  Testing EL format..."
            $el_path = Join-Path $TEMP_DIR $el_file
            $el_args = @(
                "--graph-type", $graph_type,
                "--vertex-type", $vertex_type,
                "--edge-type", $edge_type,
                "--scale", "8",
                "--degree", "8",
                "--gen-type", "erdos_renyi",
                "--save-file", $el_path
            )
            
            & $EXECUTABLE_PATH @el_args
            if ($LASTEXITCODE -eq 0) {
                Write-Host "  EL format passed"
                $passed_count++
            } else {
                Write-Host "  EL format failed"
                throw "EL format test failed"
            }
            
            # Test with ELAB format
            Write-Host "  Testing ELAB format..."
            $elab_path = Join-Path $TEMP_DIR $elab_file
            $elab_args = @(
                "--graph-type", $graph_type,
                "--vertex-type", $vertex_type,
                "--edge-type", $edge_type,
                "--scale", "8",
                "--degree", "8",
                "--gen-type", "erdos_renyi",
                "--save-file", $elab_path
            )
            
            & $EXECUTABLE_PATH @elab_args
            if ($LASTEXITCODE -eq 0) {
                Write-Host "  ELAB format passed"
                $passed_count++
            } else {
                Write-Host "  ELAB format failed"
                throw "ELAB format test failed"
            }
            
            Write-Host ""
        }
    }
}
finally {
    # Clean up temp directory
    Write-Host "Cleaning up..."
    if (Test-Path $TEMP_DIR) {
        Remove-Item -Recurse -Force $TEMP_DIR
        Write-Host "Removed temp directory: $TEMP_DIR"
    }
}

Write-Host "=========================================="
Write-Host "Load/Save Test Results:"
Write-Host "Total tests: $test_count"
Write-Host "Passed: $passed_count"
Write-Host "Failed: $($test_count - $passed_count)"
Write-Host "=========================================="

if ($passed_count -eq $test_count) {
    Write-Host "All load/save tests passed!"
    exit 0
} else {
    Write-Host "Some load/save tests failed!"
    exit 1
} 