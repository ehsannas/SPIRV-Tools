// Copyright (c) 2017 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Common validation fixtures for unit tests

#include "gmock/gmock.h"
#include "source/val/decoration.h"
#include "unit_spirv.h"
#include "val_fixtures.h"

namespace {

using ::testing::Eq;
using ::testing::HasSubstr;
using libspirv::Decoration;
using std::string;
using std::vector;

using ValidateDecorations = spvtest::ValidateBase<bool>;

TEST_F(ValidateDecorations, ValidateOpDecorateRegistration) {
  string spirv = R"(
    OpCapability Shader
    OpCapability Linkage
    OpMemoryModel Logical GLSL450
    OpDecorate %1 ArrayStride 4
    OpDecorate %1 Uniform
    %2 = OpTypeFloat 32
    %1 = OpTypeRuntimeArray %2
    ; Since %1 is used first in Decoration, it gets id 1.
)";
  const uint32_t id = 1;
  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateAndRetrieveValidationState());
  // Must have 2 decorations.
  EXPECT_THAT(vstate_->id_decorations(id),
              Eq(vector<Decoration>{Decoration(SpvDecorationArrayStride, {4}),
                                    Decoration(SpvDecorationUniform)}));
}

TEST_F(ValidateDecorations, ValidateOpMemberDecorateRegistration) {
  string spirv = R"(
    OpCapability Shader
    OpCapability Linkage
    OpMemoryModel Logical GLSL450
    OpDecorate %_arr_double_uint_6 ArrayStride 4
    OpMemberDecorate %_struct_115 0 Offset 0
    OpMemberDecorate %_struct_115 1 Offset 4
    OpMemberDecorate %_struct_115 2 NonReadable
    OpMemberDecorate %_struct_115 2 Offset 8
    OpDecorate %_struct_115 BufferBlock
    %float = OpTypeFloat 32
    %uint = OpTypeInt 32 0
    %uint_6 = OpConstant %uint 6
    %_arr_double_uint_6 = OpTypeArray %float %uint_6
    %_struct_115 = OpTypeStruct %float %float %_arr_double_uint_6
)";
  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateAndRetrieveValidationState());

  // The array must have 1 decoration.
  const uint32_t arr_id = 1;
  EXPECT_THAT(
      vstate_->id_decorations(arr_id),
      Eq(vector<Decoration>{Decoration(SpvDecorationArrayStride, {4})}));

  // The struct must have 5 decorations.
  const uint32_t struct_id = 2;
  EXPECT_THAT(vstate_->id_decorations(struct_id),
              Eq(vector<Decoration>{Decoration(SpvDecorationOffset, {0}, 0),
                                    Decoration(SpvDecorationOffset, {4}, 1),
                                    Decoration(SpvDecorationNonReadable, {}, 2),
                                    Decoration(SpvDecorationOffset, {8}, 2),
                                    Decoration(SpvDecorationBufferBlock)}));
}

TEST_F(ValidateDecorations, ValidateGroupDecorateRegistration) {
  string spirv = R"(
               OpCapability Shader
               OpCapability Linkage
               OpMemoryModel Logical GLSL450
               OpDecorate %1 DescriptorSet 0
               OpDecorate %1 NonWritable
               OpDecorate %1 Restrict
          %1 = OpDecorationGroup
               OpGroupDecorate %1 %2 %3
               OpGroupDecorate %1 %4
  %float = OpTypeFloat 32
%_runtimearr_float = OpTypeRuntimeArray %float
  %_struct_9 = OpTypeStruct %_runtimearr_float
%_ptr_Uniform__struct_9 = OpTypePointer Uniform %_struct_9
         %2 = OpVariable %_ptr_Uniform__struct_9 Uniform
 %_struct_10 = OpTypeStruct %_runtimearr_float
%_ptr_Uniform__struct_10 = OpTypePointer Uniform %_struct_10
         %3 = OpVariable %_ptr_Uniform__struct_10 Uniform
 %_struct_11 = OpTypeStruct %_runtimearr_float
%_ptr_Uniform__struct_11 = OpTypePointer Uniform %_struct_11
         %4 = OpVariable %_ptr_Uniform__struct_11 Uniform
  )";
  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateAndRetrieveValidationState());

  // Decoration group has 3 decorations.
  auto expected_decorations = vector<Decoration>{
      Decoration(SpvDecorationDescriptorSet, {0}),
      Decoration(SpvDecorationNonWritable), Decoration(SpvDecorationRestrict)};

  // Decoration group is applied to id 1, 2, 3, and 4. Note that id 1 (which is
  // the decoration group id) also has all the decorations.
  EXPECT_THAT(vstate_->id_decorations(1), Eq(expected_decorations));
  EXPECT_THAT(vstate_->id_decorations(2), Eq(expected_decorations));
  EXPECT_THAT(vstate_->id_decorations(3), Eq(expected_decorations));
  EXPECT_THAT(vstate_->id_decorations(4), Eq(expected_decorations));
}

