/*
 * Copyright (c) 2001, Bit Farm, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* Modified slightly for Race into Space project. */

/**
 * Logging for C - implementation.
 *
 * See log4c.h for documentation.
 */
#include "logging.h"
#include "macros.h"
#include <stdlib.h>
#include <assert.h>

struct LogCategory _LOGV(LOG_ROOT_CAT) = {
    0, 0, 0,
    STRINGIFY(LOG_ROOT_CAT), LP_UNINITIALIZED, 0,
    NULL, 0
};

void _log_logEvent(struct LogCategory *category, struct LogEvent *ev, ...)
{
    struct LogCategory *cat = category;

    while (1) {
        struct LogAppender *appender = cat->appender;

        if (appender != NULL) {
            va_start(ev->ap, ev);
            appender->doAppend(appender, ev);
            va_end(ev->ap);
        }

        if (!cat->willLogToParent) {
            break;
        }

        cat = cat->parent;
    }
} // _log_logEvent

static const char *initCategory(struct LogCategory *category)
{
    if (category == &_LOGV(LOG_ROOT_CAT)) {
        if (category->thresholdPriority == LP_UNINITIALIZED) {
            category->thresholdPriority = LP_WARNING;
        }

        if (!category->appender) {
            category->appender = log_defaultLogAppender;
        }
    } else {
        log_setParent(category, category->parent);
    }

    return ""; /* applyControlString(category); */
}

/**
 * This gets called the first time a category is referenced and performs the
 * initialization.
 * Also resets threshold to inherited!
 */
int _log_initCat(int priority, struct LogCategory *category)
{

    initCategory(category);

    return priority >= category->thresholdPriority;
} // _log_initCat

void log_setParent(struct LogCategory *cat, struct LogCategory *parent)
{

    assert(parent != NULL);

    // unlink from current parent
    if (cat->thresholdPriority != LP_UNINITIALIZED) {
        struct LogCategory **cpp = &parent->firstChild;

        while (*cpp != cat && *cpp != NULL) {
            cpp = &(*cpp)->nextSibling;
        }

        assert(*cpp == cat);
        *cpp = cat->nextSibling;
    }

    // Set new parent
    cat->parent = parent;
    cat->nextSibling = parent->firstChild;
    parent->firstChild = cat;

    // Make sure parent is initialized
    initCategory(parent);

    // Reset priority
    if (cat->isThreshInherited) {
        cat->thresholdPriority = parent->thresholdPriority;
    }

} // log_setParent

static void setInheritedThresholds(struct LogCategory *cat)
{
    struct LogCategory *child = cat->firstChild;

    for (; child != NULL; child = child->nextSibling) {
        if (child->isThreshInherited) {
            child->thresholdPriority = cat->thresholdPriority;
            setInheritedThresholds(child);
        }
    }
}

void log_setThreshold(struct LogCategory *cat, int thresholdPriority)
{
    cat->thresholdPriority = thresholdPriority;
    cat->isThreshInherited = 0;
    setInheritedThresholds(cat);
}

void log_setAppender(struct LogCategory *cat, struct LogAppender *app)
{
    cat->appender = app;
}
