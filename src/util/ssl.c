#include "dropboxUtil.h"

void ShowCerts(SSL* ssl) {
  X509 *cert;
  char *line;

  cert = SSL_get_peer_certificate(ssl); /* Get certificates (if available) */
  if (cert != NULL) {
    printf("Peer Certificate:\n");
    line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
    printf("Subject: %s\n", line);
    free(line);
    line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
    printf("Issuer: %s\n", line);
    free(line);
    X509_free(cert);
  } else {
    printf("No certificate was presented.\n");
  }
}

void LoadCertificates(SSL_CTX* ctx, char* CertFile, char* KeyFile) {
  /* set the local certificate from CertFile */
  if (SSL_CTX_use_certificate_file(ctx, CertFile, SSL_FILETYPE_PEM) <= 0) {
    ERR_print_errors_fp(stderr);
    abort();
  }

  /* set the private key from KeyFile (may be the same as CertFile) */
  if (SSL_CTX_use_PrivateKey_file(ctx, KeyFile, SSL_FILETYPE_PEM) <= 0) {
    ERR_print_errors_fp(stderr);
    abort();
  }

  /* verify private key */
  if (!SSL_CTX_check_private_key(ctx)) {
    fprintf(stderr, "Private key does not match the public certificate\n");
    abort();
  }
}