TEST_F(ValidateDecorations, ValidateGroupMemberDecorateRegistration) {
  string spirv = R"(
               OpCapability Shader
               OpCapability Linkage
               OpMemoryModel Logical GLSL450
               OpDecorate %1 Offset 3
          %1 = OpDecorationGroup
               OpGroupMemberDecorate %1 %_struct_1 3 %_struct_2 3 %_struct_3 3
      %float = OpTypeFloat 32
%_runtimearr = OpTypeRuntimeArray %float
  %_struct_1 = OpTypeStruct %float %float %float %_runtimearr
  %_struct_2 = OpTypeStruct %float %float %float %_runtimearr
  %_struct_3 = OpTypeStruct %float %float %float %_runtimearr
  )";
  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateAndRetrieveValidationState());
  // Decoration group has 1 decoration.
  auto expected_decorations =
      vector<Decoration>{Decoration(SpvDecorationOffset, {3}, 3)};

  // Decoration group is applied to id 2, 3, and 4.
  EXPECT_THAT(vstate_->id_decorations(2), Eq(expected_decorations));
  EXPECT_THAT(vstate_->id_decorations(3), Eq(expected_decorations));
  EXPECT_THAT(vstate_->id_decorations(4), Eq(expected_decorations));
}

TEST_F(ValidateDecorations, LinkageImportUsedForInitializedVariableBad) {
  string spirv = R"(
               OpCapability Shader
               OpCapability Linkage
               OpMemoryModel Logical GLSL450
               OpDecorate %target LinkageAttributes "link_ptr" Import
      %float = OpTypeFloat 32
 %_ptr_float = OpTypePointer Uniform %float
       %zero = OpConstantNull %float
     %target = OpVariable %_ptr_float Uniform %zero
  )";
  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateAndRetrieveValidationState());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("A module-scope OpVariable with initialization value "
                        "cannot be marked with the Import Linkage Type."));
}
TEST_F(ValidateDecorations, LinkageExportUsedForInitializedVariableGood) {
  string spirv = R"(
               OpCapability Shader
               OpCapability Linkage
               OpMemoryModel Logical GLSL450
               OpDecorate %target LinkageAttributes "link_ptr" Export
      %float = OpTypeFloat 32
 %_ptr_float = OpTypePointer Uniform %float
       %zero = OpConstantNull %float
     %target = OpVariable %_ptr_float Uniform %zero
  )";
  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateAndRetrieveValidationState());
}

TEST_F(ValidateDecorations, StructAllMembersHaveBuiltInDecorationsGood) {
  string spirv = R"(
               OpCapability Shader
               OpCapability Linkage
               OpMemoryModel Logical GLSL450
               OpMemberDecorate %_struct_1 0 BuiltIn Position
               OpMemberDecorate %_struct_1 1 BuiltIn Position
               OpMemberDecorate %_struct_1 2 BuiltIn Position
               OpMemberDecorate %_struct_1 3 BuiltIn Position
      %float = OpTypeFloat 32
%_runtimearr = OpTypeRuntimeArray %float
  %_struct_1 = OpTypeStruct %float %float %float %_runtimearr
  )";
  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateAndRetrieveValidationState());
}

TEST_F(ValidateDecorations, MixedBuiltInDecorationsBad) {
  string spirv = R"(
               OpCapability Shader
               OpCapability Linkage
               OpMemoryModel Logical GLSL450
               OpMemberDecorate %_struct_1 0 BuiltIn Position
               OpMemberDecorate %_struct_1 1 BuiltIn Position
      %float = OpTypeFloat 32
%_runtimearr = OpTypeRuntimeArray %float
  %_struct_1 = OpTypeStruct %float %float %float %_runtimearr
  )";
  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateAndRetrieveValidationState());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("When BuiltIn decoration is applied to a structure-type "
                "member, all members of that structure type must also be "
                "decorated with BuiltIn (No allowed mixing of built-in "
                "variables and non-built-in variables within a single "
                "structure). Structure id 1 does not meet this requirement."));
}

TEST_F(ValidateDecorations, StructContainsBuiltInStructBad) {
  string spirv = R"(
               OpCapability Shader
               OpCapability Linkage
               OpMemoryModel Logical GLSL450
               OpMemberDecorate %_struct_1 0 BuiltIn Position
               OpMemberDecorate %_struct_1 1 BuiltIn Position
               OpMemberDecorate %_struct_1 2 BuiltIn Position
               OpMemberDecorate %_struct_1 3 BuiltIn Position
      %float = OpTypeFloat 32
%_runtimearr = OpTypeRuntimeArray %float
  %_struct_1 = OpTypeStruct %float %float %float %_runtimearr
  %_struct_2 = OpTypeStruct %_struct_1
  )";
  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateAndRetrieveValidationState());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Structure <id> 1 contains members with BuiltIn "
                        "decoration. Therefore this structure may not be "
                        "contained as a member of another structure type. "
                        "Structure <id> 4 contains structure <id> 1."));
}

TEST_F(ValidateDecorations, StructContainsNonBuiltInStructGood) {
  string spirv = R"(
               OpCapability Shader
               OpCapability Linkage
               OpMemoryModel Logical GLSL450
      %float = OpTypeFloat 32
  %_struct_1 = OpTypeStruct %float
  %_struct_2 = OpTypeStruct %_struct_1
  )";
  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateAndRetrieveValidationState());
}

TEST_F(ValidateDecorations, MultipleBuiltInObjectsConsumedByOpEntryPointBad) {
  string spirv = R"(
               OpCapability Shader
               OpCapability Geometry
               OpMemoryModel Logical GLSL450
               OpEntryPoint Geometry %main "main" %in_1 %in_2
               OpMemberDecorate %struct_1 0 BuiltIn InvocationId
               OpMemberDecorate %struct_2 0 BuiltIn Position
      %int = OpTypeInt 32 1
     %void = OpTypeVoid
     %func = OpTypeFunction %void
    %float = OpTypeFloat 32
 %struct_1 = OpTypeStruct %int
 %struct_2 = OpTypeStruct %float
%ptr_builtin_1 = OpTypePointer Input %struct_1
%ptr_builtin_2 = OpTypePointer Input %struct_2
%in_1 = OpVariable %ptr_builtin_1 Input
%in_2 = OpVariable %ptr_builtin_2 Input
       %main = OpFunction %void None %func
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_BINARY, ValidateAndRetrieveValidationState());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("There must be at most one object per Storage Class "
                        "that can contain a structure type containing members "
                        "decorated with BuiltIn, consumed per entry-point."));
}

