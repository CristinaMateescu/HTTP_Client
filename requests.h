#ifndef _REQUESTS_
#define _REQUESTS_

char *compute_get_request(char *host, const char *url, char *url_params, char* cookie, const char* auth);
char *compute_post_request(char *host, const char *url,const char* type, char *form_data, const char* cookies, const char* auth);

#endif
