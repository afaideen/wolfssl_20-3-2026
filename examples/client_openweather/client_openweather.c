#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Force wolfSSL to use local user_settings.h only */
#define WOLFSSL_NO_OPTIONS_H
#define WOLFSSL_USER_SETTINGS
/* Put wolfSSL headers before Windows socket headers */
#include <wolfssl/options.h>
#include <wolfssl/ssl.h>

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>

/* ================= CONFIG ================= */

// Select provider
#define OPENWEATHERMAP
// #define WEATHERAPI
/* ========================================== */

#if defined(OPENWEATHERMAP) && defined(WEATHERAPI)
    #error "Enable only one provider"
#elif !defined(OPENWEATHERMAP) && !defined(WEATHERAPI)
    #error "Enable one provider"
#endif



#if defined(OPENWEATHERMAP)
    #define API_NAME        "OpenWeatherMap"
    #define API_HOST        "api.openweathermap.org"
    #define API_PORT        "443"
    #define API_ENV_KEY     "OPENWEATHERMAP_KEY"

    // Select ONE mode only
    #define SINGLE_ROOT_FILE
    // #define DUAL_ROOT_BUNDLE_FILE
    // #define DUAL_ROOT_BUFFER

    // #define CERT_DIR "C:/project/wolfssl_20-3-2026/examples/client_openweather/certs/openweathermap/"
    #define CERT_DIR "examples/client_openweather/certs/openweathermap/"

    /* Root files */
    #if defined(DUAL_ROOT_BUFFER)
        /* Bundle output (for file-based dual mode) */
        #define BUNDLE_CA    CERT_DIR "bundle.pem"
    #else
        #define USERTRUST_CA CERT_DIR "usertrust.crt"
        #define AAA_CA       CERT_DIR "aaa.cer"
    #endif


    /* Buffer (for embedded mode) */
    extern const char* root_ca_bundle_pem;
#elif defined(WEATHERAPI)
    #define API_NAME        "WeatherAPI"
    #define API_HOST        "api.weatherapi.com"
    #define API_PORT        "443"
    #define API_ENV_KEY     "WEATHERAPI_KEY"

    #define CERT_DIR        "examples/client_openweather/certs/weatherapi/"
    #define ISRG_Root_X1_CA              CERT_DIR "ISRG_Root_X1.pem"
#endif



#if defined(DUAL_ROOT_BUNDLE_FILE)
    int append_file(const char* dst, const char* src)
    {
        FILE* fsrc = fopen(src, "rb");
        if (!fsrc) {
            printf("append_file: failed to open source: %s\n", src);
            return -1;
        }

        FILE* fdst = fopen(dst, "ab");
        if (!fdst) {
            printf("append_file: failed to open destination: %s\n", dst);
            fclose(fsrc);
            return -1;
        }

        char buf[1024];
        size_t n;
        while ((n = fread(buf, 1, sizeof(buf), fsrc)) > 0) {
            if (fwrite(buf, 1, n, fdst) != n) {
                printf("append_file: write failed to destination: %s\n", dst);
                fclose(fsrc);
                fclose(fdst);
                return -1;
            }
        }

        fclose(fsrc);
        fclose(fdst);
        return 0;
    }

    int build_ca_bundle(const char* bundle, const char* ca1, const char* ca2)
    {
        printf("Building CA bundle: %s\n", bundle);
        printf("  CA1: %s\n", ca1);
        printf("  CA2: %s\n", ca2);

        remove(bundle);

        if (append_file(bundle, ca1) != 0) {
            printf("Failed appending CA1\n");
            return -1;
        }

        if (append_file(bundle, ca2) != 0) {
            printf("Failed appending CA2\n");
            return -1;
        }

        return 0;
    }

#endif
static int tcp_connect(const char* host, const char* port)
{
    struct addrinfo hints;
    struct addrinfo* result = NULL;
    struct addrinfo* ptr = NULL;
    int sockfd = -1;
    int ret;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    ret = getaddrinfo(host, port, &hints, &result);
    if (ret != 0) {
        printf("getaddrinfo failed: %d\n", ret);
        return -1;
    }

    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
        sockfd = (int)socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (sockfd == -1) {
            continue;
        }

        if (connect(sockfd, ptr->ai_addr, (int)ptr->ai_addrlen) == 0) {
            break;
        }

        closesocket(sockfd);
        sockfd = -1;
    }

    freeaddrinfo(result);
    return sockfd;
}

static int verify_cb(int preverify, WOLFSSL_X509_STORE_CTX* store)
{
    int err = store->error;
    int depth = store->error_depth;

    printf("verify_cb: preverify=%d err=%d depth=%d\n", preverify, err, depth);

    return preverify;
}