TEST_F(ValidateDecorations,
       OneBuiltInObjectPerStorageClassConsumedByOpEntryPointGood) {
  string spirv = R"(
               OpCapability Shader
               OpCapability Geometry
               OpMemoryModel Logical GLSL450
               OpEntryPoint Geometry %main "main" %in_1 %out_1
               OpMemberDecorate %struct_1 0 BuiltIn InvocationId
               OpMemberDecorate %struct_2 0 BuiltIn Position
      %int = OpTypeInt 32 1
     %void = OpTypeVoid
     %func = OpTypeFunction %void
    %float = OpTypeFloat 32
 %struct_1 = OpTypeStruct %int
 %struct_2 = OpTypeStruct %float
%ptr_builtin_1 = OpTypePointer Input %struct_1
%ptr_builtin_2 = OpTypePointer Output %struct_2
%in_1 = OpVariable %ptr_builtin_1 Input
%out_1 = OpVariable %ptr_builtin_2 Output
       %main = OpFunction %void None %func
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateAndRetrieveValidationState());
}

TEST_F(ValidateDecorations, NoBuiltInObjectsConsumedByOpEntryPointGood) {
  string spirv = R"(
               OpCapability Shader
               OpCapability Geometry
               OpMemoryModel Logical GLSL450
               OpEntryPoint Geometry %main "main" %in_1 %out_1
      %int = OpTypeInt 32 1
     %void = OpTypeVoid
     %func = OpTypeFunction %void
    %float = OpTypeFloat 32
 %struct_1 = OpTypeStruct %int
 %struct_2 = OpTypeStruct %float
%ptr_builtin_1 = OpTypePointer Input %struct_1
%ptr_builtin_2 = OpTypePointer Output %struct_2
%in_1 = OpVariable %ptr_builtin_1 Input
%out_1 = OpVariable %ptr_builtin_2 Output
       %main = OpFunction %void None %func
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateAndRetrieveValidationState());
}

TEST_F(ValidateDecorations, EntryPointFunctionHasLinkageAttributeBad) {
  string spirv = R"(
      OpCapability Shader
      OpCapability Linkage
      OpMemoryModel Logical GLSL450
      OpEntryPoint GLCompute %main "main"
      OpDecorate %main LinkageAttributes "import_main" Import
%1 = OpTypeVoid
%2 = OpTypeFunction %1
%main = OpFunction %1 None %2
%4 = OpLabel
     OpReturn
     OpFunctionEnd
)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_BINARY, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("The LinkageAttributes Decoration (Linkage name: import_main) "
                "cannot be applied to function id 1 because it is targeted by "
                "an OpEntryPoint instruction."));
}

TEST_F(ValidateDecorations, FunctionDeclarationWithoutImportLinkageBad) {
  string spirv = R"(
               OpCapability Shader
               OpCapability Linkage
               OpMemoryModel Logical GLSL450
     %void = OpTypeVoid
     %func = OpTypeFunction %void
       %main = OpFunction %void None %func
               OpFunctionEnd
  )";
  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_BINARY, ValidateAndRetrieveValidationState());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Function declaration (id 3) must have a LinkageAttributes "
                "decoration with the Import Linkage type."));
}

TEST_F(ValidateDecorations, FunctionDeclarationWithImportLinkageGood) {
  string spirv = R"(
               OpCapability Shader
               OpCapability Linkage
               OpMemoryModel Logical GLSL450
               OpDecorate %main LinkageAttributes "link_fn" Import
     %void = OpTypeVoid
     %func = OpTypeFunction %void
       %main = OpFunction %void None %func
               OpFunctionEnd
  )";
  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateAndRetrieveValidationState());
}

TEST_F(ValidateDecorations, FunctionDeclarationWithExportLinkageBad) {
  string spirv = R"(
               OpCapability Shader
               OpCapability Linkage
               OpMemoryModel Logical GLSL450
               OpDecorate %main LinkageAttributes "link_fn" Export
     %void = OpTypeVoid
     %func = OpTypeFunction %void
       %main = OpFunction %void None %func
               OpFunctionEnd
  )";
  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_BINARY, ValidateAndRetrieveValidationState());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Function declaration (id 1) must have a LinkageAttributes "
                "decoration with the Import Linkage type."));
}

TEST_F(ValidateDecorations, FunctionDefinitionWithImportLinkageBad) {
  string spirv = R"(
               OpCapability Shader
               OpCapability Linkage
               OpMemoryModel Logical GLSL450
               OpDecorate %main LinkageAttributes "link_fn" Import
     %void = OpTypeVoid
     %func = OpTypeFunction %void
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_BINARY, ValidateAndRetrieveValidationState());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Function definition (id 1) may not be decorated with "
                        "Import Linkage type."));
}

TEST_F(ValidateDecorations, FunctionDefinitionWithoutImportLinkageGood) {
  string spirv = R"(
               OpCapability Shader
               OpCapability Linkage
               OpMemoryModel Logical GLSL450
     %void = OpTypeVoid
     %func = OpTypeFunction %void
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
  )";
  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateAndRetrieveValidationState());
}

