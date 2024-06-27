#!/bin/bash
echo "running profiling"
# Source the environment file
# source /home/HardwareSecurity112/tools/HS_Final_Project/env.cshrc
make clean
make -j 3
# Define the target benchmarks
targets=(
    "./benchmark/c17.bench"
    "./benchmark/c432.bench"
    "./benchmark/c3540.bench"
)

# Function to extract information from sld output
parse_sld_output() {
    local output="$1"
    
    # Extract keys and gates
    keys=$(echo "$output" | grep -oP 'keys=\d+' | cut -d '=' -f 2)
    gates=$(echo "$output" | grep -oP 'gates=\d+' | cut -d '=' -f 2)

    # Extract key
    key=$(echo "$output" | grep -oP 'key=\K.*')

    # Extract cube_count
    cube_count=$(echo "$output" | grep -oP 'cube_count=\d+' | cut -d '=' -f 2)

    # Extract cpu_time
    cpu_time=$(echo "$output" | grep -oP 'cpu_time=\d+\.\d+' | cut -d '=' -f 2)

    # Output the extracted information
    echo "Keys: $keys"
    echo "Gates: $gates"
    echo "Key: $key"
    echo "Cube Count: $cube_count"
    echo "CPU Time: $cpu_time"
}

# Loop through each target and execute the required commands
for target in "${targets[@]}"; do
    # Generate the test.bench filename based on the target
    test_bench="test.bench"

    # Execute the logic_lock command
    ./logic_lock "$target" "$test_bench" > /dev/null

    # Capture the output of the sld command
    sld_output=$(sld "$test_bench" "$target")
    
    # echo $sld_output
    # echo -e "\n\n\n"
    # Parse and extract information from the sld output
    parse_sld_output "$sld_output"
    # echo -e "\n\n\n"
    
    # Run the lcmp command with the parsed key
    lcmp_output=$(lcmp "$target" "$test_bench" key="$key")

    # Check if the output contains "equivalent"
    if echo "$lcmp_output" | grep -q "equivalent"; then
        echo -e "\e[32mOutput is equivalent\e[0m"
    else
        # Print error message in red
        echo -e "\e[31mError: Output is not equivalent\e[0m"
    fi
    
    # Calculate profiling result
    alpha=1 # not for sure the alpha & beta
    beta=100000
    profiling_result=$(echo "scale=10; $alpha * $cube_count / ($gates * $keys) + $beta * $cpu_time / ($gates * $keys)" | bc -l)

    # Output the profiling result
    echo -e "\e[35mProfiling Result:$profiling_result\n \e[0m"
    
done
