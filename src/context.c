#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>

#include "config.h"
#include "common.h"
#include "msg.h"

#include "context.h"
#include "spirv.h"

/* OpenGL */
extern const struct ra_ctx_fns ra_ctx_android;

/* Vulkan */
extern const struct ra_ctx_fns ra_ctx_vulkan_xlib;
extern const struct ra_ctx_fns ra_ctx_vulkan_android;

static const struct ra_ctx_fns *contexts[] = {
// OpenGL contexts:
#if HAVE_EGL_ANDROID
    &ra_ctx_android,
#endif

// Vulkan contexts:
#if HAVE_VULKAN
#if HAVE_ANDROID
    &ra_ctx_vulkan_android,
#endif
#if HAVE_X11
    &ra_ctx_vulkan_xlib,
#endif
#endif
};

struct ra_ctx *ra_ctx_create(struct mpv_global *global, struct mp_log *parent, 
                             struct ra_ctx_opts opts)
{
    bool api_auto = !opts.context_type || strcmp(opts.context_type, "auto") == 0;
    bool ctx_auto = !opts.context_name || strcmp(opts.context_name, "auto") == 0;

    if (ctx_auto) {
        mp_verbose(parent, "Probing for best GPU context.\n");
        opts.probing = true;
    }

    for (int i = 0; i < MP_ARRAY_SIZE(contexts); i++) {
        if (contexts[i]->hidden)
            continue;
        if (!opts.probing && strcmp(contexts[i]->name, opts.context_name) != 0)
            continue;
        if (!api_auto && strcmp(contexts[i]->type, opts.context_type) != 0)
            continue;

        struct ra_ctx *ctx = talloc_ptrtype(NULL, ctx);
        *ctx = (struct ra_ctx) {
            .global = global,
            .log = mp_log_new(ctx, parent, contexts[i]->type),
            .opts = opts,
            .fns = contexts[i],
        };

        MP_VERBOSE(ctx, "Initializing GPU context '%s'\n", ctx->fns->name);
        if (contexts[i]->init(ctx)) 
            return ctx;

        talloc_free(ctx);
    }

    // If we've reached this point, then none of the contexts matched the name
    // requested, or the backend creation failed for all of them.
    mp_err(parent, "Failed initializing any suitable GPU context!\n");
    return NULL;
}

struct ra_ctx *ra_ctx_create_by_name(struct mpv_global *global, 
                                     struct mp_log *parent, const char *name)
{
    for (int i = 0; i < MP_ARRAY_SIZE(contexts); i++) {
        if (strcmp(name, contexts[i]->name) != 0)
            continue;

        struct ra_ctx *ctx = talloc_ptrtype(NULL, ctx);
        *ctx = (struct ra_ctx) {
            .global = global,
            .log = mp_log_new(ctx, parent, contexts[i]->type),
            .fns = contexts[i],
        };

        MP_VERBOSE(ctx, "Initializing GPU context '%s'\n", ctx->fns->name);
        if (contexts[i]->init(ctx))
            return ctx;
        talloc_free(ctx);
    }

    mp_err(parent, "Failed initializing suitable GPU context for '%s'\n", name);
    return NULL;
}

void ra_ctx_destroy(struct ra_ctx **ctx_ptr)
{
    struct ra_ctx *ctx = *ctx_ptr;
    if (!ctx)
        return;

    if (ctx->spirv && ctx->spirv->fns->uninit)
        ctx->spirv->fns->uninit(ctx);

    ctx->fns->uninit(ctx);
    talloc_free(ctx);

    *ctx_ptr = NULL;
}
