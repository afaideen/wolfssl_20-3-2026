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

#define OWM_HOST "api.openweathermap.org"
#define OWM_PORT "443"

extern const char* root_ca_bundle_pem;

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

    const char* api_key = getenv("OWM_API_KEY");

    if (api_key == NULL) {
        printf("Error: OWM_API_KEY not set\n");
        return -1;
    }

    char request[512];

    int req_len = snprintf(request, sizeof(request),
        "GET /data/2.5/weather?lat=2.5148&lon=102.8158"
        "&appid=%s"
        "&units=metric HTTP/1.1\r\n"
        "Host: " OWM_HOST "\r\n"
        "User-Agent: wolfssl-openweather/1.0\r\n"
        "Accept: application/json\r\n"
        "Connection: close\r\n"
        "\r\n",
        api_key
    );
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

    printf("DUAL-ROOT BUFFER TEST\n");
    printf("Using USERTrust + AAA root trust anchors\n");

    if (wolfSSL_CTX_load_verify_buffer(ctx,
        (const unsigned char*)root_ca_bundle_pem,
        (long)strlen(root_ca_bundle_pem),
        WOLFSSL_FILETYPE_PEM) != WOLFSSL_SUCCESS) {
        printf("load root CA bundle failed\n");
        goto cleanup;
    }

    wolfSSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, verify_cb);

    ssl = wolfSSL_new(ctx);
    if (ssl == NULL) {
        printf("wolfSSL_new failed\n");
        goto cleanup;
    }

    sockfd = tcp_connect(OWM_HOST, OWM_PORT);
    if (sockfd < 0) {
        printf("tcp_connect failed\n");
        goto cleanup;
    }

    if (wolfSSL_set_fd(ssl, sockfd) != WOLFSSL_SUCCESS) {
        printf("wolfSSL_set_fd failed\n");
        goto cleanup;
    }

    if (wolfSSL_UseSNI(ssl, WOLFSSL_SNI_HOST_NAME,
        OWM_HOST, (word16)strlen(OWM_HOST)) != WOLFSSL_SUCCESS) {
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

    printf("TLS connected to %s:%s\n", OWM_HOST, OWM_PORT);
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