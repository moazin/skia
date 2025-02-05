/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSPIRVUniformHandler_DEFINED
#define GrSPIRVUniformHandler_DEFINED

#include "src/core/SkTBlockList.h"
#include "src/gpu/GrUniformDataManager.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"

/*
 * This class can be used for basic SPIR-V uniform handling. It will make a single uniform buffer
 * for all the uniforms and will be placed in the first set and binding. Textures and samplers are
 * placed in the second set and kept as separate objects. They are interleaved as sampler texture
 * pairs with each object in the next binding slot.
 */
class GrSPIRVUniformHandler : public GrGLSLUniformHandler {
public:
    static const int kUniformsPerBlock = 8;

    const GrShaderVar& getUniformVariable(UniformHandle u) const override;
    const char* getUniformCStr(UniformHandle u) const override;

    struct SPIRVUniformInfo : public UniformInfo {
        int fUBOOffset;
    };
    typedef SkTBlockList<SPIRVUniformInfo> UniformInfoArray;
    enum {
        kUniformBinding = 0,
        kUniformDescriptorSet = 0,
        kSamplerTextureDescriptorSet = 1,
    };
    uint32_t getRTFlipOffset() const;

    int numUniforms() const override {
        return fUniforms.count();
    }

    UniformInfo& uniform(int idx) override {
        return fUniforms.item(idx);
    }
    const UniformInfo& uniform(int idx) const override {
        return fUniforms.item(idx);
    }

    /**
     * Call after all legacy style uniforms have been added to assign offsets to new style uniforms
     * and create the data structure needed to transfer new style uniforms to GrUniformDataManager.
     * This must be called before appendUniformDecls() in order to ensure new style uniforms get
     * declared. It must be called only once.
     */
    GrUniformDataManager::ProgramUniforms getNewProgramUniforms(const GrUniformAggregator&);

private:
    explicit GrSPIRVUniformHandler(GrGLSLProgramBuilder* program);

    SamplerHandle addSampler(const GrBackendFormat&, GrSamplerState, const GrSwizzle&,
                             const char* name, const GrShaderCaps*) override;
    const char* samplerVariable(SamplerHandle handle) const override;
    GrSwizzle samplerSwizzle(SamplerHandle handle) const override;
    void appendUniformDecls(const GrUniformAggregator&,
                            GrShaderFlags visibility,
                            SkString*) const override;
    UniformHandle internalAddUniformArray(const GrFragmentProcessor* owner,
                                          uint32_t visibility,
                                          GrSLType type,
                                          const char* name,
                                          bool mangleName,
                                          int arrayCount,
                                          const char** outName) override;

    UniformInfoArray    fUniforms;
    UniformInfoArray    fNewUniforms;
    UniformInfoArray    fSamplers;
    UniformInfoArray    fTextures;
    SkTArray<GrSwizzle> fSamplerSwizzles;
    SkTArray<SkString>  fSamplerReferences;

    uint32_t fCurrentUBOOffset = 0;
    uint32_t fRTFlipOffset = 0;

    friend class GrD3DPipelineStateBuilder;
    friend class GrDawnProgramBuilder;

    using INHERITED = GrGLSLUniformHandler;
};

#endif
