#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Put wolfSSL headers before Windows socket headers */
#ifndef WOLFSSL_USER_SETTINGS
#include <wolfssl/options.h>
#endif
#include <wolfssl/ssl.h>

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#define OWM_HOST "api.openweathermap.org"
#define OWM_PORT "443"
//#define CA_CERT_FILE "C:\\project\\wolfssl_20-3-2026\\certs\\ca-cert.pem"
//#define CA_CERT_FILE "C:\\project\\wolfssl_20-3-2026\\certs\\openweather_root\\openweather_chain.pem"
//#define USERROOT_CERT_FILE "C:\\project\\wolfssl_20-3-2026\\certs\\openweather_root\\openweather_root.crt"
/* Replace this PEM body with your exported USERTrust RSA root certificate */
static const char usertrust_root_ca_pem[] =
"-----BEGIN CERTIFICATE-----\n"
"MIIF3jCCA8agAwIBAgIQAf1tMPyjylGoG7xkDjUDLTANBgkqhkiG9w0BAQwFADCB\n"
"iDELMAkGA1UEBhMCVVMxEzARBgNVBAgTCk5ldyBKZXJzZXkxFDASBgNVBAcTC0pl\n"
"cnNleSBDaXR5MR4wHAYDVQQKExVUaGUgVVNFUlRSVVNUIE5ldHdvcmsxLjAsBgNV\n"
"BAMTJVVTRVJUcnVzdCBSU0EgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMTAw\n"
"MjAxMDAwMDAwWhcNMzgwMTE4MjM1OTU5WjCBiDELMAkGA1UEBhMCVVMxEzARBgNV\n"
"BAgTCk5ldyBKZXJzZXkxFDASBgNVBAcTC0plcnNleSBDaXR5MR4wHAYDVQQKExVU\n"
"aGUgVVNFUlRSVVNUIE5ldHdvcmsxLjAsBgNVBAMTJVVTRVJUcnVzdCBSU0EgQ2Vy\n"
"dGlmaWNhdGlvbiBBdXRob3JpdHkwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIK\n"
"AoICAQCAEmUXNg7D2wiz0KxXDXbtzSfTTK1Qg2HiqiBNCS1kCdzOiZ/MPans9s/B\n"
"3PHTsdZ7NygRK0faOca8Ohm0X6a9fZ2jY0K2dvKpOyuR+OJv0OwWIJAJPuLodMkY\n"
"tJHUYmTbf6MG8YgYapAiPLz+E/CHFHv25B+O1ORRxhFnRghRy4YUVD+8M/5+bJz/\n"
"Fp0YvVGONaanZshyZ9shZrHUm3gDwFA66Mzw3LyeTP6vBZY1H1dat//O+T23LLb2\n"
"VN3I5xI6Ta5MirdcmrS3ID3KfyI0rn47aGYBROcBTkZTmzNg95S+UzeQc0PzMsNT\n"
"79uq/nROacdrjGCT3sTHDN/hMq7MkztReJVni+49Vv4M0GkPGw/zJSZrM233bkf6\n"
"c0Plfg6lZrEpfDKEY1WJxA3Bk1QwGROs0303p+tdOmw1XNtB1xLaqUkL39iAigmT\n"
"Yo61Zs8liM2EuLE/pDkP2QKe6xJMlXzzawWpXhaDzLhn4ugTncxbgtNMs+1b/97l\n"
"c6wjOy0AvzVVdAlJ2ElYGn+SNuZRkg7zJn0cTRe8yexDJtC/QV9AqURE9JnnV4ee\n"
"UB9XVKg+/XRjL7FQZQnmWEIuQxpMtPAlR1n6BB6T1CZGSlCBst6+eLf8ZxXhyVeE\n"
"Hg9j1uliutZfVS7qXMYoCAQlObgOK6nyTJccBz8NUvXt7y+CDwIDAQABo0IwQDAd\n"
"BgNVHQ4EFgQUU3m/WqorSs9UgOHYm8Cd8rIDZsswDgYDVR0PAQH/BAQDAgEGMA8G\n"
"A1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQEMBQADggIBAFzUfA3P9wF9QZllDHPF\n"
"Up/L+M+ZBn8b2kMVn54CVVeWFPFSPCeHlCjtHzoBN6J2/FNQwISbxmtOuowhT6KO\n"
"VWKR82kV2LyI48SqC/3vqOlLVSoGIG1VeCkZ7l8wXEskEVX/JJpuXior7gtNn3/3\n"
"ATiUFJVDBwn7YKnuHKsSjKCaXqeYalltiz8I+8jRRa8YFWSQEg9zKC7F4iRO/Fjs\n"
"8PRF/iKz6y+O0tlFYQXBl2+odnKPi4w2r78NBc5xjeambx9spnFixdjQg3IM8WcR\n"
"iQycE0xyNN+81XHfqnHd4blsjDwSXWXavVcStkNr/+XeTWYRUc+ZruwXtuhxkYze\n"
"Sf7dNXGiFSeUHM9h4ya7b6NnJSFd5t0dCy5oGzuCr+yDZ4XUmFF0sbmZgIn/f3gZ\n"
"XHlKYC6SQK5MNyosycdiyA5d9zZbyuAlJQG03RoHnHcAP9Dc1ew91Pq7P8yF1m9/\n"
"qS3fuQL39ZeatTXaw2ewh0qpKJ4jjv9cJ2vhsE/zB+4ALtRZh8tSQZXq9EfX7mRB\n"
"VXyNWQKV3WKdwrnuWih0hKWbt5DHDAff9Yk2dDLWKMGwsAvgnEzDHNb842m1R0aB\n"
"L6KCq9NjRHDEjf8tM7qtj3u1cIiuPhnPQCjY/MiQu12ZIvVS5ljFH4gxQ+6IHdfG\n"
"jjxDah2nGN59PRbxYvnKkKj9\n"
"-----END CERTIFICATE-----\n"
;


