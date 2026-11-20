
# gateway_abstract_layer

## Structure

| Path                                      | Purpose                                                                                                                             |
|-------------------------------------------|-------------------------------------------------------------------------------------------------------------------------------------|
| _build                                    | The [CMake build tree](https://cmake.org/cmake/help/latest/manual/cmake.1.html#introduction-to-cmake-buildsystems), can be deleted. |
| cmake                                     | Generated [CMake](https://cmake.org/) files. May be deleted if user.cmake has not been added                                        |
| .vscode                                   | See [VSCode](https://code.visualstudio.com/docs/getstarted/settings)                                                                |
| .vscode/settings.json                     | Workspace specific settings                                                                                                         |
| .vscode/gateway_abstract_layer.mplab.json | The MPLAB project file, should not be deleted                                                                                       |
| out                                       | Final build artifacts                                                                                                               |

## CANopen object dictionary

The static OD (`src/software_core/canopen/od/gateway_od.c/.h`) is generated from
`gateway_node.dcf` with lely `dcf2dev`. Always pass `--include-config`, otherwise
the OD is compiled without `config.h` and its structs do not match lely
(lely asserts in `rbtree_find()` at start-up):

```sh
OD=src/software_core/canopen/od
.venv-dcf/bin/dcf2dev --no-strings --include-config -o $OD/gateway_od.c $OD/gateway_node.dcf gateway_od
.venv-dcf/bin/dcf2dev --no-strings --include-config --header -o $OD/gateway_od.h $OD/gateway_node.dcf gateway_od
```
