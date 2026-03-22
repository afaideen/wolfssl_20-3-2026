---

# 🚀 **wolfSSL + CLion Session Summary (Today)**

## 🧩 1. Set up wolfSSL build in CLion

* Imported wolfSSL source into CLion
* Built using **CMake + MinGW**
* Verified successful compilation of `wolfssl` library
* Learned:

   * how CMake builds wolfSSL
   * how targets and linking work (`ws2_32`, etc.)

---

## 🌐 2. Created custom TLS client (`client_openweather`)

* Added new executable target:

  ```cmake
  add_executable(client_openweather ...)
  ```
* Implemented HTTPS client using:

    * `wolfSSL_connect`
    * socket API (WinSock)
* Successfully connected to:

    * `api.openweathermap.org:443`
* Verified:

    * TLS 1.2 handshake
    * Cipher: `TLS_AES_256_GCM_SHA384`
    * HTTP 200 response ✅

---

## 🔐 3. Debugged certificate validation (-188 error)

* Initial issue:

  ```text
  wolfSSL_connect failed, err=-188
  ```
* Root cause:

    * missing / incorrect CA certificate

### Fix:

* Identified correct chain:

    * USERTrust (root)
    * Sectigo (intermediate)
* Learned:

    * wolfSSL requires correct **trust anchor**

---

## 🧠 4. Implemented dual-root trust model

* Added:

    * `USERTrust RSA Certification Authority`
    * `AAA Certificate Services`
* Observed behavior:

    * ❌ USERTrust only → fail
    * ❌ AAA only → fail
    * ✅ USERTrust + AAA → success

👉 Key insight:

> wolfSSL may require **multiple trust anchors** for chain validation

---

## 📦 5. Combined certs into bundle

* Created:

  ```text
  root_bundle.pem
  ```
* Also embedded as:

  ```c
  root_ca_bundle_pem
  ```
* Switched to:

  ```c
  wolfSSL_CTX_load_verify_buffer(...)
  ```

👉 Cleaner + embedded-friendly design

---

## 🧱 6. Modularized code (good architecture)

Created:

```text
client_openweather.c   → main app
cert_ref.c             → certificate handling
cert_ref.h             → interface
```

Learned:

* separation of concerns
* symbol visibility (`static`, `extern`)
* linker error debugging

---

## ⚙️ 7. Fixed linker + build issues

Issues solved:

* `undefined reference to root_ca_bundle_pem`
* missing object in target
* incorrect CMake config

Fix:

```cmake
add_executable(client_openweather
    client_openweather.c
    cert_ref.c
)
```

---

## 🔄 8. Made CMake dynamic (auto source discovery)

Final solution:

```cmake
file(GLOB CLIENT_OPENWEATHER_SOURCES CONFIGURE_DEPENDS
    "examples/client_openweather/*.c"
)
```

Result:

* auto-detect new `.c` files
* no need to edit CMake again

---

## 🧪 9. External vs embedded cert loading

Explored both:

### Embedded (current)

```c
wolfSSL_CTX_load_verify_buffer(...)
```

### File-based

```c
wolfSSL_CTX_load_verify_locations(...)
```

Learned:

* desktop vs embedded tradeoffs

---

## 🔑 10. Secured API key using environment variable

Replaced hardcoded key with:

```c
getenv("OWM_API_KEY")
```

Configured in CLion:

```
OWM_API_KEY=xxxx
```

---

## 🧩 11. Migrated away from IDE/WIN config

Goal:

```text
Remove dependency on IDE/WIN/user_settings.h
```

Steps:

* copied `user_settings.h` locally
* updated include path
* removed IDE/WIN dependency

---

## ⚙️ 12. Activated `WOLFSSL_USER_SETTINGS`

* Added:

  ```cmake
  target_compile_definitions(client_openweather PRIVATE WOLFSSL_USER_SETTINGS)
  ```
* Verified with:

  ```c
  #warning "USING LOCAL user_settings.h"
  ```

Result:

* confirmed local config is used ✅

---

# 🧠 **Key Learnings (Important for your journey)**

### TLS / wolfSSL

* Certificate chain validation is critical
* Multiple root CAs may be required
* Error `-188` = trust issue, not network issue

### Build system

* CMake targets must include all source files
* `GLOB` must be used correctly
* include paths determine header resolution

### Architecture

* modular design (cert / tls / net separation)
* config ownership (`user_settings.h`)
* desktop vs embedded differences

---

# 🏁 Final State

You now have:

✔ Working TLS client (OpenWeather)
✔ Verified certificate chain
✔ Dual-root trust model
✔ Modular codebase
✔ Dynamic CMake build
✔ Local config (`user_settings.h`)
✔ Environment-based secrets

---