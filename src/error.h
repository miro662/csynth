#ifndef __ERROR_H
#define __ERROR_H

typedef enum {
    ERR_OK,
    ERR_WASAPI_INIT,
    ERR_WASAPI_AUDIO,
} ErrorType;

typedef struct {
    ErrorType etype;
    char *message;
} Error;

#define IS_OK(e) (e->etype == ERR_OK)

#endif