TEST_F(ValidateDecorations, BuiltinVariablesGoodVulkan) {
  const spv_target_env env = SPV_ENV_VULKAN_1_0;
  std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %gl_FragCoord %_entryPointOutput
OpExecutionMode %main OriginUpperLeft
OpSource HLSL 500
OpDecorate %gl_FragCoord BuiltIn FragCoord
OpDecorate %_entryPointOutput Location 0
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%float_0 = OpConstant %float 0
%14 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_ptr_Input_v4float = OpTypePointer Input %v4float
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput = OpVariable %_ptr_Output_v4float Output
%main = OpFunction %void None %3
%5 = OpLabel
OpStore %_entryPointOutput %14
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, env);
  EXPECT_EQ(SPV_SUCCESS, ValidateAndRetrieveValidationState(env));
}

TEST_F(ValidateDecorations, BuiltinVariablesWithLocationDecorationVulkan) {
  const spv_target_env env = SPV_ENV_VULKAN_1_0;
  std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %gl_FragCoord %_entryPointOutput
OpExecutionMode %main OriginUpperLeft
OpSource HLSL 500
OpDecorate %gl_FragCoord BuiltIn FragCoord
OpDecorate %gl_FragCoord Location 0
OpDecorate %_entryPointOutput Location 0
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%float_0 = OpConstant %float 0
%14 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_ptr_Input_v4float = OpTypePointer Input %v4float
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput = OpVariable %_ptr_Output_v4float Output
%main = OpFunction %void None %3
%5 = OpLabel
OpStore %_entryPointOutput %14
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, env);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateAndRetrieveValidationState(env));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("A BuiltIn variable (id 2) cannot have any Location or "
                        "Component decorations"));
}
TEST_F(ValidateDecorations, BuiltinVariablesWithComponentDecorationVulkan) {
  const spv_target_env env = SPV_ENV_VULKAN_1_0;
  std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %gl_FragCoord %_entryPointOutput
OpExecutionMode %main OriginUpperLeft
OpSource HLSL 500
OpDecorate %gl_FragCoord BuiltIn FragCoord
OpDecorate %gl_FragCoord Component 0
OpDecorate %_entryPointOutput Location 0
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%float_0 = OpConstant %float 0
%14 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_ptr_Input_v4float = OpTypePointer Input %v4float
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput = OpVariable %_ptr_Output_v4float Output
%main = OpFunction %void None %3
%5 = OpLabel
OpStore %_entryPointOutput %14
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, env);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateAndRetrieveValidationState(env));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("A BuiltIn variable (id 2) cannot have any Location or "
                        "Component decorations"));
}

TEST_F(ValidateDecorations, BlockMissingOffsetBad) {
  string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpDecorate %Output Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
     %Output = OpTypeStruct %float
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateAndRetrieveValidationState());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("must be explicitly laid out with Offset decorations"));
}

TEST_F(ValidateDecorations, BufferBlockMissingOffsetBad) {
  string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpDecorate %Output BufferBlock
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
     %Output = OpTypeStruct %float
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateAndRetrieveValidationState());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("must be explicitly laid out with Offset decorations"));
}

TEST_F(ValidateDecorations, BlockNestedStructMissingOffsetBad) {
  string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %Output 0 Offset 0
               OpMemberDecorate %Output 1 Offset 16
               OpMemberDecorate %Output 2 Offset 32
               OpDecorate %Output Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %v3float = OpTypeVector %float 3
        %int = OpTypeInt 32 1
          %S = OpTypeStruct %v3float %int
     %Output = OpTypeStruct %float %v4float %S
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateAndRetrieveValidationState());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("must be explicitly laid out with Offset decorations"));
}

TEST_F(ValidateDecorations, BufferBlockNestedStructMissingOffsetBad) {
  string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %Output 0 Offset 0
               OpMemberDecorate %Output 1 Offset 16
               OpMemberDecorate %Output 2 Offset 32
               OpDecorate %Output BufferBlock
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %v3float = OpTypeVector %float 3
        %int = OpTypeInt 32 1
          %S = OpTypeStruct %v3float %int
     %Output = OpTypeStruct %float %v4float %S
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateAndRetrieveValidationState());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("must be explicitly laid out with Offset decorations"));
}

TEST_F(ValidateDecorations, BlockGLSLSharedBad) {
  string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpDecorate %Output Block
               OpDecorate %Output GLSLShared
               OpMemberDecorate %Output 0 Offset 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
     %Output = OpTypeStruct %float
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateAndRetrieveValidationState());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("must not use GLSLShared decoration"));
}

TEST_F(ValidateDecorations, BufferBlockGLSLSharedBad) {
  string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpDecorate %Output BufferBlock
               OpDecorate %Output GLSLShared
               OpMemberDecorate %Output 0 Offset 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
     %Output = OpTypeStruct %float
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateAndRetrieveValidationState());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("must not use GLSLShared decoration"));
}

TEST_F(ValidateDecorations, BlockNestedStructGLSLSharedBad) {
  string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpMemberDecorate %S 0 Offset 0
               OpDecorate %S GLSLShared
               OpMemberDecorate %Output 0 Offset 0
               OpMemberDecorate %Output 1 Offset 16
               OpMemberDecorate %Output 2 Offset 32
               OpDecorate %Output Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
        %int = OpTypeInt 32 1
          %S = OpTypeStruct %int
     %Output = OpTypeStruct %float %v4float %S
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateAndRetrieveValidationState());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("must not use GLSLShared decoration"));
}