static const char aaa_root_ca_pem[] =
"-----BEGIN CERTIFICATE-----\n"
"MIIEMjCCAxqgAwIBAgIBATANBgkqhkiG9w0BAQUFADB7MQswCQYDVQQGEwJHQjEb\n"
"MBkGA1UECAwSR3JlYXRlciBNYW5jaGVzdGVyMRAwDgYDVQQHDAdTYWxmb3JkMRow\n"
"GAYDVQQKDBFDb21vZG8gQ0EgTGltaXRlZDEhMB8GA1UEAwwYQUFBIENlcnRpZmlj\n"
"YXRlIFNlcnZpY2VzMB4XDTA0MDEwMTAwMDAwMFoXDTI4MTIzMTIzNTk1OVowezEL\n"
"MAkGA1UEBhMCR0IxGzAZBgNVBAgMEkdyZWF0ZXIgTWFuY2hlc3RlcjEQMA4GA1UE\n"
"BwwHU2FsZm9yZDEaMBgGA1UECgwRQ29tb2RvIENBIExpbWl0ZWQxITAfBgNVBAMM\n"
"GEFBQSBDZXJ0aWZpY2F0ZSBTZXJ2aWNlczCCASIwDQYJKoZIhvcNAQEBBQADggEP\n"
"ADCCAQoCggEBAL5AnfRu4ep2hxxNRUSOvkbIgwadwSr+GB+O5AL686tdUIoWMQua\n"
"BtDFcCLNSS1UY8y2bmhGC1Pqy0wkwLxyTurxFa70VJoSCsN6sjNg4tqJVfMiWPPe\n"
"3M/vg4aijJRPn2jymJBGhCfHdr/jzDUsi14HZGWCwEiwqJH5YZ92IFCokcdmtet4\n"
"YgNW8IoaE+oxox6gmf049vYnMlhvB/VruPsUK6+3qszWY19zjNoFmag4qMsXeDZR\n"
"rOme9Hg6jc8P2ULimAyrL58OAd7vn5lJ8S3frHRNG5i1R8XlKdH5kBjHYpy+g8cm\n"
"ez6KJcfA3Z3mNWgQIJ2P2N7Sw4ScDV7oL8kCAwEAAaOBwDCBvTAdBgNVHQ4EFgQU\n"
"oBEKIz6W8Qfs4q8p74Klf9AwpLQwDgYDVR0PAQH/BAQDAgEGMA8GA1UdEwEB/wQF\n"
"MAMBAf8wewYDVR0fBHQwcjA4oDagNIYyaHR0cDovL2NybC5jb21vZG9jYS5jb20v\n"
"QUFBQ2VydGlmaWNhdGVTZXJ2aWNlcy5jcmwwNqA0oDKGMGh0dHA6Ly9jcmwuY29t\n"
"b2RvLm5ldC9BQUFDZXJ0aWZpY2F0ZVNlcnZpY2VzLmNybDANBgkqhkiG9w0BAQUF\n"
"AAOCAQEACFb8AvCb6P+k+tZ7xkSAzk/ExfYAWMymtrwUSWgEdujm7l3sAg9g1o1Q\n"
"GE8mTgHj5rCl7r+8dFRBv/38ErjHT1r0iWAFf2C3BUrz9vHCv8S5dIa2LX1rzNLz\n"
"Rt0vxuBqw8M0Ayx9lt1awg6nCpnBBYurDC/zXDrPbDdVCYfeU0BsWO/8tqtlbgT2\n"
"G9w84FoVxp7Z8VlIMCFlA2zs6SFz7JsDoeA3raAVGI/6ugLOpyypEBMs1OUIJqsi\n"
"l2D4kF501KKaU73yqWjgom7C12yxow+ev+to51byrvLjKzg6CYG1a4XXvi3tPxq3\n"
"smPi9WIsgtRqAEFQ8TmDn5XpNpaYbg==\n"
"-----END CERTIFICATE-----\n"
;

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

    if (store->current_cert) {
        char subject[256];
        subject[0] = '\0';
        wolfSSL_X509_NAME_oneline(
            wolfSSL_X509_get_subject_name(store->current_cert),
            subject, sizeof(subject));
        printf("verify_cb subject: %s\n", subject);
    }

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

    // const char* request =
    //     "GET /data/2.5/weather?lat=2.5148&lon=102.8158"
    //     "&appid=de39ea676b5e2acb6c8d4bab078f07af"
    //     "&units=metric HTTP/1.1\r\n"
    //     "Host: " OWM_HOST "\r\n"
    //     "User-Agent: wolfssl-openweather/1.0\r\n"
    //     "Accept: application/json\r\n"
    //     "Connection: close\r\n"
    //     "\r\n";

    const char* api_key = getenv("OWM_API_KEY");

    if (api_key == NULL) {
        printf("Error: OWM_API_KEY not set\n");
        return -1;
    }

    char request[512];

    int req_len  = snprintf(request, sizeof(request),
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
    

    ctx = wolfSSL_CTX_new(wolfTLSv1_3_client_method());
    if (ctx == NULL) {
        printf("wolfSSL_CTX_new failed\n");
        goto cleanup;
    }
    wolfSSL_Debugging_ON();

    printf("DUAL-ROOT BUFFER TEST\n");
    printf("Using USERTrust + AAA root trust anchors\n");
 
    //if (wolfSSL_CTX_load_system_CA_certs(ctx) != WOLFSSL_SUCCESS) {
    //    printf("wolfSSL_CTX_load_system_CA_certs failed\n");
    //    goto cleanup;
    //}
  
    if (wolfSSL_CTX_load_verify_buffer(ctx,
        (const unsigned char*)usertrust_root_ca_pem,
        (long)strlen(usertrust_root_ca_pem),
        WOLFSSL_FILETYPE_PEM) != WOLFSSL_SUCCESS) {
        
        printf("load USERTrust self-signed root failed\n");
        goto cleanup;
    }
    if (wolfSSL_CTX_load_verify_buffer(ctx,
        (const unsigned char*)aaa_root_ca_pem,
        (long)strlen(aaa_root_ca_pem),
        WOLFSSL_FILETYPE_PEM) != WOLFSSL_SUCCESS) {
        printf("load AAA root failed\n");
        goto cleanup;
    }


    /* first smoke test only */
    //wolfSSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);
    //wolfSSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
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


    ret = wolfSSL_connect(ssl);
    if (ret != WOLFSSL_SUCCESS) {
        err = wolfSSL_get_error(ssl, ret);
        //printf("wolfSSL_connect failed, err=%d\n", err);
        printf("wolfSSL_connect failed, ret=%d err=%d\n", ret, err);
        goto cleanup;
    }

    printf("TLS connected to %s:%s\n", OWM_HOST, OWM_PORT);
    printf("Negotiated TLS version: %s\n", wolfSSL_get_version(ssl));
    printf("Cipher: %s\n", wolfSSL_CIPHER_get_name(wolfSSL_get_current_cipher(ssl)));

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
