#!/bin/bash

# Loop 10 times
for i in {1..10}; do
    echo "Run #$i"
    make run-all

    # Check if the command was successful
    if [ $? -ne 0 ]; then
        echo "Error occurred during iteration #$i"
        exit 1
    fi
done

echo "All runs completed successfully."