TEST_F(ValidateDecorations, BufferBlockNestedStructGLSLSharedBad) {
  string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpMemberDecorate %S 0 Offset 0
               OpDecorate %S GLSLShared
               OpMemberDecorate %Output 0 Offset 0
               OpMemberDecorate %Output 1 Offset 16
               OpMemberDecorate %Output 2 Offset 32
               OpDecorate %Output BufferBlock
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
        %int = OpTypeInt 32 1
          %S = OpTypeStruct %int
     %Output = OpTypeStruct %float %v4float %S
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateAndRetrieveValidationState());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("must not use GLSLShared decoration"));
}

TEST_F(ValidateDecorations, BlockGLSLPackedBad) {
  string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpDecorate %Output Block
               OpDecorate %Output GLSLPacked
               OpMemberDecorate %Output 0 Offset 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
     %Output = OpTypeStruct %float
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateAndRetrieveValidationState());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("must not use GLSLPacked decoration"));
}

TEST_F(ValidateDecorations, BufferBlockGLSLPackedBad) {
  string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpDecorate %Output BufferBlock
               OpDecorate %Output GLSLPacked
               OpMemberDecorate %Output 0 Offset 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
     %Output = OpTypeStruct %float
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateAndRetrieveValidationState());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("must not use GLSLPacked decoration"));
}

TEST_F(ValidateDecorations, BlockNestedStructGLSLPackedBad) {
  string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpMemberDecorate %S 0 Offset 0
               OpDecorate %S GLSLPacked
               OpMemberDecorate %Output 0 Offset 0
               OpMemberDecorate %Output 1 Offset 16
               OpMemberDecorate %Output 2 Offset 32
               OpDecorate %Output Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
        %int = OpTypeInt 32 1
          %S = OpTypeStruct %int
     %Output = OpTypeStruct %float %v4float %S
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateAndRetrieveValidationState());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("must not use GLSLPacked decoration"));
}

TEST_F(ValidateDecorations, BufferBlockNestedStructGLSLPackedBad) {
  string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpMemberDecorate %S 0 Offset 0
               OpDecorate %S GLSLPacked
               OpMemberDecorate %Output 0 Offset 0
               OpMemberDecorate %Output 1 Offset 16
               OpMemberDecorate %Output 2 Offset 32
               OpDecorate %Output BufferBlock
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
        %int = OpTypeInt 32 1
          %S = OpTypeStruct %int
     %Output = OpTypeStruct %float %v4float %S
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateAndRetrieveValidationState());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("must not use GLSLPacked decoration"));
}

TEST_F(ValidateDecorations, BlockMissingArrayStrideBad) {
  string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpDecorate %Output Block
               OpMemberDecorate %Output 0 Offset 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
        %int = OpTypeInt 32 1
      %int_3 = OpConstant %int 3
      %array = OpTypeArray %float %int_3
     %Output = OpTypeStruct %array
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateAndRetrieveValidationState());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("must be explicitly laid out with ArrayStride decorations"));
}

TEST_F(ValidateDecorations, BufferBlockMissingArrayStrideBad) {
  string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpDecorate %Output BufferBlock
               OpMemberDecorate %Output 0 Offset 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
        %int = OpTypeInt 32 1
      %int_3 = OpConstant %int 3
      %array = OpTypeArray %float %int_3
     %Output = OpTypeStruct %array
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateAndRetrieveValidationState());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("must be explicitly laid out with ArrayStride decorations"));
}

TEST_F(ValidateDecorations, BlockNestedStructMissingArrayStrideBad) {
  string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %Output 0 Offset 0
               OpMemberDecorate %Output 1 Offset 16
               OpMemberDecorate %Output 2 Offset 32
               OpDecorate %Output Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
        %int = OpTypeInt 32 1
      %int_3 = OpConstant %int 3
      %array = OpTypeArray %float %int_3
          %S = OpTypeStruct %array
     %Output = OpTypeStruct %float %v4float %S
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateAndRetrieveValidationState());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("must be explicitly laid out with ArrayStride decorations"));
}

TEST_F(ValidateDecorations, BufferBlockNestedStructMissingArrayStrideBad) {
  string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %Output 0 Offset 0
               OpMemberDecorate %Output 1 Offset 16
               OpMemberDecorate %Output 2 Offset 32
               OpDecorate %Output BufferBlock
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
        %int = OpTypeInt 32 1
      %int_3 = OpConstant %int 3
      %array = OpTypeArray %float %int_3
          %S = OpTypeStruct %array
     %Output = OpTypeStruct %float %v4float %S
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateAndRetrieveValidationState());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("must be explicitly laid out with ArrayStride decorations"));
}

TEST_F(ValidateDecorations, BlockMissingMatrixStrideBad) {
  string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpDecorate %Output Block
               OpMemberDecorate %Output 0 Offset 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
     %matrix = OpTypeMatrix %v3float 4
     %Output = OpTypeStruct %matrix
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateAndRetrieveValidationState());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("must be explicitly laid out with MatrixStride decorations"));
}

TEST_F(ValidateDecorations, BufferBlockMissingMatrixStrideBad) {
  string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpDecorate %Output BufferBlock
               OpMemberDecorate %Output 0 Offset 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
     %matrix = OpTypeMatrix %v3float 4
     %Output = OpTypeStruct %matrix
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateAndRetrieveValidationState());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("must be explicitly laid out with MatrixStride decorations"));
}

