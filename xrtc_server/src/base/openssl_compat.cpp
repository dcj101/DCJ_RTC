#include <openssl/ssl.h>
#include <openssl/evp.h>

// OpenSSL 3.0 compatibility layer for legacy libraries (like librtcbase.a)
// linked against OpenSSL 1.1

extern "C" {

#undef SSL_get_peer_certificate
X509 *SSL_get_peer_certificate(const SSL *ssl) {
    return SSL_get1_peer_certificate(ssl);
}

#undef EVP_MD_size
int EVP_MD_size(const EVP_MD *md) {
    return EVP_MD_get_size(md);
}

#undef EVP_MD_type
int EVP_MD_type(const EVP_MD *md) {
    return EVP_MD_get_type(md);
}

}
