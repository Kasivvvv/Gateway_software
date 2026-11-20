set -e

cd /Users/kasiawcislo/Documents/Studia/Magisterka/Gateway_software/Gateway_abstract_layer/gateway_abstract_layer/_build/gateway_abstract_layer/default
/opt/homebrew/bin/cmake --regenerate-during-build -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