TEST_F(ValidateDecorations, BlockMissingMatrixStrideArrayBad) {
  string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpDecorate %Output Block
               OpMemberDecorate %Output 0 Offset 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
     %matrix = OpTypeMatrix %v3float 4
        %int = OpTypeInt 32 1
      %int_3 = OpConstant %int 3
      %array = OpTypeArray %matrix %int_3
     %Output = OpTypeStruct %matrix
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateAndRetrieveValidationState());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("must be explicitly laid out with MatrixStride decorations"));
}

TEST_F(ValidateDecorations, BufferBlockMissingMatrixStrideArrayBad) {
  string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpDecorate %Output BufferBlock
               OpMemberDecorate %Output 0 Offset 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
     %matrix = OpTypeMatrix %v3float 4
        %int = OpTypeInt 32 1
      %int_3 = OpConstant %int 3
      %array = OpTypeArray %matrix %int_3
     %Output = OpTypeStruct %matrix
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateAndRetrieveValidationState());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("must be explicitly laid out with MatrixStride decorations"));
}

TEST_F(ValidateDecorations, BlockNestedStructMissingMatrixStrideBad) {
  string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %Output 0 Offset 0
               OpMemberDecorate %Output 1 Offset 16
               OpMemberDecorate %Output 2 Offset 32
               OpDecorate %Output Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
    %v4float = OpTypeVector %float 4
     %matrix = OpTypeMatrix %v3float 4
          %S = OpTypeStruct %matrix
     %Output = OpTypeStruct %float %v4float %S
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateAndRetrieveValidationState());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("must be explicitly laid out with MatrixStride decorations"));
}

TEST_F(ValidateDecorations, BufferBlockNestedStructMissingMatrixStrideBad) {
  string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %Output 0 Offset 0
               OpMemberDecorate %Output 1 Offset 16
               OpMemberDecorate %Output 2 Offset 32
               OpDecorate %Output BufferBlock
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
    %v4float = OpTypeVector %float 4
     %matrix = OpTypeMatrix %v3float 4
          %S = OpTypeStruct %matrix
     %Output = OpTypeStruct %float %v4float %S
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateAndRetrieveValidationState());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("must be explicitly laid out with MatrixStride decorations"));
}

TEST_F(ValidateDecorations, BlockStandardUniformBufferLayout) {
  string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpMemberDecorate %F 0 Offset 0
               OpMemberDecorate %F 1 Offset 8
               OpDecorate %_arr_float_uint_2 ArrayStride 16
               OpDecorate %_arr_mat3v3float_uint_2 ArrayStride 48
               OpMemberDecorate %O 0 Offset 0
               OpMemberDecorate %O 1 Offset 16
               OpMemberDecorate %O 2 Offset 32
               OpMemberDecorate %O 3 Offset 64
               OpMemberDecorate %O 4 ColMajor
               OpMemberDecorate %O 4 Offset 80
               OpMemberDecorate %O 4 MatrixStride 16
               OpDecorate %_arr_O_uint_2 ArrayStride 176
               OpMemberDecorate %Output 0 Offset 0
               OpMemberDecorate %Output 1 Offset 8
               OpMemberDecorate %Output 2 Offset 16
               OpMemberDecorate %Output 3 Offset 32
               OpMemberDecorate %Output 4 Offset 48
               OpMemberDecorate %Output 5 Offset 64
               OpMemberDecorate %Output 6 ColMajor
               OpMemberDecorate %Output 6 Offset 96
               OpMemberDecorate %Output 6 MatrixStride 16
               OpMemberDecorate %Output 7 Offset 128
               OpDecorate %Output Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
    %v3float = OpTypeVector %float 3
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
          %F = OpTypeStruct %int %v2uint
     %uint_2 = OpConstant %uint 2
%_arr_float_uint_2 = OpTypeArray %float %uint_2
%mat2v3float = OpTypeMatrix %v3float 2
     %v3uint = OpTypeVector %uint 3
%mat3v3float = OpTypeMatrix %v3float 3
%_arr_mat3v3float_uint_2 = OpTypeArray %mat3v3float %uint_2
          %O = OpTypeStruct %v3uint %v2float %_arr_float_uint_2 %v2float %_arr_mat3v3float_uint_2
%_arr_O_uint_2 = OpTypeArray %O %uint_2
     %Output = OpTypeStruct %float %v2float %v3float %F %float %_arr_float_uint_2 %mat2v3float %_arr_O_uint_2
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateAndRetrieveValidationState());
}

TEST_F(ValidateDecorations, BufferBlock16bitStandardStorageBufferLayout) {
  string spirv = R"(
             OpCapability Shader
             OpCapability StorageUniform16
             OpExtension "SPV_KHR_16bit_storage"
             OpMemoryModel Logical GLSL450
             OpEntryPoint GLCompute %main "main"
             OpExecutionMode %main LocalSize 1 1 1
             OpDecorate %f32arr ArrayStride 4
             OpDecorate %f16arr ArrayStride 2
             OpMemberDecorate %SSBO32 0 Offset 0
             OpMemberDecorate %SSBO16 0 Offset 0
             OpDecorate %SSBO32 BufferBlock
             OpDecorate %SSBO16 BufferBlock
     %void = OpTypeVoid
    %voidf = OpTypeFunction %void
      %u32 = OpTypeInt 32 0
      %i32 = OpTypeInt 32 1
      %f32 = OpTypeFloat 32
    %uvec3 = OpTypeVector %u32 3
 %c_i32_32 = OpConstant %i32 32
%c_i32_128 = OpConstant %i32 128
   %f32arr = OpTypeArray %f32 %c_i32_128
      %f16 = OpTypeFloat 16
   %f16arr = OpTypeArray %f16 %c_i32_128
   %SSBO32 = OpTypeStruct %f32arr
   %SSBO16 = OpTypeStruct %f16arr
%_ptr_Uniform_SSBO32 = OpTypePointer Uniform %SSBO32
 %varSSBO32 = OpVariable %_ptr_Uniform_SSBO32 Uniform
%_ptr_Uniform_SSBO16 = OpTypePointer Uniform %SSBO16
 %varSSBO16 = OpVariable %_ptr_Uniform_SSBO16 Uniform
     %main = OpFunction %void None %voidf
    %label = OpLabel
             OpReturn
             OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateAndRetrieveValidationState());
}

