#include "msg.h"
#include "spirv.h"
#include "config.h"

extern const struct spirv_compiler_fns spirv_shaderc;

static const struct spirv_compiler_fns *compilers[] = {
#if HAVE_SHADERC
    &spirv_shaderc,
#endif
};

bool spirv_compiler_init(struct ra_ctx *ctx)
{
    for (int i = 0; i < MP_ARRAY_SIZE(compilers); i++) {
        ctx->spirv = talloc_zero(ctx, struct spirv_compiler);
        ctx->spirv->log = ctx->log,
        ctx->spirv->fns = compilers[i];

        MP_VERBOSE(ctx, "Initializing SPIR-V compiler '%s'\n", ctx->spirv->name);
        if (ctx->spirv->fns->init(ctx))
            return true;
        talloc_free(ctx->spirv);
        ctx->spirv = NULL;
    }

    MP_ERR(ctx, "Failed initializing SPIR-V compiler!\n");
    return false;
}
