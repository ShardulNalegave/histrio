
#ifndef HISTRIO_STATUS_H
#define HISTRIO_STATUS_H

#include "errno.h"

typedef int histrio_status_t;

#define HISTRIO_OK                          0

#define HISTRIO_ERR_INVALID_ARGUMENT        EINVAL
#define HISTRIO_ERR_OUT_OF_MEMORY           ENOMEM
#define HISTRIO_ERR_TIMED_OUT               ETIMEDOUT
#define HISTRIO_ERR_TRY_AGAIN               EAGAIN
#define HISTRIO_ERR_NOT_FOUND               ENOENT
#define HISTRIO_ERR_ALREADY_EXISTS          EEXIST
#define HISTRIO_ERR_INTERRUPTED             EINTR

#define HISTRIO_ERR_UNSUPPORTED             1001
#define HISTRIO_ERR_INTERNAL                1002
#define HISTRIO_ERR_FAILED_PRECONDITION     1003
#define HISTRIO_ERR_RESOURCE_EXHAUSTED      1004

typedef struct {
    histrio_status_t code;
    const char *message;
} histrio_result_t;

#define HISTRIO_FAIL(c, msg) ((histrio_result_t){ .code = (c), .message = (msg) })
#define HISTRIO_SUCCESS()    ((histrio_result_t){ .code = HISTRIO_OK, .message = NULL })

#endif