TEST_F(ValidateDecorations, BufferBlockStandardStorageBufferLayout) {
  string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpMemberDecorate %F 0 Offset 0
               OpMemberDecorate %F 1 Offset 8
               OpDecorate %_arr_float_uint_2 ArrayStride 4
               OpDecorate %_arr_mat3v3float_uint_2 ArrayStride 48
               OpMemberDecorate %O 0 Offset 0
               OpMemberDecorate %O 1 Offset 16
               OpMemberDecorate %O 2 Offset 24
               OpMemberDecorate %O 3 Offset 32
               OpMemberDecorate %O 4 ColMajor
               OpMemberDecorate %O 4 Offset 48
               OpMemberDecorate %O 4 MatrixStride 16
               OpDecorate %_arr_O_uint_2 ArrayStride 144
               OpMemberDecorate %Output 0 Offset 0
               OpMemberDecorate %Output 1 Offset 8
               OpMemberDecorate %Output 2 Offset 16
               OpMemberDecorate %Output 3 Offset 32
               OpMemberDecorate %Output 4 Offset 48
               OpMemberDecorate %Output 5 Offset 52
               OpMemberDecorate %Output 6 ColMajor
               OpMemberDecorate %Output 6 Offset 64
               OpMemberDecorate %Output 6 MatrixStride 16
               OpMemberDecorate %Output 7 Offset 96
               OpDecorate %Output BufferBlock
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
    %v3float = OpTypeVector %float 3
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
          %F = OpTypeStruct %int %v2uint
     %uint_2 = OpConstant %uint 2
%_arr_float_uint_2 = OpTypeArray %float %uint_2
%mat2v3float = OpTypeMatrix %v3float 2
     %v3uint = OpTypeVector %uint 3
%mat3v3float = OpTypeMatrix %v3float 3
%_arr_mat3v3float_uint_2 = OpTypeArray %mat3v3float %uint_2
          %O = OpTypeStruct %v3uint %v2float %_arr_float_uint_2 %v2float %_arr_mat3v3float_uint_2
%_arr_O_uint_2 = OpTypeArray %O %uint_2
     %Output = OpTypeStruct %float %v2float %v3float %F %float %_arr_float_uint_2 %mat2v3float %_arr_O_uint_2
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateAndRetrieveValidationState());
}

TEST_F(ValidateDecorations,
       BlockStandardUniformBufferLayoutIncorrectOffset0Bad) {
  string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpMemberDecorate %F 0 Offset 0
               OpMemberDecorate %F 1 Offset 8
               OpDecorate %_arr_float_uint_2 ArrayStride 16
               OpDecorate %_arr_mat3v3float_uint_2 ArrayStride 48
               OpMemberDecorate %O 0 Offset 0
               OpMemberDecorate %O 1 Offset 16
               OpMemberDecorate %O 2 Offset 24
               OpMemberDecorate %O 3 Offset 33
               OpMemberDecorate %O 4 ColMajor
               OpMemberDecorate %O 4 Offset 80
               OpMemberDecorate %O 4 MatrixStride 16
               OpDecorate %_arr_O_uint_2 ArrayStride 176
               OpMemberDecorate %Output 0 Offset 0
               OpMemberDecorate %Output 1 Offset 8
               OpMemberDecorate %Output 2 Offset 16
               OpMemberDecorate %Output 3 Offset 32
               OpMemberDecorate %Output 4 Offset 48
               OpMemberDecorate %Output 5 Offset 64
               OpMemberDecorate %Output 6 ColMajor
               OpMemberDecorate %Output 6 Offset 96
               OpMemberDecorate %Output 6 MatrixStride 16
               OpMemberDecorate %Output 7 Offset 128
               OpDecorate %Output Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
    %v3float = OpTypeVector %float 3
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
          %F = OpTypeStruct %int %v2uint
     %uint_2 = OpConstant %uint 2
%_arr_float_uint_2 = OpTypeArray %float %uint_2
%mat2v3float = OpTypeMatrix %v3float 2
     %v3uint = OpTypeVector %uint 3
%mat3v3float = OpTypeMatrix %v3float 3
%_arr_mat3v3float_uint_2 = OpTypeArray %mat3v3float %uint_2
          %O = OpTypeStruct %v3uint %v2float %_arr_float_uint_2 %v2float %_arr_mat3v3float_uint_2
%_arr_O_uint_2 = OpTypeArray %O %uint_2
     %Output = OpTypeStruct %float %v2float %v3float %F %float %_arr_float_uint_2 %mat2v3float %_arr_O_uint_2
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateAndRetrieveValidationState());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("must follow standard uniform buffer layout rules"));
}