int main(void)
{
    WSADATA wsaData;
    WOLFSSL_CTX* ctx = NULL;
    WOLFSSL* ssl = NULL;
    int sockfd = -1;
    int ret;
    int err;

    const char* api_key;

    char request[512];
    int req_len;
    api_key = getenv(API_ENV_KEY);
    if (api_key == NULL) {
        printf("Error: %s not set\n",API_ENV_KEY);
        return -1;
    }
#if defined(OPENWEATHERMAP)

    req_len = snprintf(request, sizeof(request),
        "GET /data/2.5/weather?lat=2.5148&lon=102.8158"
        "&appid=%s"
        "&units=metric HTTP/1.1\r\n"
        "Host: " API_HOST "\r\n"
        "User-Agent: wolfssl-openweather/1.0\r\n"
        "Accept: application/json\r\n"
        "Connection: close\r\n"
        "\r\n",
        api_key
    );
#elif defined(WEATHERAPI)

     req_len = snprintf(request, sizeof(request),
        "GET /v1/current.json?key=%s&q=segamat&aqi=no HTTP/1.1\r\n"
        "Host: " API_HOST "\r\n"
        "User-Agent: wolfssl-weatherapi/1.0\r\n"
        "Accept: application/json\r\n"
        "Connection: close\r\n"
        "\r\n",
        api_key
    );
#endif

    if (req_len < 0 || req_len >= (int)sizeof(request)) {
        printf("Error: request buffer too small\n");
        return -1;
    }

    ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (ret != 0) {
        printf("WSAStartup failed: %d\n", ret);
        return 1;
    }

    wolfSSL_Init();

    /* TLS 1.2 only: matches your trimmed user_settings.h direction */
    ctx = wolfSSL_CTX_new(wolfTLSv1_2_client_method());
    if (ctx == NULL) {
        printf("wolfSSL_CTX_new failed\n");
        goto cleanup;
    }

    wolfSSL_Debugging_ON();





#if defined(OPENWEATHERMAP)

    #if defined(SINGLE_ROOT_FILE)
        printf("SINGLE-ROOT CA TEST\n");
        printf("Using AAA root trust anchor\n");

        // if (wolfSSL_CTX_load_verify_locations(ctx, USERTRUST_CA, 0) != WOLFSSL_SUCCESS) {
        if (wolfSSL_CTX_load_verify_locations(ctx, AAA_CA, 0) != WOLFSSL_SUCCESS) {
            printf("Failed to load CA bundle\n");
            goto cleanup;
        }
    #elif defined(DUAL_ROOT_BUNDLE_FILE)
        printf("DUAL-ROOT CA BUNDLE TEST\n");
        printf("Using USERTrust + AAA root trust anchors\n");

        if (build_ca_bundle(BUNDLE_CA, USERTRUST_CA, AAA_CA) != 0) {
            printf("Failed to build CA bundle\n");
            goto cleanup;
        }
        if (wolfSSL_CTX_load_verify_locations(ctx, BUNDLE_CA, 0) != WOLFSSL_SUCCESS) {
            printf("Failed to load CA bundle\n");
            goto cleanup;
        }
    #elif defined(DUAL_ROOT_BUFFER)
        printf("MODE: DUAL ROOT BUFFER\n");
        printf("Using embedded CA bundle\n");

        if (wolfSSL_CTX_load_verify_buffer(ctx,
            (const unsigned char*)root_ca_bundle_pem,
            (long)strlen(root_ca_bundle_pem),
            WOLFSSL_FILETYPE_PEM) != WOLFSSL_SUCCESS) {
            printf("load root CA bundle failed\n");
            goto cleanup;
        }
    #else
        #error "No CA mode selected"

    #endif

#elif defined(WEATHERAPI)
    if (wolfSSL_CTX_load_verify_locations(ctx,
    ISRG_Root_X1_CA,
    0) != WOLFSSL_SUCCESS) {
        printf("load root CA file failed\n");
        goto cleanup;
    }
#endif


    wolfSSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, verify_cb);

    ssl = wolfSSL_new(ctx);
    if (ssl == NULL) {
        printf("wolfSSL_new failed\n");
        goto cleanup;
    }

    sockfd = tcp_connect(API_HOST, API_PORT);
    if (sockfd < 0) {
        printf("tcp_connect failed\n");
        goto cleanup;
    }

    if (wolfSSL_set_fd(ssl, sockfd) != WOLFSSL_SUCCESS) {
        printf("wolfSSL_set_fd failed\n");
        goto cleanup;
    }

    if (wolfSSL_UseSNI(ssl, WOLFSSL_SNI_HOST_NAME,
        API_HOST, (word16)strlen(API_HOST)) != WOLFSSL_SUCCESS) {
        printf("wolfSSL_UseSNI failed\n");
        goto cleanup;
    }

    if (wolfSSL_set_cipher_list(ssl, "ECDHE-RSA-AES128-GCM-SHA256") != WOLFSSL_SUCCESS) {
        printf("wolfSSL_set_cipher_list failed\n");
        goto cleanup;
    }

    ret = wolfSSL_connect(ssl);
    if (ret != WOLFSSL_SUCCESS) {
        err = wolfSSL_get_error(ssl, ret);
        printf("wolfSSL_connect failed, ret=%d err=%d\n", ret, err);
        goto cleanup;
    }

    printf("TLS connected to %s:%s\n", API_HOST, API_PORT);
    printf("Negotiated TLS version: %s\n", wolfSSL_get_version(ssl));
    printf("Cipher: %s\n",
        wolfSSL_CIPHER_get_name(wolfSSL_get_current_cipher(ssl)));

    ret = wolfSSL_write(ssl, request, (int)strlen(request));
    if (ret <= 0) {
        err = wolfSSL_get_error(ssl, ret);
        printf("wolfSSL_write failed, err=%d\n", err);
        goto cleanup;
    }

    printf("=== HTTP RESPONSE BEGIN ===\n");

    for (;;) {
        char buf[2048];

        ret = wolfSSL_read(ssl, buf, sizeof(buf) - 1);
        if (ret > 0) {
            buf[ret] = '\0';
            printf("%s", buf);
        }
        else {
            err = wolfSSL_get_error(ssl, ret);

            if (err == WOLFSSL_ERROR_WANT_READ ||
                err == WOLFSSL_ERROR_WANT_WRITE) {
                continue;
            }
            break;
        }
    }

    printf("\n=== HTTP RESPONSE END ===\n");

cleanup:
    if (ssl != NULL) {
        wolfSSL_shutdown(ssl);
        wolfSSL_free(ssl);
    }

    if (sockfd >= 0) {
        closesocket(sockfd);
    }

    if (ctx != NULL) {
        wolfSSL_CTX_free(ctx);
    }

    wolfSSL_Cleanup();
    WSACleanup();

    return 0;
}