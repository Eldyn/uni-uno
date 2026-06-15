# API Contract Modernization TODO

- [x] Extract contract constants to a single file that can be read by both frontend and backend for all validation.
- [x] Lock down frontend types: Map ClientActions to specific interfaces with expected values, use that on emit, emitAndWait, and all similar.
- [x] Lock down C++ Payloads: map json objects with NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE, get() that payload directly, and send a safe error if it fails instead of the many Get<T> and/or GetOr<T> we currently use.
- [x] TEST: install all necessary tools to test if the server + frontend still communicate as expected or if there are any errors.