TEST_F(ValidateDecorations,
       BlockStandardUniformBufferLayoutIncorrectOffset1Bad) {
  string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpMemberDecorate %F 0 Offset 0
               OpMemberDecorate %F 1 Offset 8
               OpDecorate %_arr_float_uint_2 ArrayStride 16
               OpDecorate %_arr_mat3v3float_uint_2 ArrayStride 48
               OpMemberDecorate %O 0 Offset 0
               OpMemberDecorate %O 1 Offset 16
               OpMemberDecorate %O 2 Offset 32
               OpMemberDecorate %O 3 Offset 64
               OpMemberDecorate %O 4 ColMajor
               OpMemberDecorate %O 4 Offset 80
               OpMemberDecorate %O 4 MatrixStride 16
               OpDecorate %_arr_O_uint_2 ArrayStride 176
               OpMemberDecorate %Output 0 Offset 0
               OpMemberDecorate %Output 1 Offset 8
               OpMemberDecorate %Output 2 Offset 16
               OpMemberDecorate %Output 3 Offset 32
               OpMemberDecorate %Output 4 Offset 48
               OpMemberDecorate %Output 5 Offset 71
               OpMemberDecorate %Output 6 ColMajor
               OpMemberDecorate %Output 6 Offset 96
               OpMemberDecorate %Output 6 MatrixStride 16
               OpMemberDecorate %Output 7 Offset 128
               OpDecorate %Output Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
    %v3float = OpTypeVector %float 3
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
          %F = OpTypeStruct %int %v2uint
     %uint_2 = OpConstant %uint 2
%_arr_float_uint_2 = OpTypeArray %float %uint_2
%mat2v3float = OpTypeMatrix %v3float 2
     %v3uint = OpTypeVector %uint 3
%mat3v3float = OpTypeMatrix %v3float 3
%_arr_mat3v3float_uint_2 = OpTypeArray %mat3v3float %uint_2
          %O = OpTypeStruct %v3uint %v2float %_arr_float_uint_2 %v2float %_arr_mat3v3float_uint_2
%_arr_O_uint_2 = OpTypeArray %O %uint_2
     %Output = OpTypeStruct %float %v2float %v3float %F %float %_arr_float_uint_2 %mat2v3float %_arr_O_uint_2
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateAndRetrieveValidationState());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("must follow standard uniform buffer layout rules"));
}

TEST_F(ValidateDecorations, BlockUniformBufferLayoutIncorrectArrayStrideBad) {
  string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpMemberDecorate %F 0 Offset 0
               OpMemberDecorate %F 1 Offset 8
               OpDecorate %_arr_float_uint_2 ArrayStride 16
               OpDecorate %_arr_mat3v3float_uint_2 ArrayStride 49
               OpMemberDecorate %O 0 Offset 0
               OpMemberDecorate %O 1 Offset 16
               OpMemberDecorate %O 2 Offset 32
               OpMemberDecorate %O 3 Offset 64
               OpMemberDecorate %O 4 ColMajor
               OpMemberDecorate %O 4 Offset 80
               OpMemberDecorate %O 4 MatrixStride 16
               OpDecorate %_arr_O_uint_2 ArrayStride 176
               OpMemberDecorate %Output 0 Offset 0
               OpMemberDecorate %Output 1 Offset 8
               OpMemberDecorate %Output 2 Offset 16
               OpMemberDecorate %Output 3 Offset 32
               OpMemberDecorate %Output 4 Offset 48
               OpMemberDecorate %Output 5 Offset 64
               OpMemberDecorate %Output 6 ColMajor
               OpMemberDecorate %Output 6 Offset 96
               OpMemberDecorate %Output 6 MatrixStride 16
               OpMemberDecorate %Output 7 Offset 128
               OpDecorate %Output Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
    %v3float = OpTypeVector %float 3
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
          %F = OpTypeStruct %int %v2uint
     %uint_2 = OpConstant %uint 2
%_arr_float_uint_2 = OpTypeArray %float %uint_2
%mat2v3float = OpTypeMatrix %v3float 2
     %v3uint = OpTypeVector %uint 3
%mat3v3float = OpTypeMatrix %v3float 3
%_arr_mat3v3float_uint_2 = OpTypeArray %mat3v3float %uint_2
          %O = OpTypeStruct %v3uint %v2float %_arr_float_uint_2 %v2float %_arr_mat3v3float_uint_2
%_arr_O_uint_2 = OpTypeArray %O %uint_2
     %Output = OpTypeStruct %float %v2float %v3float %F %float %_arr_float_uint_2 %mat2v3float %_arr_O_uint_2
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateAndRetrieveValidationState());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("must follow standard uniform buffer layout rules"));
}

TEST_F(ValidateDecorations,
       BufferBlockStandardStorageBufferLayoutImproperStraddleBad) {
  string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpMemberDecorate %Output 0 Offset 0
               OpMemberDecorate %Output 1 Offset 8
               OpDecorate %Output BufferBlock
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
     %Output = OpTypeStruct %float %v3float
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateAndRetrieveValidationState());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("must follow standard storage buffer layout rules"));
}

TEST_F(ValidateDecorations, BlockUniformBufferLayoutOffsetInsidePaddingBad) {
  string spirv = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpDecorate %_arr_float_uint_2 ArrayStride 16
               OpMemberDecorate %Output 0 Offset 0
               OpMemberDecorate %Output 1 Offset 20
               OpDecorate %Output Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
     %uint_2 = OpConstant %uint 2
%_arr_float_uint_2 = OpTypeArray %float %uint_2
     %Output = OpTypeStruct %_arr_float_uint_2 %float
%_ptr_Uniform_Output = OpTypePointer Uniform %Output
 %dataOutput = OpVariable %_ptr_Uniform_Output Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateAndRetrieveValidationState());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("must follow standard uniform buffer layout rules"));
}

}  // anonymous namespace
