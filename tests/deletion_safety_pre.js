// Give the native C++ cleanup fixtures an isolated filesystem under Node/wasm.
Module.preRun = [function() {
    FS.mkdir('/deletion-safety');
    FS.mkdir('/deletion-safety/native');
}];
