# 🔐 TLS Client – OpenWeather (wolfSSL)

## Overview

This project demonstrates a **minimal-footprint TLS client** using wolfSSL to securely connect to:

https://api.openweathermap.org

### Key Features

- Embedded-style TLS (no OS trust store)
- In-memory CA certificates (PEM buffers)
- Full certificate chain verification
- TLS 1.2 (minimal configuration)
- Restricted cipher: ECDHE-RSA-AES128-GCM-SHA256
- Deterministic TLS configuration via user_settings.h
- HTTP GET + JSON response

---

## ⚙️ TLS Optimization (Minimal Footprint)

### Final TLS Profile

TLS Version: TLS 1.2  
Cipher: ECDHE-RSA-AES128-GCM-SHA256  
Key Exchange: ECDHE (ECC)  
Certificate: RSA (2048-bit)  
Hash: SHA-256  

### Key Decisions

Removed:
- TLS 1.3
- DTLS
- ALPN, OCSP, CRL
- OpenSSL compatibility layer
- Legacy algorithms (RC4, DES3, DSA, MD4, MD5, SHA1, SHA512)
- System CA dependency

Kept:
- RSA (certificate verification)
- ECC (ECDHE key exchange)
- AES-GCM
- SHA-256
- Manual CA loading

### Cipher Restriction

```c
wolfSSL_set_cipher_list(ssl, "ECDHE-RSA-AES128-GCM-SHA256");
```

---

## 🧩 Configuration Strategy

wolfSSL configuration is fully controlled via:

user_settings.h

### Key Design

```c
#define WOLFSSL_NO_OPTIONS_H
#define WOLFSSL_USER_SETTINGS
```

### Result

options.h (generated) → DISABLED  
user_settings.h       → ACTIVE CONFIG  

---

## 🧠 Certificate Strategy

Uses dual-root trust model:

- USERTrust RSA Certification Authority
- AAA Certificate Services

Handles cross-signed chains correctly.

---

## 🔗 TLS Chain

USERTrust Root → Sectigo Intermediate → Server

---

## ⚙️ TLS Setup

```c
wolfSSL_CTX_load_verify_buffer(ctx, root_ca_bundle_pem, strlen(root_ca_bundle_pem), WOLFSSL_FILETYPE_PEM);
wolfSSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, verify_cb);
```

---

## 🔍 Verification Output

verify_cb: preverify=1 err=0 depth=0  
TLSv1.2  
Cipher: TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256  
HTTP/1.1 200 OK  

---

## 🧪 Common Error

Error -188 (ASN_NO_SIGNER_E)

Cause:
- Missing or incorrect root CA

Fix:
- Use correct self-signed USERTrust + AAA roots

---

## 🧱 Final Architecture

Application  
↓  
wolfSSL (user_settings.h controlled)  
↓  
TLS 1.2 (ECDHE-RSA-AES128-GCM-SHA256)  
↓  
TCP Socket  
↓  
api.openweathermap.org  

---

## 🧩 Build (Visual Studio)

- Open in Visual Studio 2026
- Build: Ctrl + Shift + B
- Output: ./Debug/x64/client_openweather.exe

---

## 🧩 Embedded Notes (PIC32MZ)

- Convert PEM → DER
- Store in flash
- Use minimal trust anchors
- Avoid filesystem dependency
- Enable session resumption (optional)

---

## ✅ Status

- TLS handshake successful
- Certificate chain verified
- Cipher restricted to AES128-GCM-SHA256
- HTTP response received
- user_settings.h fully controls configuration

---

## 🚀 Key Takeaway

In embedded TLS, you must control trust anchors and features explicitly.

This project demonstrates:
- Minimal TLS configuration
- Deterministic build strategy
- Real-world certificate validation
- Production-grade embedded TLS design
