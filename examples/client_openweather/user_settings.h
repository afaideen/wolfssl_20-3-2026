#ifndef _WIN_USER_SETTINGS_H_
#define _WIN_USER_SETTINGS_H_

#ifndef _WIN32
    #error This user_settings.h header is only designed for Windows
#endif

/* -------------------------------------------------------
 * Minimal TLS 1.2 client config for api.openweathermap.org
 *
 * Target working path:
 *   TLS 1.2
 *   ECDHE-RSA-AES128-GCM-SHA256 preferred
 *   RSA certificate verification
 *   SNI
 *   Manual CA loading from buffer
 * ------------------------------------------------------- */

/* Local / custom settings mode */
#define USE_WOLFSSL_IO

/* ---- Core requirements ---- */
#define HAVE_ECC
#define HAVE_AESGCM
#define HAVE_SNI

/* RSA is required for the server cert chain */
#define WC_RSA_PSS
#define WC_RSA_BLINDING

/* ---- Disable unused / legacy features ---- */
#define NO_PSK
#define NO_RC4
#define NO_DSA
#define NO_MD4
#define NO_DES3
#define NO_OLD_TLS

/* ---- Trim optional features not needed for this app ---- */
#define NO_MD5
#define NO_SHA
#define NO_SHA512
#define NO_DH

/* ---- TLS 1.3-related / extra features intentionally omitted ---- */
/* no WOLFSSL_TLS13 */
/* no HAVE_HKDF */

/* ---- Compatibility extras intentionally omitted for smaller size ---- */
/* no OPENSSL_EXTRA */

/* ---- Not needed because CA is loaded from root_ca_bundle_pem in code ---- */
/* no WOLFSSL_SYS_CA_CERTS */

/* ---- Not needed ---- */
/* no DTLS */
/* no ALPN */
/* no OCSP */
/* no CRL */
/* no cert generation */
/* no session ticket features explicitly enabled */

#endif /* _WIN_USER_SETTINGS_H_ */