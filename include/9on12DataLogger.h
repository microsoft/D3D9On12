// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once

namespace D3D9on12
{
    class ShaderDataLogger
    {
    public:
        ShaderDataLogger() : m_WorstCaseShader(1, 0) {}

        void AddShaderData(D3D10_SB_TOKENIZED_PROGRAM_TYPE shaderType, UINT numInstructions, UINT numExtraInstructions)
        {
            float extraInstructionOverhead = (float)numExtraInstructions / (float)numInstructions;
            float worstShaderOverhead = (float)m_WorstCaseShader.m_numExtraInstructions / (float)m_WorstCaseShader.m_numInstructions;
            if (extraInstructionOverhead > worstShaderOverhead)
            {
                m_WorstCaseShader = ShaderInfo(numInstructions, numExtraInstructions );
            }

            m_shaderData[shaderType].m_totalInstructions += numInstructions;
            m_shaderData[shaderType].m_totalExtraInstructions += numExtraInstructions;
            m_shaderData[shaderType].m_totalShaders++;
        }

    private:
        class ShaderInfo
        {
        public:
            ShaderInfo() : m_numInstructions(0), m_numExtraInstructions(0) {}
            ShaderInfo(UINT numInstructions, UINT numExtraInstructions) : m_numInstructions(numInstructions), m_numExtraInstructions(numExtraInstructions) {}

            UINT m_numInstructions;
            UINT m_numExtraInstructions;
        };

        struct AccumulatedShaderData
        {
            AccumulatedShaderData() { memset(this, 0, sizeof(*this)); }

            UINT64 m_totalInstructions;
            UINT64 m_totalExtraInstructions;
            UINT64 m_totalShaders;
        };

        AccumulatedShaderData m_shaderData[2];
        ShaderInfo m_WorstCaseShader;
    };

    struct DataLogger
    {

        void AddShaderData(D3D10_SB_TOKENIZED_PROGRAM_TYPE shaderType, UINT numInstructions, UINT numExtraInstructions)
        {
            m_shaderDataLogger.AddShaderData(shaderType, numInstructions, numExtraInstructions);
        }

        ShaderDataLogger m_shaderDataLogger;
    };
};