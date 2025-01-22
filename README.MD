# kgg-dec

Kugou `kgg` decryption tool.

## Build

Build in Windows with CMake + Visual Studio 2022

```cmd
cmake --preset vs -DCMAKE_BUILD_TYPE=Release
cmake --build --preset vs-release --config Release
copy /y README.MD .\\build\\vs2022\\
```

## Usage

1. Copy `infra.dll` from KuGou installation directory to where `kgg-dec.exe` is located.
2. Invoke exe with path to the directory containing kgg file.

    ```shell
    kgg-dec.exe .
    ```

### CLI Options

* `[--infra-dll </path/to/infra.dll>]`: Specify the path to `infra.dll`, defaults to `infra.dll`.
* `[--scan-all-file-ext <0|1>]`: Whether to scan all file extensions, defaults to `0` to scan only `kgg` files.
* `[--db </path/to/KGMusicV3.db>]`: Specify the path to `KGMusicV3.db`, defaults to `%AppData%/Kugou8/KGMusicV3.db`.

After specifying the parameters, you can specify any number of files or directories.