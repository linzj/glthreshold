#ifndef GLCOMMON_H
#define GLCOMMON_H
#include "log.h"
#include <GLES2/gl2.h>
/* report GL errors, if any, to stderr */
bool checkError(const char* functionName);

// check for empty context
#if defined(GL_NO_ADDITIONAL_CHECK)
static inline void checkContextNotNull(...)
{
}
#else
void checkContextNotNull(int line, const char* file);
#endif // GL_NO_ADDITIONAL_CHECK

#define CHECK_CONTEXT_NOT_NULL() checkContextNotNull(__LINE__, __FILE__)

#endif /* GLCOMMON_H */
