#ifndef util_h
#define util_h
  #include "dropboxUtil.h"
#endif

void ShowCerts(SSL* ssl);
void LoadCertificates(SSL_CTX* ctx, char* CertFile, char* KeyFile);
