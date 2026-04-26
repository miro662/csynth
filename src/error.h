#ifndef __ERROR_H
#define __ERROR_H

typedef enum {
    /// There is no error
    ERR_OK,
    ERR_SDL_INIT,
    ERR_SDL_AUDIO,
} ErrorType;

typedef struct {
    /// Type
    ErrorType etype;

    /// Message with detailed error info. Can be null
    char *message;
} Error;

#define IS_OK(e) (e->etype == ERR_OK)

#endif
