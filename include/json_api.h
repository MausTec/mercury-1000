#ifndef __json_api_h
#define __json_api_h

#include "cJSON.h"

#ifdef __cplusplus
extern "C" {
#endif

void json_api_process_obj(cJSON *root, cJSON *response);
const char* json_api_process_str(const char *str);
char* json_api_process(const char* str, size_t len);
void json_api_free_str(char* str);

#ifdef __cplusplus
}
#endif

#endif
